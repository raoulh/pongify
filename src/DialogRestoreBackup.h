#ifndef DIALOGRESTOREBACKUP_H
#define DIALOGRESTOREBACKUP_H

#include <QDialog>

namespace Ui { class DialogRestoreBackup; }

class DialogRestoreBackup : public QDialog
{
    Q_OBJECT
public:
    explicit DialogRestoreBackup(const QString &uuid, const QString &tournamentName, QWidget *parent = nullptr);
    ~DialogRestoreBackup();

    int selectedBackupIndex() const;

private:
    Ui::DialogRestoreBackup *ui;
    QString m_uuid;
    int m_selectedIndex = -1;
};

#endif // DIALOGRESTOREBACKUP_H
