#include "DialogPlayersHtml.h"
#include "ui_DialogPlayersHtml.h"
#include <QStringConverter>

DialogPlayersHtml::DialogPlayersHtml(Tournament *t, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DialogPlayersHtml),
    tournament(t)
{
    ui->setupUi(this);

    ui->textEdit->setHtml(buildHtml());
}

DialogPlayersHtml::~DialogPlayersHtml()
{
    delete ui;
}

QString DialogPlayersHtml::readFile(QString f)
{
    QFile file(f);
    if (!file.open(QFile::ReadOnly))
        return QString();

    QByteArray data = file.readAll();
    auto decoder = QStringDecoder(QStringDecoder::Utf8);
    return decoder(data);
}

QString DialogPlayersHtml::buildHtml()
{
    QString html;

    QString header = readFile(":/data/header.html");
    QString footer = readFile(":/data/footer.html");
    QString table_header = readFile(":/data/table_header.html");
    QString table_item = readFile(":/data/table_item.html");
    QString table_footer = readFile(":/data/table_footer.html");

    html += header;

    QSet<Player *> s;
    QList<Player *> allPlayers;
    for (int i = 0;i < tournament->serieCount();i++)
    {
        auto serie = tournament->getSerie(i);
        auto players = reinterpret_cast<PlayerModel *>(serie->get_players());
        for (int j = 0;j < players->rowCount();j++)
        {
            auto p = players->item(j);

            auto pA = PlayerModel::Instance()->getFromLicense(p->get_license());
            if (pA && !s.contains(pA))
            {
                s.insert(pA);
                allPlayers.append(pA);
            }

            auto pB = PlayerModel::Instance()->getFromLicense(p->get_licenseSecond());
            if (pB && !s.contains(pB))
            {
                s.insert(pB);
                allPlayers.append(pB);
            }
        }
    }

    QCollator sorter;
    std::sort(allPlayers.begin(), allPlayers.end(),
              [&sorter](Player *a, Player *b)
    {
        return sorter.compare(a->get_club(), b->get_club()) < 0;
    });

    html += table_header.arg("Lic.", "Prénom", "Nom", "Class.", "Club");

    for (int i = 0;i < allPlayers.count();i++)
    {
        auto p = allPlayers.at(i);
        html += table_item.arg(p->get_license(), p->get_firstName(), p->get_lastName(), p->get_ranking(), p->get_club());
    }

    html += table_footer;
    html += footer;
    return html;
}
