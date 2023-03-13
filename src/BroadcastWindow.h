#ifndef BROADCASTWINDOW_H
#define BROADCASTWINDOW_H

#include <QQuickView>
#include <QWidget>
#include "qqmlhelpers.h"

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

public:
    explicit BroadcastWindow(QScreen *scr, bool fullscreen, Tournament *t, QWidget *parent = nullptr);
    ~BroadcastWindow();

signals:
    void windowClosed();

private slots:
    void quickViewStatusChanged(QQuickView::Status status);

private:
    BWQQuickView *view = nullptr;
    Tournament *currentTournament = nullptr;

    void loadQmlApp();
};

#endif // BROADCASTWINDOW_H
