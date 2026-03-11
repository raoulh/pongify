#include "DialogPlayerList.h"
#include "ui_DialogPlayerList.h"
#include "TSerie.h"
#include "Tournament.h"
#include "PlayerModel.h"
#include "DialogPlayers.h"
#include "DialogAddDoublePlayers.h"

#include <QMessageBox>
#include <QStyledItemDelegate>
#include <QPainter>
#include <QMenu>
#include <QInputDialog>

class TwoLineItemDelegate: public QStyledItemDelegate
{
public:
    TwoLineItemDelegate():
        QStyledItemDelegate()
    {}

    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override
    {
        QStyledItemDelegate::paint(painter,option,index);

        //When only one player, draw using default class
        if (!index.data(PlayerModel::RoleFirstName).isValid() ||
            index.data(PlayerModel::RoleFirstNameSecond).toString() == "")
            return QStyledItemDelegate::paint(painter, option, index);

        painter->save();

        QStyleOptionViewItem opt = option;
        initStyleOption(&opt, index);

        QString headerText = QStringLiteral("%1 %2").arg(
                                 index.data(PlayerModel::RoleFirstName).toString(),
                                 index.data(PlayerModel::RoleLastName).toString());
        QString subText = QStringLiteral("%1 %2").arg(
                              index.data(PlayerModel::RoleFirstNameSecond).toString(),
                              index.data(PlayerModel::RoleLastNameSecond).toString());

        QIcon icon = qvariant_cast<QIcon>(index.data(Qt::DecorationRole));
        QSize iconsize = icon.actualSize(option.decorationSize);

        // draw correct background
        opt.text = "";
        QStyle *style = opt.widget ? opt.widget->style() : QApplication::style();
        style->drawControl(QStyle::CE_ItemViewItem, &opt, painter, opt.widget);

        QRect rect = opt.rect;
        QPalette::ColorGroup cg = opt.state & QStyle::State_Enabled ? QPalette::Normal : QPalette::Disabled;
        if (cg == QPalette::Normal && !(opt.state & QStyle::State_Active))
            cg = QPalette::Inactive;

        // set pen color
        if (opt.state & QStyle::State_Selected)
            painter->setPen(opt.palette.color(cg, QPalette::HighlightedText));
        else
            painter->setPen(opt.palette.color(cg, QPalette::Text));

        QFont font = painter->font();
        QFont subFont = painter->font();
        //font.setBold(true);
        //subFont.setWeight(subFont.weight() - 4);
        //subFont.setPointSize(subFont.pointSize() - 1);
        //subFont.setItalic(true);

        // draw 2 lines of text
        painter->setFont(font);
        painter->drawText(QRect(rect.left() + iconsize.width() + 8,
                                rect.top(),
                                rect.width() - iconsize.width() - 8,
                                rect.height() / 2),
                          opt.displayAlignment, headerText);
        painter->setFont(subFont);

        if (!(opt.state & QStyle::State_Selected))
        {
            painter->setPen(QColor::fromString(u"#A0A0A0"));
        }

        painter->drawText(QRect(rect.left() + iconsize.width() + 8,
                                rect.top() + rect.height() / 2,
                                rect.width() - iconsize.width() - 8,
                                rect.height() / 2),
                          opt.displayAlignment, subText);

        painter->restore();
    }

    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const override
    {
        //When only one player, draw using default class
        if (!index.data(PlayerModel::RoleFirstName).isValid() ||
             index.data(PlayerModel::RoleFirstNameSecond).toString() == "")
            return QStyledItemDelegate::sizeHint(option, index);

        QIcon icon = qvariant_cast<QIcon>(index.data(Qt::DecorationRole));
        QSize iconsize = icon.actualSize(option.decorationSize);

        QFont font = QApplication::font();
        QFont subFont = QApplication::font();

        subFont.setPointSize(subFont.pointSize() - 1);
        subFont.setItalic(true);

        QFontMetrics fm(font), subfm(subFont);

        QString headerText = QStringLiteral("%1 %2").arg(
                                 index.data(PlayerModel::RoleFirstName).toString(),
                                 index.data(PlayerModel::RoleLastName).toString());
        QString subText = QStringLiteral("%1 %2").arg(
                              index.data(PlayerModel::RoleFirstNameSecond).toString(),
                              index.data(PlayerModel::RoleLastNameSecond).toString());

        int textWidth = fm.horizontalAdvance(headerText);
        int subWidth = subfm.horizontalAdvance(subText);
        if (textWidth < subWidth) textWidth = subWidth;

        QSize sz(iconsize.width() + textWidth + 10, fm.height() + subfm.height() + 8);
        if (sz.width() < 180)
            sz.setWidth(180);

        return sz;
    }
};

DialogPlayerList::DialogPlayerList(TSerie *s, Tournament *t, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DialogPlayerList),
    serie(s),
    tournament(t)
{
    ui->setupUi(this);

    ui->labelSerie->setText(QStringLiteral("Série: %1").arg(serie->get_name()));

    playerModel = PlayerModel::createEmpty();
    for (int i = 0;i < serie->getPlayerModel()->rowCount();i++)
    {
        playerModel->appendClone(serie->getPlayerModel()->item(i));
    }

    filterModel = new PlayerFilterModel(this);
    filterModel->setSourceModel(playerModel);

    ui->treeView->setItemDelegate(new TwoLineItemDelegate());
    ui->treeView->itemDelegate()->setParent(ui->treeView);
    ui->treeView->setModel(filterModel);
    ui->treeView->setUniformRowHeights(true);

    connect(playerModel, &PlayerModel::playersChanged, this, [this]()
    {
        ui->labelPlayerCount->setText(QStringLiteral("%1 joueurs").arg(playerModel->rowCount()));
    });
    ui->labelPlayerCount->setText(QStringLiteral("%1 joueurs").arg(playerModel->rowCount()));
}

DialogPlayerList::~DialogPlayerList()
{
    delete playerModel;
    delete ui;
}

void DialogPlayerList::on_pushButtonAdd_clicked()
{
    if (serie->get_isDouble())
    {
        if (tournament)
        {
            QMenu menu;
            menu.addAction("Depuis la base CDSLS", this, [this]() { addDouble(nullptr); });
            menu.addAction("Depuis les inscrits au tournoi", this, [this]() { addDouble(tournament->getTournamentPlayerModel()); });
            menu.addAction("Depuis une autre série", this, [this]() { addDoubleFromOtherSerie(); });
            menu.exec(ui->pushButtonAdd->mapToGlobal(QPoint(0, ui->pushButtonAdd->height())));
        }
        else
        {
            addDouble(nullptr);
        }
        return;
    }

    // If we have a tournament, show a menu with multiple sources
    if (tournament)
    {
        QMenu menu;
        menu.addAction("Depuis la base CDSLS", this, [this]()
        {
            DialogPlayers d(PlayerModel::Instance(), true);
            if (d.exec() == QDialog::Accepted)
            {
                auto selected = d.getSelectedList();
                for (auto p : selected)
                {
                    if (p && checkRanking(p->get_ranking()))
                        playerModel->appendClone(p);
                }
            }
        });
        menu.addAction("Depuis les inscrits au tournoi", this, [this]() { addFromRoster(); });
        menu.addAction("Depuis une autre série", this, [this]() { addFromOtherSerie(); });
        menu.exec(ui->pushButtonAdd->mapToGlobal(QPoint(0, ui->pushButtonAdd->height())));
        return;
    }

    DialogPlayers d(PlayerModel::Instance(), true);
    if (d.exec() == QDialog::Accepted)
    {
        auto selected = d.getSelectedList();
        for (auto p : selected)
        {
            if (p && checkRanking(p->get_ranking()))
            {
                qDebug() << "add player: " << p->get_firstName() << " " << p->get_lastName() << " " << p->get_club();
                playerModel->appendClone(p);
            }
        }
    }
}

void DialogPlayerList::on_pushButtonRemove_clicked()
{
    auto indexes = ui->treeView->selectionModel()->selectedIndexes();
    if (indexes.isEmpty())
        return;
    auto idx = filterModel->indexToSource(indexes.at(0).row());

    auto ret = QMessageBox::question(this, "Supprimer", "Supprimer le joueur sélectionné ?");
    if (ret != QMessageBox::Yes)
        return;

    playerModel->removeRow(idx);
}

void DialogPlayerList::on_buttonBox_accepted()
{
    serie->replaceAllPlayers(playerModel);

    // Auto-add players to tournament roster if not already there
    if (tournament)
    {
        auto *roster = tournament->getTournamentPlayerModel();
        for (int i = 0; i < playerModel->rowCount(); i++)
        {
            auto *p = playerModel->item(i);
            if (!roster->getFromLicense(p->get_license()))
                roster->appendClone(p);
            // For doubles, also add the second player
            if (!p->get_licenseSecond().isEmpty() &&
                !roster->getFromLicense(p->get_licenseSecond()))
            {
                QJsonObject p2Json;
                p2Json.insert("firstname", p->get_firstNameSecond());
                p2Json.insert("lastname", p->get_lastNameSecond());
                p2Json.insert("license", p->get_licenseSecond());
                p2Json.insert("ranking", p->get_rankingSecond());
                p2Json.insert("club", p->get_clubSecond());
                p2Json.insert("license_valid", p->get_licenseValidSecond());
                roster->loadPlayer(p2Json);
            }
        }
    }

    accept();
}

bool DialogPlayerList::checkRanking(QString rank)
{
    QString maxRank = serie->get_ranking();

    if (maxRank.toLower() == "open")
        return true; //always accept all players for open series

    int max, player_rank;
    if (maxRank.toLower() == "nc")
        max = 100;
    else
    {
        bool v;
        max = maxRank.toInt(&v);
        if (!v)
        {
            qDebug() << "Failed to convert ranking: " << maxRank;
            return true;
        }
    }

    if (rank.toLower() == "nc")
        player_rank = 100;
    else
    {
        bool v;
        player_rank = rank.toInt(&v);
        if (!v)
        {
            qDebug() << "Failed to convert ranking: " << rank;
            return true;
        }
    }

    return player_rank >= max;
}

void DialogPlayerList::addFromRoster()
{
    if (!tournament) return;

    auto *roster = tournament->getTournamentPlayerModel();
    DialogPlayers d(roster, true);
    d.setWindowTitle("Sélectionner depuis le roster du tournoi");
    if (d.exec() == QDialog::Accepted)
    {
        auto selected = d.getSelectedList();
        for (auto p : selected)
        {
            if (p && checkRanking(p->get_ranking()))
            {
                if (!playerModel->getFromLicense(p->get_license()))
                    playerModel->appendClone(p);
            }
        }
    }
}

void DialogPlayerList::addFromOtherSerie()
{
    if (!tournament) return;

    // Build list of other series
    QStringList serieNames;
    QList<int> serieIndexes;
    for (int i = 0; i < tournament->serieCount(); i++)
    {
        auto *s = tournament->getSerie(i);
        if (s->get_serieUid() == serie->get_serieUid()) continue;
        serieNames << QStringLiteral("%1 (%2 joueurs)").arg(s->get_name()).arg(s->getPlayerModel()->rowCount());
        serieIndexes << i;
    }

    if (serieNames.isEmpty())
    {
        QMessageBox::information(this, "Info", "Aucune autre série disponible.");
        return;
    }

    bool ok;
    QString choice = QInputDialog::getItem(this, "Sélectionner une série",
                                           "Importer les joueurs depuis:", serieNames, 0, false, &ok);
    if (!ok) return;

    int choiceIdx = serieNames.indexOf(choice);
    if (choiceIdx < 0 || choiceIdx >= serieIndexes.count()) return;

    auto *sourceSerie = tournament->getSerie(serieIndexes[choiceIdx]);
    auto *sourceModel = sourceSerie->getPlayerModel();

    // Open player selection dialog from source serie's players
    DialogPlayers d(sourceModel, true);
    d.setWindowTitle(QStringLiteral("Joueurs de \"%1\"").arg(sourceSerie->get_name()));
    if (d.exec() == QDialog::Accepted)
    {
        auto selected = d.getSelectedList();
        for (auto p : selected)
        {
            if (p && checkRanking(p->get_ranking()))
            {
                if (!playerModel->getFromLicense(p->get_license()))
                    playerModel->appendClone(p);
            }
        }
    }
}

void DialogPlayerList::addDouble(PlayerModel *source)
{
    DialogAddDoublePlayers d(source);
    if (d.exec() == QDialog::Accepted)
    {
        auto p1 = d.getPlayer1();
        auto p2 = d.getPlayer2();

        QJsonObject merged = p1->toJson();
        merged.insert("firstnameSecond", p2->get_firstName());
        merged.insert("lastnameSecond", p2->get_lastName());
        merged.insert("clubSecond", p2->get_club());
        merged.insert("licenseSecond", p2->get_license());
        merged.insert("rankingSecond", p2->get_ranking());
        merged.insert("license_validSecond", p2->get_licenseValid());

        playerModel->loadPlayer(merged);
    }
}

void DialogPlayerList::addDoubleFromOtherSerie()
{
    if (!tournament) return;

    // Build list of other double series only
    QStringList serieNames;
    QList<int> serieIndexes;
    for (int i = 0; i < tournament->serieCount(); i++)
    {
        auto *s = tournament->getSerie(i);
        if (s->get_serieUid() == serie->get_serieUid()) continue;
        if (!s->get_isDouble()) continue;
        serieNames << QStringLiteral("%1 (%2 équipes)").arg(s->get_name()).arg(s->getPlayerModel()->rowCount());
        serieIndexes << i;
    }

    if (serieNames.isEmpty())
    {
        QMessageBox::information(this, "Info", "Aucune autre série double disponible.");
        return;
    }

    bool ok;
    QString choice = QInputDialog::getItem(this, "Sélectionner une série",
                                           "Importer les équipes depuis:", serieNames, 0, false, &ok);
    if (!ok) return;

    int choiceIdx = serieNames.indexOf(choice);
    if (choiceIdx < 0 || choiceIdx >= serieIndexes.count()) return;

    auto *sourceSerie = tournament->getSerie(serieIndexes[choiceIdx]);
    auto *srcModel = sourceSerie->getPlayerModel();

    // Import double pairs directly
    for (int i = 0; i < srcModel->rowCount(); i++)
    {
        auto *p = srcModel->item(i);
        if (!playerModel->getFromLicense(p->get_license()))
            playerModel->appendClone(p);
    }
}
