#include "BroadcastModel.h"
#include "TSerie.h"

BroadcastView::BroadcastView(QObject *parent):
    QObject{parent}
{
    update_viewSerie(nullptr);
    set_viewVisible(true);

    connect(this, &BroadcastView::viewSerieChanged, this, [=]()
    {
        auto s = dynamic_cast<TSerie *>(get_viewSerie());
        if (s)
            update_name(s->get_name());
    });
}
