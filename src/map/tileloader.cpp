/*************************************************************************
 *  Copyright (c) 2026 Anthony Vital <anthony.vital@gmail.com>           *
 *                                                                       *
 *  This file is part of Marmot.                                         *
 *                                                                       *
 *  Marmot is free software: you can redistribute it and/or modify       *
 *  it under the terms of the GNU General Public License as published by *
 *  the Free Software Foundation, either version 3 of the License, or    *
 *  (at your option) any later version.                                  *
 *                                                                       *
 *  Marmot is distributed in the hope that it will be useful,            *
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of       *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the        *
 *  GNU General Public License for more details.                         *
 *                                                                       *
 *  You should have received a copy of the GNU General Public License    *
 *  along with Marmot. If not, see <http://www.gnu.org/licenses/>.       *
 *************************************************************************/

#include "tileloader.h"

#include <QCoreApplication>
#include <QDateTime>
#include <QNetworkAccessManager>
#include <QNetworkDiskCache>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QPointer>
#include <QStandardPaths>
#include <QThreadPool>

namespace {
const qint64 DiskCacheSize = 512ll*1024*1024;
const int MaxAttempts = 3;
}

TileLoader::TileLoader(QObject *parent)
    : QObject(parent)
    , m_network(new QNetworkAccessManager(this))
    , m_cache(256)
{
    auto diskCache = new QNetworkDiskCache(this);
    diskCache->setCacheDirectory(QStandardPaths::writableLocation(QStandardPaths::CacheLocation)+QStringLiteral("/tiles"));
    diskCache->setMaximumCacheSize(DiskCacheSize);
    m_network->setCache(diskCache);
}

void TileLoader::setUserAgent(const QString &userAgent)
{
    m_userAgent = userAgent.toUtf8();
}

void TileLoader::setCapacity(int tiles)
{
    m_cache.setMaxCost(tiles);
}

QImage TileLoader::image(const TileKey &key) const
{
    const QImage *image = m_cache.object(key);
    return image ? *image : QImage();
}

void TileLoader::request(const TileKey &key, const QUrl &url)
{
    if (m_cache.contains(key) || m_downloads.contains(key) || m_decoding.contains(key))
        return;
    const auto failure = m_failures.constFind(key);
    if (failure != m_failures.constEnd()
            && (failure->count >= MaxAttempts
                || QDateTime::currentMSecsSinceEpoch()-failure->time < RetryDelay)) {
        return;
    }

    QNetworkRequest request(url);
    if (!m_userAgent.isEmpty())
        request.setHeader(QNetworkRequest::UserAgentHeader, m_userAgent);
    request.setAttribute(QNetworkRequest::CacheLoadControlAttribute, QNetworkRequest::PreferCache);
    QNetworkReply *reply = m_network->get(request);
    m_downloads.insert(key, reply);
    connect(reply, &QNetworkReply::finished, this, [this, key, reply]() {
        downloadFinished(key, reply);
    });
}

void TileLoader::cancelAllExcept(const QSet<TileKey> &wanted)
{
    QList<QNetworkReply *> unwanted;
    for (auto it = m_downloads.begin(); it != m_downloads.end();) {
        if (wanted.contains(it.key())) {
            ++it;
        } else {
            unwanted << it.value();
            it = m_downloads.erase(it);
        }
    }
    // abort() emits finished(), which must not find the reply in m_downloads anymore.
    for (QNetworkReply *reply : std::as_const(unwanted))
        reply->abort();
}

void TileLoader::downloadFinished(const TileKey &key, QNetworkReply *reply)
{
    reply->deleteLater();
    if (m_downloads.value(key) != reply)
        return; // cancelled
    m_downloads.remove(key);

    if (reply->error() != QNetworkReply::NoError) {
        setFailed(key);
        return;
    }

    m_decoding.insert(key);
    const QByteArray data = reply->readAll();
    const QPointer<TileLoader> self(this);
    QThreadPool::globalInstance()->start([self, key, data]() {
        QImage image = QImage::fromData(data);
        if (!image.isNull()) {
            image = image.convertToFormat(image.hasAlphaChannel() ? QImage::Format_RGBA8888_Premultiplied
                                                                  : QImage::Format_RGBX8888);
        }
        QMetaObject::invokeMethod(qApp, [self, key, image]() {
            if (self)
                self->decodeFinished(key, image);
        }, Qt::QueuedConnection);
    });
}

void TileLoader::decodeFinished(const TileKey &key, const QImage &image)
{
    m_decoding.remove(key);
    if (image.isNull()) {
        setFailed(key);
        return;
    }
    m_failures.remove(key);
    m_cache.insert(key, new QImage(image));
    Q_EMIT(tileLoaded(key));
}

void TileLoader::setFailed(const TileKey &key)
{
    Failure &failure = m_failures[key];
    failure.time = QDateTime::currentMSecsSinceEpoch();
    ++failure.count;
    Q_EMIT(tileFailed(key));
}
