#ifndef DIALOGQUALIFICATION_H
#define DIALOGQUALIFICATION_H

#include <QDialog>

namespace Ui {
class DialogQualification;
}

class Tournament;
class TSerie;
class Player;

// Represents a ranked player from a source serie
struct QualifiedPlayer
{
    Player *player = nullptr;
    int rank = 0;         // 1-based rank within source serie
    int score = 0;        // Points (RR) or 0 (single)
    QString serieName;    // Name of the source serie
    QString serieUid;     // UID of the source serie
};

class DialogQualification : public QDialog
{
    Q_OBJECT

public:
    // Mode: fromSource = true when triggered from a finished serie (transfer out)
    //        fromSource = false when triggered from a stopped serie (import in)
    explicit DialogQualification(Tournament *tournament,
                                 TSerie *contextSerie,
                                 bool fromSource,
                                 QWidget *parent = nullptr);
    ~DialogQualification();

private slots:
    void on_buttonBox_accepted();
    void on_pushButtonApplyTopN_clicked();
    void onSourceSelectionChanged();

private:
    Ui::DialogQualification *ui;
    Tournament *tournament = nullptr;
    TSerie *contextSerie = nullptr;
    bool fromSource = false;

    QList<QualifiedPlayer> allRankedPlayers;

    void buildRankedPlayersFromSerie(TSerie *serie);
    void refreshPlayerList();
};

#endif // DIALOGQUALIFICATION_H
