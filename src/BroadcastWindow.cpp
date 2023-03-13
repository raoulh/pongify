#include "BroadcastWindow.h"
#include <QScreen>
#include <QQuickStyle>
#include <QQmlContext>
#include "TStorage.h"
#include "PlayerModel.h"
#include "Tournament.h"
#include <QMessageBox>

BroadcastWindow::BroadcastWindow(QScreen *scr, bool fullscreen, Tournament *t, QWidget *parent):
    QWidget{parent},
    currentTournament(t)
{
    update_fullscreen(fullscreen);
    update_screen(scr);
    loadQmlApp();
}

BroadcastWindow::~BroadcastWindow()
{
    delete view;
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
    view->setX(screenRect.x());
    view->setY(screenRect.y());

    if (get_fullscreen())
    {
        view->showFullScreen();
    }
    else
    {
        view->showMaximized();
    }
}
