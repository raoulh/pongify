#ifndef DIALOGCLOUDFLARESETUP_H
#define DIALOGCLOUDFLARESETUP_H

#include <QDialog>

namespace Ui { class DialogCloudflareSetup; }

class CloudflareSetup;

class DialogCloudflareSetup : public QDialog
{
    Q_OBJECT
public:
    explicit DialogCloudflareSetup(QWidget *parent = nullptr);
    ~DialogCloudflareSetup();

    QString workerUrl() const;
    QString adminSecret() const;
    bool setupSucceeded() const { return m_success; }

private slots:
    void onDeploy();

private:
    void appendLog(const QString &text);
    void setRunning(bool running);

    Ui::DialogCloudflareSetup *ui;
    CloudflareSetup *m_setup = nullptr;
    bool m_success = false;
};

#endif // DIALOGCLOUDFLARESETUP_H
