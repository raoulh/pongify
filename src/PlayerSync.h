#ifndef PLAYERSYNC_H
#define PLAYERSYNC_H

#include <QObject>
#include <QProgressDialog>
#include <QProcess>

class QEventLoop;

class PlayerSync : public QObject
{
    Q_OBJECT
public:
    explicit PlayerSync(QObject *parent = nullptr);
    virtual ~PlayerSync();

public slots:
    void start();

private slots:
    void cancel();
    void readStdOut();

private:
    QProcess *cdslsProc = nullptr;
    QProgressDialog *progressDlg = nullptr;
    QEventLoop *eventLoop = nullptr;
};

#endif // PLAYERSYNC_H
