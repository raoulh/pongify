#include "CloudflareSetup.h"
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QFile>
#include <QUuid>

const QString CloudflareSetup::cfBaseURL = "https://api.cloudflare.com/client/v4";
const QString CloudflareSetup::workerName = "pongify-live";
const QString CloudflareSetup::kvTitle = "PONGIFY_TOURNAMENTS";

CloudflareSetup::CloudflareSetup(QObject *parent)
    : QObject(parent)
{
    m_nam = new QNetworkAccessManager(this);
}

void CloudflareSetup::start(const QString &apiToken)
{
    m_token = apiToken.trimmed();
    m_aborted = false;
    m_currentStep = 0;
    m_accountId.clear();
    m_namespaceId.clear();
    m_adminSecret.clear();
    m_workerUrl.clear();

    if (m_token.isEmpty()) {
        fail(tr("Le token API est vide."));
        return;
    }

    stepVerifyToken();
}

void CloudflareSetup::abort()
{
    m_aborted = true;
}

void CloudflareSetup::emitStep(const QString &description)
{
    m_currentStep++;
    emit stepChanged(m_currentStep, m_totalSteps, description);
}

void CloudflareSetup::fail(const QString &message)
{
    emit errorOccurred(message);
    emit finished(false);
}

// ---------------------------------------------------------------------------
// Cloudflare API helpers
// ---------------------------------------------------------------------------

QNetworkReply *CloudflareSetup::cfGet(const QString &url)
{
    QNetworkRequest req{QUrl(url)};
    req.setRawHeader("Authorization", ("Bearer " + m_token).toUtf8());
    return m_nam->get(req);
}

QNetworkReply *CloudflareSetup::cfPost(const QString &url, const QByteArray &body,
                                       const QString &contentType)
{
    QNetworkRequest req{QUrl(url)};
    req.setRawHeader("Authorization", ("Bearer " + m_token).toUtf8());
    req.setHeader(QNetworkRequest::ContentTypeHeader, contentType);
    return m_nam->post(req, body);
}

QNetworkReply *CloudflareSetup::cfPut(const QString &url, const QByteArray &body,
                                      const QString &contentType)
{
    QNetworkRequest req{QUrl(url)};
    req.setRawHeader("Authorization", ("Bearer " + m_token).toUtf8());
    req.setHeader(QNetworkRequest::ContentTypeHeader, contentType);
    return m_nam->put(req, body);
}

CloudflareSetup::CfResponse CloudflareSetup::parseCfResponse(QNetworkReply *reply)
{
    CfResponse r;
    QByteArray data = reply->readAll();

    QJsonParseError parseErr;
    QJsonDocument doc = QJsonDocument::fromJson(data, &parseErr);
    if (parseErr.error != QJsonParseError::NoError) {
        r.errorMessage = tr("Réponse invalide du serveur: %1").arg(parseErr.errorString());
        return r;
    }

    QJsonObject obj = doc.object();
    r.success = obj["success"].toBool();

    if (obj["result"].isArray())
        r.resultArray = obj["result"].toArray();
    else if (obj["result"].isObject())
        r.result = obj["result"].toObject();

    if (!r.success) {
        QJsonArray errors = obj["errors"].toArray();
        QStringList msgs;
        for (const auto &e : errors) {
            QJsonObject err = e.toObject();
            msgs << QString("[%1] %2").arg(err["code"].toInt()).arg(err["message"].toString());
        }
        r.errorMessage = msgs.join("; ");
    }

    return r;
}

// ---------------------------------------------------------------------------
// Step 1: Verify token and get account ID
// ---------------------------------------------------------------------------

void CloudflareSetup::stepVerifyToken()
{
    if (m_aborted) return;
    emitStep(tr("Vérification du token et récupération du compte..."));

    QNetworkReply *reply = cfGet(cfBaseURL + "/accounts?page=1&per_page=5");
    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        reply->deleteLater();
        if (m_aborted) return;

        if (reply->error() != QNetworkReply::NoError) {
            fail(tr("Erreur réseau: %1").arg(reply->errorString()));
            return;
        }

        auto resp = parseCfResponse(reply);
        if (!resp.success) {
            fail(tr("Token invalide ou impossible de récupérer les comptes: %1").arg(resp.errorMessage));
            return;
        }

        if (resp.resultArray.isEmpty()) {
            fail(tr("Aucun compte Cloudflare trouvé pour ce token."));
            return;
        }

        QJsonObject account = resp.resultArray.first().toObject();
        m_accountId = account["id"].toString();
        QString accountName = account["name"].toString();
        emit stepChanged(m_currentStep, m_totalSteps,
                         tr("Compte: %1 (%2)").arg(accountName, m_accountId));

        stepEnsureKVNamespace();
    });
}

// ---------------------------------------------------------------------------
// Step 2: Create or find KV namespace
// ---------------------------------------------------------------------------

void CloudflareSetup::stepEnsureKVNamespace()
{
    if (m_aborted) return;
    emitStep(tr("Création du namespace KV..."));

    QString url = QString("%1/accounts/%2/storage/kv/namespaces").arg(cfBaseURL, m_accountId);
    QByteArray body = QString("{\"title\":\"%1\"}").arg(kvTitle).toUtf8();

    QNetworkReply *reply = cfPost(url, body);
    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        reply->deleteLater();
        if (m_aborted) return;

        auto resp = parseCfResponse(reply);
        if (resp.success) {
            m_namespaceId = resp.result["id"].toString();
            emit stepChanged(m_currentStep, m_totalSteps,
                             tr("Namespace KV créé: %1").arg(m_namespaceId));
            stepGenerateAdminSecret();
            return;
        }

        // Already exists — list and find by title
        QString listUrl = QString("%1/accounts/%2/storage/kv/namespaces?per_page=100")
                              .arg(cfBaseURL, m_accountId);
        QNetworkReply *listReply = cfGet(listUrl);
        connect(listReply, &QNetworkReply::finished, this, [this, listReply]() {
            listReply->deleteLater();
            if (m_aborted) return;

            auto listResp = parseCfResponse(listReply);
            if (!listResp.success) {
                fail(tr("Impossible de lister les namespaces KV: %1").arg(listResp.errorMessage));
                return;
            }

            for (const auto &ns : listResp.resultArray) {
                QJsonObject nsObj = ns.toObject();
                if (nsObj["title"].toString() == kvTitle) {
                    m_namespaceId = nsObj["id"].toString();
                    emit stepChanged(m_currentStep, m_totalSteps,
                                     tr("Namespace KV existant: %1").arg(m_namespaceId));
                    stepGenerateAdminSecret();
                    return;
                }
            }

            fail(tr("Namespace KV introuvable après création échouée. "
                     "Vérifiez les permissions du token."));
        });
    });
}

// ---------------------------------------------------------------------------
// Step 3: Generate admin secret
// ---------------------------------------------------------------------------

void CloudflareSetup::stepGenerateAdminSecret()
{
    if (m_aborted) return;
    emitStep(tr("Génération du secret admin..."));

    // 64 hex chars = 256 bits
    m_adminSecret = QUuid::createUuid().toString(QUuid::Id128)
                  + QUuid::createUuid().toString(QUuid::Id128);

    stepDeployWorker();
}

// ---------------------------------------------------------------------------
// Step 4: Deploy the worker
// ---------------------------------------------------------------------------

void CloudflareSetup::stepDeployWorker()
{
    if (m_aborted) return;
    emitStep(tr("Déploiement du Worker Cloudflare..."));

    // Read worker.js from Qt resources
    QFile workerFile(":/cloudflare/worker.js");
    if (!workerFile.open(QIODevice::ReadOnly)) {
        fail(tr("Impossible de lire le fichier worker.js depuis les ressources."));
        return;
    }
    QByteArray workerJS = workerFile.readAll();
    workerFile.close();

    // Build multipart body
    QString boundary = "----PongifyBoundary" + QUuid::createUuid().toString(QUuid::Id128);

    QByteArray body;
    // Part 1: worker.js (ES module)
    body.append(("--" + boundary + "\r\n").toUtf8());
    body.append("Content-Disposition: form-data; name=\"worker.js\"; filename=\"worker.js\"\r\n");
    body.append("Content-Type: application/javascript+module\r\n\r\n");
    body.append(workerJS);
    body.append("\r\n");

    // Part 2: metadata JSON
    QString metadata = QString(
        "{"
        "\"main_module\":\"worker.js\","
        "\"bindings\":[{\"type\":\"kv_namespace\",\"name\":\"KV\",\"namespace_id\":\"%1\"}],"
        "\"compatibility_date\":\"2024-01-01\""
        "}").arg(m_namespaceId);

    body.append(("--" + boundary + "\r\n").toUtf8());
    body.append("Content-Disposition: form-data; name=\"metadata\"\r\n");
    body.append("Content-Type: application/json\r\n\r\n");
    body.append(metadata.toUtf8());
    body.append("\r\n");

    body.append(("--" + boundary + "--\r\n").toUtf8());

    QString url = QString("%1/accounts/%2/workers/scripts/%3")
                      .arg(cfBaseURL, m_accountId, workerName);
    QString contentType = "multipart/form-data; boundary=" + boundary;

    QNetworkReply *reply = cfPut(url, body, contentType);
    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        reply->deleteLater();
        if (m_aborted) return;

        if (reply->error() != QNetworkReply::NoError) {
            auto resp = parseCfResponse(reply);
            fail(tr("Échec du déploiement du Worker: %1").arg(
                resp.errorMessage.isEmpty() ? reply->errorString() : resp.errorMessage));
            return;
        }

        auto resp = parseCfResponse(reply);
        if (!resp.success) {
            fail(tr("Échec du déploiement du Worker: %1").arg(resp.errorMessage));
            return;
        }

        stepSetAdminSecret();
    });
}

// ---------------------------------------------------------------------------
// Step 5: Set ADMIN_SECRET on the worker
// ---------------------------------------------------------------------------

void CloudflareSetup::stepSetAdminSecret()
{
    if (m_aborted) return;
    emitStep(tr("Configuration du secret ADMIN_SECRET..."));

    QString url = QString("%1/accounts/%2/workers/scripts/%3/secrets")
                      .arg(cfBaseURL, m_accountId, workerName);
    QByteArray body = QString("{\"name\":\"ADMIN_SECRET\",\"text\":\"%1\",\"type\":\"secret_text\"}")
                          .arg(m_adminSecret).toUtf8();

    QNetworkReply *reply = cfPut(url, body);
    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        reply->deleteLater();
        if (m_aborted) return;

        if (reply->error() != QNetworkReply::NoError) {
            auto resp = parseCfResponse(reply);
            fail(tr("Échec de la configuration du secret: %1").arg(
                resp.errorMessage.isEmpty() ? reply->errorString() : resp.errorMessage));
            return;
        }

        stepEnableSubdomain();
    });
}

// ---------------------------------------------------------------------------
// Step 6: Enable workers.dev subdomain
// ---------------------------------------------------------------------------

void CloudflareSetup::stepEnableSubdomain()
{
    if (m_aborted) return;
    emitStep(tr("Activation du sous-domaine workers.dev..."));

    // Account-level: enable workers.dev subdomain
    QString accountUrl = QString("%1/accounts/%2/workers/subdomain")
                             .arg(cfBaseURL, m_accountId);
    QNetworkReply *reply = cfPost(accountUrl, "{\"enabled\":true}");
    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        reply->deleteLater();
        if (m_aborted) return;

        // Ignore errors here (may already be enabled)

        // Script-level: enable workers.dev route
        QString scriptUrl = QString("%1/accounts/%2/workers/scripts/%3/subdomain")
                                .arg(cfBaseURL, m_accountId, workerName);
        QNetworkReply *scriptReply = cfPost(scriptUrl, "{\"enabled\":true}");
        connect(scriptReply, &QNetworkReply::finished, this, [this, scriptReply]() {
            scriptReply->deleteLater();
            if (m_aborted) return;

            // Ignore errors here too (may already be enabled)

            stepGetWorkerUrl();
        });
    });
}

// ---------------------------------------------------------------------------
// Step 7: Get worker URL
// ---------------------------------------------------------------------------

void CloudflareSetup::stepGetWorkerUrl()
{
    if (m_aborted) return;
    emitStep(tr("Récupération de l'URL du Worker..."));

    QString url = QString("%1/accounts/%2/workers/subdomain").arg(cfBaseURL, m_accountId);
    QNetworkReply *reply = cfGet(url);
    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        reply->deleteLater();
        if (m_aborted) return;

        if (reply->error() != QNetworkReply::NoError) {
            fail(tr("Impossible de récupérer le sous-domaine: %1").arg(reply->errorString()));
            return;
        }

        auto resp = parseCfResponse(reply);
        if (!resp.success) {
            fail(tr("Impossible de récupérer le sous-domaine: %1").arg(resp.errorMessage));
            return;
        }

        QString subdomain = resp.result["subdomain"].toString();
        m_workerUrl = QString("https://%1.%2.workers.dev").arg(workerName, subdomain);

        emit stepChanged(m_currentStep, m_totalSteps,
                         tr("URL: %1").arg(m_workerUrl));
        emit finished(true);
    });
}
