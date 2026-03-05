#ifndef BROADCASTPREVIEWPROVIDER_H
#define BROADCASTPREVIEWPROVIDER_H

#include <QQuickImageProvider>
#include <QImage>
#include <QMutex>
#include <QMutexLocker>

class BroadcastPreviewProvider : public QQuickImageProvider
{
public:
    BroadcastPreviewProvider()
        : QQuickImageProvider(QQuickImageProvider::Image)
    {}

    void updateImage(const QImage &img)
    {
        QMutexLocker locker(&mutex);
        currentImage = img;
    }

    QImage requestImage(const QString &id, QSize *size, const QSize &requestedSize) override
    {
        Q_UNUSED(id)
        Q_UNUSED(requestedSize)
        QMutexLocker locker(&mutex);
        if (size)
            *size = currentImage.size();
        return currentImage;
    }

private:
    QImage currentImage;
    QMutex mutex;
};

#endif // BROADCASTPREVIEWPROVIDER_H
