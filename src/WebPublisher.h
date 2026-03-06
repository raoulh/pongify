#ifndef WEBPUBLISHER_H
#define WEBPUBLISHER_H

#include <QObject>
#include <QTimer>

class QNetworkAccessManager;
class QNetworkReply;
class Tournament;

class WebPublisher : public QObject
{
    Q_OBJECT
public:
    static WebPublisher *Instance()
    {
        static WebPublisher m;
        return &m;
    }

    void setTournament(Tournament *t);
    void clearTournament();
    void publish();
    void unpublish();
    void uploadWebapp(const QString &version);

    QString workerUrl() const;
    QString adminSecret() const;
    bool enabled() const;

    void setWorkerUrl(const QString &url);
    void setAdminSecret(const QString &secret);
    void setEnabled(bool enabled);

    void loadSettings();
    void saveSettings();

    static QString generateSecret();   // 64 chars hex, 256 bits
    static QByteArray generateKey();   // 32 bytes raw, for AES-256

    bool testConnection();

signals:
    void publishError(const QString &message);
    void publishSuccess();

private:
    WebPublisher();

    void doPublish();
    void doEncryptAndSend(const QByteArray &plaintext);
    void handleReply(QNetworkReply *reply);
    void reconnectSignals();

    QNetworkAccessManager *m_nam = nullptr;
    QTimer *m_debounceTimer = nullptr;
    Tournament *m_tournament = nullptr;

    QString m_workerUrl;
    QString m_adminSecret;
    bool m_enabled = false;

    QList<QMetaObject::Connection> m_connections;
};

#endif // WEBPUBLISHER_H
