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

    connect(currentTournament, &Tournament::timeBroadcastChangeChanged, this, [this]()
    {
        for (auto v: views->toList())
            v->set_viewTime(currentTournament->get_timeBroadcastChange());

        timerViewChange->stop();
        timerViewChange->start(getCurrentViewTime());
    });

    connect(currentTournament, &Tournament::seriesStatusChanged, this, &BroadcastWindow::reloadViews);
    reloadViews();

    timerViewChange->start(getCurrentViewTime());

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
    int old = curr;
    curr++;
    if (curr >= views->count())
        curr = 0;

    while (!views->at(curr)->get_viewVisible())
    {
        curr++;

        if (curr == old)
            return; //all views are disabled, break loop

        if (curr >= views->count())
            curr = 0;
    }

    update_currentViewIndex(curr);
    //timer will be restarted by the QML view via setCurrentViewTimer()
}

void BroadcastWindow::previousView()
{
    int curr = get_currentViewIndex();
    int old = curr;
    curr--;
    if (curr < 0)
        curr = views->count() - 1;

    while (!views->at(curr)->get_viewVisible())
    {
        curr--;

        if (curr == old)
            return; //all views are disabled, break loop

        if (curr < 0)
            curr = views->count() - 1;
    }

    update_currentViewIndex(curr);
    //timer will be restarted by the QML view via setCurrentViewTimer()
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
    v->set_viewVisible(currentTournament->get_defaultViewVisible());
    v->set_viewTime(10000);
    connect(currentTournament, &Tournament::defaultViewVisibleChanged, v, [this, v]()
    {
        v->set_viewVisible(currentTournament->get_defaultViewVisible());
    });
    views->append(v);

    for (int i = 0;i < currentTournament->serieCount();i++)
    {
        auto s = currentTournament->getSerie(i);

        if (s->get_status() == "playing" && s->get_tournamentType() == "single")
        {
            auto v = new BroadcastView(this);
            v->update_viewUrl("qrc:/qml/broadcast/SerieBracketView.qml");
            v->update_viewSerie(s);
            v->set_viewVisible(s->get_viewVisible());
            v->set_viewTime(currentTournament->get_timeBroadcastChange());
            connect(s, &TSerie::viewVisibleChanged, this, [v](bool visible)
            {
                v->set_viewVisible(visible);
            });
            views->append(v);
        }
        else if (s->get_status() == "playing" && s->get_tournamentType() == "roundrobin")
        {
            auto v = new BroadcastView(this);
            v->update_viewUrl("qrc:/qml/broadcast/RoundRobinView.qml");
            v->update_viewSerie(s);
            v->set_viewVisible(s->get_viewVisible());
            v->set_viewTime(currentTournament->get_timeBroadcastChange());
            connect(s, &TSerie::viewVisibleChanged, this, [v](bool visible)
            {
                v->set_viewVisible(visible);
            });
            views->append(v);
        }
        else if (s->get_status() == "finished")
        {
            auto v = new BroadcastView(this);
            v->update_viewUrl("qrc:/qml/broadcast/HallOfFameView.qml");
            v->update_viewSerie(s);
            v->set_viewVisible(s->get_viewVisible());
            v->set_viewTime(10000);
            connect(s, &TSerie::viewVisibleChanged, this, [v](bool visible)
            {
                v->set_viewVisible(visible);
            });
            views->append(v);
        }
    }
}

void BroadcastWindow::timerViewTick()
{
    nextView();
}

void BroadcastWindow::setCurrentViewTimer(int ms)
{
    int t = ms < 3000 ? 3000 : ms;
    timerViewChange->stop();
    timerViewChange->start(t);
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

int BroadcastWindow::getCurrentViewTime()
{
    if (views->count() == 0 || get_currentViewIndex() >= views->count())
        return 10000;
    int t = views->at(get_currentViewIndex())->get_viewTime();
    return t < 2000? 2000 : t;
}
