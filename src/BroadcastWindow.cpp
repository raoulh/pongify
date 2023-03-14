#include "BroadcastWindow.h"
#include <QScreen>
#include <QQuickStyle>
#include <QQmlContext>
#include "TStorage.h"
#include "PlayerModel.h"
#include "Tournament.h"
#include <QMessageBox>
#include <QTimer>

BroadcastWindow::BroadcastWindow(QScreen *scr, bool fullscreen, Tournament *t, QWidget *parent):
    QWidget{parent},
    currentTournament(t)
{
    views = new QQmlObjectListModel<BroadcastView>(this, "view");
    update_views(views);
    update_currentViewIndex(0);

    timerViewChange = new QTimer(this);
    connect(timerViewChange, &QTimer::timeout, this, &BroadcastWindow::timerViewTick);
    timerViewChange->start(10000); //TODO make it configurable?

    connect(currentTournament, &Tournament::seriesStatusChanged, this, &BroadcastWindow::reloadViews);
    reloadViews();

    update_fullscreen(fullscreen);
    update_screen(scr);
    loadQmlApp();
}

BroadcastWindow::~BroadcastWindow()
{
    delete view;
    delete views;
}

void BroadcastWindow::nextView()
{
    int curr = get_currentViewIndex();
    curr++;
    if (curr >= views->count())
        curr = 0;
    update_currentViewIndex(curr);

    timerViewChange->stop();
    timerViewChange->start();
}

void BroadcastWindow::previousView()
{
    int curr = get_currentViewIndex();
    curr--;
    if (curr < 0)
        curr = views->count() - 1;
    update_currentViewIndex(curr);

    timerViewChange->stop();
    timerViewChange->start();
}

void BroadcastWindow::quickViewStatusChanged(QQuickView::Status status)
{
    if (status == QQuickView::Error)
    {
        qDebug() << "Failed to load BroadcastWindow.qml:";
        for (const auto &e: view->errors())
            qDebug() << e.toString();

        QMessageBox::warning(nullptr, "Erreur", "Impossible d'ouvrir la vue de diffusion :(");
    }
}

void BroadcastWindow::reloadViews()
{
    views->clear();

    auto v = new BroadcastView(this);
    v->update_viewUrl("qrc:/qml/broadcast/DefaultView.qml");
    v->update_name("Information tournoi");
    views->append(v);

    for (int i = 0;i < currentTournament->serieCount();i++)
    {
        auto s = currentTournament->getSerie(i);

        if (s->get_status() == "playing" && s->get_tournamentType() == "single")
        {
            auto v = new BroadcastView(this);
            v->update_viewUrl("qrc:/qml/broadcast/SerieBracketView.qml");
            v->update_viewSerie(s);
            views->append(v);
        }
        else if (s->get_status() == "playing" && s->get_tournamentType() == "roundrobin")
        {
            auto v = new BroadcastView(this);
            v->update_viewUrl("qrc:/qml/broadcast/RoundRobinView.qml");
            v->update_viewSerie(s);
            views->append(v);
        }
        else if (s->get_status() == "finished")
        {
            auto v = new BroadcastView(this);
            v->update_viewUrl("qrc:/qml/broadcast/HallOfFameView.qml");
            v->update_viewSerie(s);
            views->append(v);
        }
    }
}

void BroadcastWindow::timerViewTick()
{
    nextView();
}

void BroadcastWindow::loadQmlApp()
{
    view = new BWQQuickView();
    view->setResizeMode(QQuickView::SizeRootObjectToView);

    connect(view, &BWQQuickView::statusChanged, this, &BroadcastWindow::quickViewStatusChanged);
    connect(view, &BWQQuickView::windowClosed, this, &BroadcastWindow::windowClosed);

    view->rootContext()->setContextProperty("broadcastWindow", this);
    view->rootContext()->setContextProperty("storage", TStorage::Instance());
    view->rootContext()->setContextProperty("playerModel", PlayerModel::Instance());
    view->rootContext()->setContextProperty("currentTournament", currentTournament);

    view->setTitle("Pongify - Vue spectateur");
    view->setSource(QUrl("qrc:///qml/BroadcastWindow.qml"));
    view->show();

    //Move to screen
    auto screenRect = get_screen()->geometry();
    view->setX(screenRect.x() + 100);
    view->setY(screenRect.y() + 100);

    if (get_fullscreen())
        view->showFullScreen();
    else
        view->showMaximized();
}
