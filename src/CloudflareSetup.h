#ifndef CLOUDFLARESETUP_H
#define CLOUDFLARESETUP_H

#include <QObject>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

class QNetworkAccessManager;
class QNetworkReply;

class CloudflareSetup : public QObject
{
    Q_OBJECT
public:
    explicit CloudflareSetup(QObject *parent = nullptr);

    void start(const QString &apiToken);
    void abort();

    QString workerUrl() const { return m_workerUrl; }
    QString adminSecret() const { return m_adminSecret; }

signals:
    void stepChanged(int step, int totalSteps, const QString &description);
    void finished(bool success);
    void errorOccurred(const QString &message);

private:
    void stepVerifyToken();
    void stepEnsureKVNamespace();
    void stepGenerateAdminSecret();
    void stepDeployWorker();
    void stepSetAdminSecret();
    void stepEnableSubdomain();
    void stepGetWorkerUrl();

    void emitStep(const QString &description);
    void fail(const QString &message);

    // Helpers for Cloudflare API
    QNetworkReply *cfGet(const QString &url);
    QNetworkReply *cfPost(const QString &url, const QByteArray &body,
                          const QString &contentType = "application/json");
    QNetworkReply *cfPut(const QString &url, const QByteArray &body,
                         const QString &contentType = "application/json");

    struct CfResponse {
        bool success = false;
        QJsonObject result;
        QJsonArray resultArray;
        QString errorMessage;
    };
    CfResponse parseCfResponse(QNetworkReply *reply);

    QNetworkAccessManager *m_nam = nullptr;
    QString m_token;
    QString m_accountId;
    QString m_namespaceId;
    QString m_adminSecret;
    QString m_workerUrl;
    bool m_aborted = false;

    int m_currentStep = 0;
    static const int m_totalSteps = 7;

    static const QString cfBaseURL;
    static const QString workerName;
    static const QString kvTitle;
};

#endif // CLOUDFLARESETUP_H
