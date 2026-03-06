#include "WebPublisher.h"
#include "Tournament.h"
#include "TSerie.h"
#include "TTable.h"
#include "version.h"
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QJsonDocument>
#include <QJsonObject>
#include <QSettings>
#include <QUuid>
#include <QFile>
#include <QEventLoop>

#include <windows.h>
#include <bcrypt.h>

WebPublisher::WebPublisher() : QObject(nullptr)
{
    m_nam = new QNetworkAccessManager(this);

    m_debounceTimer = new QTimer(this);
    m_debounceTimer->setSingleShot(true);
    m_debounceTimer->setInterval(2000);
    connect(m_debounceTimer, &QTimer::timeout, this, &WebPublisher::doPublish);

    loadSettings();
}

void WebPublisher::loadSettings()
{
    QSettings settings;
    m_workerUrl = settings.value("WebPublish/workerUrl").toString();
    m_adminSecret = settings.value("WebPublish/adminSecret").toString();
    m_enabled = settings.value("WebPublish/enabled", false).toBool();
}

void WebPublisher::saveSettings()
{
    QSettings settings;
    settings.setValue("WebPublish/workerUrl", m_workerUrl);
    settings.setValue("WebPublish/adminSecret", m_adminSecret);
    settings.setValue("WebPublish/enabled", m_enabled);
}

QString WebPublisher::workerUrl() const { return m_workerUrl; }
QString WebPublisher::adminSecret() const { return m_adminSecret; }
bool WebPublisher::enabled() const { return m_enabled; }

void WebPublisher::setWorkerUrl(const QString &url) { m_workerUrl = url; }
void WebPublisher::setAdminSecret(const QString &secret) { m_adminSecret = secret; }
void WebPublisher::setEnabled(bool enabled) { m_enabled = enabled; }

void WebPublisher::setTournament(Tournament *t)
{
    clearTournament();
    m_tournament = t;
    if (!t || !m_enabled) return;

    reconnectSignals();
    publish();
}

void WebPublisher::reconnectSignals()
{
    if (!m_tournament) return;

    // Disconnect existing connections first
    for (auto &conn : m_connections)
        disconnect(conn);
    m_connections.clear();

    m_connections << connect(m_tournament, &Tournament::seriesStatusChanged, this, [this]() {
        reconnectSignals();
        publish();
    });
    m_connections << connect(m_tournament, &Tournament::defaultViewVisibleChanged, this, &WebPublisher::publish);

    for (int i = 0; i < m_tournament->serieCount(); i++) {
        auto s = m_tournament->getSerie(i);
        m_connections << connect(s, &TSerie::matchesUpdated, this, &WebPublisher::publish);
        m_connections << connect(s, &TSerie::viewVisibleChanged, this, &WebPublisher::publish);
    }

    for (int i = 0; i < m_tournament->tableCount(); i++) {
        auto table = m_tournament->getTable(i);
        m_connections << connect(table, &TTable::matchChanged, this, &WebPublisher::publish);
    }
}

void WebPublisher::clearTournament()
{
    for (auto &conn : m_connections)
        disconnect(conn);
    m_connections.clear();
    m_tournament = nullptr;
    m_debounceTimer->stop();
}

void WebPublisher::publish()
{
    if (!m_enabled || !m_tournament || m_workerUrl.isEmpty()) return;
    m_debounceTimer->start();
}

void WebPublisher::doPublish()
{
    if (!m_tournament) return;

    QJsonObject json = m_tournament->toJsonForPublish();
    QByteArray plaintext = QJsonDocument(json).toJson(QJsonDocument::Compact);

    doEncryptAndSend(plaintext);
}

void WebPublisher::doEncryptAndSend(const QByteArray &plaintext)
{
    QByteArray key = QByteArray::fromBase64(
        m_tournament->get_encryptionKey().toLatin1(),
        QByteArray::Base64UrlEncoding
    );
    if (key.size() != 32) {
        qWarning() << "WebPublisher: invalid encryption key size";
        return;
    }

    // Generate random IV (12 bytes for AES-GCM)
    unsigned char iv[12];
    BCryptGenRandom(nullptr, iv, 12, BCRYPT_USE_SYSTEM_PREFERRED_RNG);

    // Open AES algorithm provider
    BCRYPT_ALG_HANDLE hAlg = nullptr;
    NTSTATUS status = BCryptOpenAlgorithmProvider(&hAlg, BCRYPT_AES_ALGORITHM, nullptr, 0);
    if (!BCRYPT_SUCCESS(status)) {
        qWarning() << "WebPublisher: BCryptOpenAlgorithmProvider failed";
        return;
    }

    // Set chaining mode to GCM
    status = BCryptSetProperty(hAlg, BCRYPT_CHAINING_MODE,
                               (PUCHAR)BCRYPT_CHAIN_MODE_GCM,
                               sizeof(BCRYPT_CHAIN_MODE_GCM), 0);
    if (!BCRYPT_SUCCESS(status)) {
        BCryptCloseAlgorithmProvider(hAlg, 0);
        qWarning() << "WebPublisher: BCryptSetProperty GCM failed";
        return;
    }

    // Generate symmetric key
    BCRYPT_KEY_HANDLE hKey = nullptr;
    status = BCryptGenerateSymmetricKey(hAlg, &hKey, nullptr, 0,
                                        (PUCHAR)key.data(), key.size(), 0);
    if (!BCRYPT_SUCCESS(status)) {
        BCryptCloseAlgorithmProvider(hAlg, 0);
        qWarning() << "WebPublisher: BCryptGenerateSymmetricKey failed";
        return;
    }

    // Setup GCM auth info
    BCRYPT_AUTHENTICATED_CIPHER_MODE_INFO authInfo;
    BCRYPT_INIT_AUTH_MODE_INFO(authInfo);
    unsigned char tag[16];
    authInfo.pbNonce = iv;
    authInfo.cbNonce = 12;
    authInfo.pbTag = tag;
    authInfo.cbTag = 16;

    // Encrypt
    ULONG cbCipherText = 0;
    status = BCryptEncrypt(hKey, (PUCHAR)plaintext.constData(), plaintext.size(),
                           &authInfo, nullptr, 0, nullptr, 0, &cbCipherText, 0);
    if (!BCRYPT_SUCCESS(status)) {
        BCryptDestroyKey(hKey);
        BCryptCloseAlgorithmProvider(hAlg, 0);
        qWarning() << "WebPublisher: BCryptEncrypt size query failed";
        return;
    }

    QByteArray ciphertext(cbCipherText, 0);
    ULONG cbResult = 0;

    // Reset auth info for actual encryption
    BCRYPT_INIT_AUTH_MODE_INFO(authInfo);
    authInfo.pbNonce = iv;
    authInfo.cbNonce = 12;
    authInfo.pbTag = tag;
    authInfo.cbTag = 16;

    status = BCryptEncrypt(hKey, (PUCHAR)plaintext.constData(), plaintext.size(),
                           &authInfo, nullptr, 0,
                           (PUCHAR)ciphertext.data(), cbCipherText, &cbResult, 0);

    BCryptDestroyKey(hKey);
    BCryptCloseAlgorithmProvider(hAlg, 0);

    if (!BCRYPT_SUCCESS(status)) {
        qWarning() << "WebPublisher: BCryptEncrypt failed";
        return;
    }

    // Assemble payload: IV (12) + ciphertext + tag (16)
    QByteArray payload;
    payload.append(reinterpret_cast<const char*>(iv), 12);
    payload.append(ciphertext.constData(), cbResult);
    payload.append(reinterpret_cast<const char*>(tag), 16);

    // Send via HTTP PUT
    QString url = m_workerUrl + "/api/tournament/" + m_tournament->get_uuid();
    qInfo() << "WebPublisher: publishing tournament" << m_tournament->get_uuid() << "(" << payload.size() << "bytes)";
    QNetworkRequest request{QUrl(url)};
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/octet-stream");
    request.setRawHeader("X-Write-Secret", m_tournament->get_writeSecret().toUtf8());
    request.setRawHeader("X-Admin-Secret", m_adminSecret.toUtf8());
    request.setRawHeader("X-Pongify-Version", pongify_version);

    QNetworkReply *reply = m_nam->put(request, payload);
    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        handleReply(reply);
        reply->deleteLater();
    });
}

void WebPublisher::handleReply(QNetworkReply *reply)
{
    if (reply->error() != QNetworkReply::NoError) {
        qWarning() << "WebPublisher: HTTP error:" << reply->errorString();
        emit publishError(reply->errorString());
    } else {
        int status = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
        if (status >= 200 && status < 300) {
            qInfo() << "WebPublisher: publish OK (HTTP" << status << ")";
            emit publishSuccess();
        } else {
            qWarning() << "WebPublisher: HTTP" << status << reply->readAll();
            emit publishError(QString("HTTP %1").arg(status));
        }
    }
}

void WebPublisher::unpublish()
{
    if (!m_tournament || m_workerUrl.isEmpty()) return;

    qInfo() << "WebPublisher: unpublishing tournament" << m_tournament->get_uuid();
    QString url = m_workerUrl + "/api/tournament/" + m_tournament->get_uuid();
    QNetworkRequest request{QUrl(url)};
    request.setRawHeader("X-Write-Secret", m_tournament->get_writeSecret().toUtf8());

    m_nam->deleteResource(request);
    clearTournament();
}

void WebPublisher::uploadWebapp(const QString &version)
{
    if (m_workerUrl.isEmpty() || m_adminSecret.isEmpty())
    {
        qWarning() << "WebPublisher: cannot upload webapp — workerUrl or adminSecret is empty";
        return;
    }

    qInfo() << "WebPublisher: uploading webapp version" << version;
    QStringList files = {"index.html", "app.js", "app.css"};
    for (const auto &filename : files) {
        QFile res(":/webapp/" + filename);
        if (!res.open(QIODevice::ReadOnly)) {
            qWarning() << "WebPublisher: cannot open resource" << filename;
            continue;
        }
        QByteArray content = res.readAll();
        res.close();

        qInfo() << "WebPublisher: uploading" << filename << "(" << content.size() << "bytes)";
        QString putUrl = m_workerUrl + "/api/webapp/" + version + "/" + filename;
        QNetworkRequest req{QUrl(putUrl)};
        req.setHeader(QNetworkRequest::ContentTypeHeader, "text/plain");
        req.setRawHeader("X-Admin-Secret", m_adminSecret.toUtf8());
        auto *reply = m_nam->put(req, content);
        connect(reply, &QNetworkReply::finished, this, [reply, filename]() {
            int status = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
            if (status >= 200 && status < 300)
                qInfo() << "WebPublisher: uploaded" << filename << "OK";
            else
                qWarning() << "WebPublisher: upload" << filename << "failed — HTTP" << status << reply->readAll();
            reply->deleteLater();
        });
    }
}

QString WebPublisher::generateSecret()
{
    return QUuid::createUuid().toString(QUuid::Id128)
         + QUuid::createUuid().toString(QUuid::Id128);
}

QByteArray WebPublisher::generateKey()
{
    QByteArray key(32, 0);
    BCryptGenRandom(nullptr, reinterpret_cast<PUCHAR>(key.data()), 32,
                    BCRYPT_USE_SYSTEM_PREFERRED_RNG);
    return key;
}

bool WebPublisher::testConnection()
{
    if (m_workerUrl.isEmpty()) return false;

    QNetworkRequest req(QUrl(m_workerUrl + "/api/health"));
    QNetworkReply *reply = m_nam->get(req);

    QEventLoop loop;
    connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
    QTimer::singleShot(5000, &loop, &QEventLoop::quit);
    loop.exec();

    bool ok = reply->error() == QNetworkReply::NoError;
    reply->deleteLater();
    return ok;
}
