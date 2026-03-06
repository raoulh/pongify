#ifndef DIALOGWEBPUBLISH_H
#define DIALOGWEBPUBLISH_H

#include <QDialog>

namespace Ui { class DialogWebPublish; }

class DialogWebPublish : public QDialog
{
    Q_OBJECT
public:
    explicit DialogWebPublish(QWidget *parent = nullptr);
    ~DialogWebPublish();

    QString getWorkerUrl() const;
    QString getAdminSecret() const;
    bool getEnabled() const;

private slots:
    void testConnection();
    void openCloudflareSetup();

private:
    Ui::DialogWebPublish *ui;
};

#endif // DIALOGWEBPUBLISH_H
