#ifndef BROADCASTWINDOW_H
#define BROADCASTWINDOW_H

#include <QQuickView>
#include <QWidget>
#include "qqmlhelpers.h"
#include <QQmlObjectListModel.h>
#include "BroadcastModel.h"
#include <functional>

class Tournament;

class BWQQuickView: public QQuickView
{
    Q_OBJECT
public:
    bool event(QEvent *event) override
    {
        if (event->type() == QEvent::Close)
            emit windowClosed();
        return QQuickView::event(event);
    }

signals:
    void windowClosed();
};

class BroadcastWindow : public QWidget
{
    Q_OBJECT
    QML_READONLY_PROPERTY(bool, fullscreen)
    QML_READONLY_PROPERTY(QScreen *, screen)
    QML_READONLY_PROPERTY(QObject *, views)
    QML_READONLY_PROPERTY(int, currentViewIndex)

public:
    explicit BroadcastWindow(QScreen *scr, bool fullscreen, Tournament *t, QWidget *parent = nullptr);
    ~BroadcastWindow();

signals:
    void windowClosed();

public slots:
    void nextView();
    void previousView();
    Q_INVOKABLE void setCurrentViewTimer(int ms);

public:
    void grabPreview(int maxWidth, std::function<void(const QImage &)> callback);

private slots:
    void quickViewStatusChanged(QQuickView::Status status);
    void reloadViews();
    void timerViewTick();

private:
    BWQQuickView *view = nullptr;
    Tournament *currentTournament = nullptr;
    QQmlObjectListModel<BroadcastView> *views;
    QTimer *timerViewChange = nullptr;

    void loadQmlApp();
    int getCurrentViewTime();
};

#endif // BROADCASTWINDOW_H
