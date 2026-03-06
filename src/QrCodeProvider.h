#ifndef QRCODEPROVIDER_H
#define QRCODEPROVIDER_H

#include <QQuickImageProvider>
#include <QImage>
#include <QMutex>
#include <QMutexLocker>
#include <QMap>
#include "qrcodegen.hpp"

class QrCodeProvider : public QQuickImageProvider
{
public:
    QrCodeProvider()
        : QQuickImageProvider(QQuickImageProvider::Image)
    {}

    void setBaseInfo(const QString &baseUrl, const QString &key)
    {
        QMutexLocker locker(&mutex);
        m_baseUrl = baseUrl;
        m_key = key;
        m_cache.clear();
    }

    void clear()
    {
        QMutexLocker locker(&mutex);
        m_baseUrl.clear();
        m_key.clear();
        m_cache.clear();
    }

    /// id format (from QML Image.source):
    ///   "home"                    -> deep link "/"
    ///   "serie/INDEX"             -> deep link "/serie/INDEX"
    ///   "serie/INDEX/round/ROUND" -> deep link "/serie/INDEX?round=ROUND"
    ///   "podium/INDEX"            -> deep link "/podium/INDEX"
    QImage requestImage(const QString &id, QSize *size, const QSize &requestedSize) override
    {
        Q_UNUSED(requestedSize)
        QMutexLocker locker(&mutex);

        if (m_cache.contains(id)) {
            if (size) *size = m_cache[id].size();
            return m_cache[id];
        }

        QString deepLink;
        if (id == "home")
            deepLink = "/";
        else if (id.startsWith("serie/") && id.contains("/round/")) {
            auto parts = id.split("/round/");
            deepLink = "/" + parts[0] + "?round=" + parts[1];
        } else
            deepLink = "/" + id;

        QString fullUrl = m_baseUrl + "#K=" + m_key + deepLink;

        QImage img = generateQr(fullUrl);
        m_cache[id] = img;

        if (size) *size = img.size();
        return img;
    }

private:
    QImage generateQr(const QString &url) const
    {
        using namespace qrcodegen;
        QrCode qr = QrCode::encodeText(url.toUtf8().constData(), QrCode::Ecc::MEDIUM);

        int sz = qr.getSize();
        int border = 4;
        int scale = 8;
        int imgSize = (sz + border * 2) * scale;

        QImage image(imgSize, imgSize, QImage::Format_RGB32);
        image.fill(Qt::white);

        for (int y = 0; y < sz; y++) {
            for (int x = 0; x < sz; x++) {
                if (qr.getModule(x, y)) {
                    for (int dy = 0; dy < scale; dy++) {
                        for (int dx = 0; dx < scale; dx++) {
                            image.setPixel(
                                (x + border) * scale + dx,
                                (y + border) * scale + dy,
                                qRgb(0, 0, 0)
                            );
                        }
                    }
                }
            }
        }
        return image;
    }

    QString m_baseUrl;
    QString m_key;
    QMap<QString, QImage> m_cache;
    QMutex mutex;
};

#endif // QRCODEPROVIDER_H
