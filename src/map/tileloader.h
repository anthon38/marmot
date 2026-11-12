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

#ifndef TILELOADER_H
#define TILELOADER_H

#include <QObject>
#include <QCache>
#include <QHash>
#include <QImage>
#include <QSet>
#include <QUrl>

class QNetworkAccessManager;
class QNetworkReply;

struct TileKey
{
    int type = 0;
    int zoom = 0;
    int x = 0;
    int y = 0;

    bool operator==(const TileKey &other) const
    {
        return type == other.type && zoom == other.zoom && x == other.x && y == other.y;
    }
};

inline size_t qHash(const TileKey &key, size_t seed = 0)
{
    return qHashMulti(seed, key.type, key.zoom, key.x, key.y);
}

Q_DECLARE_METATYPE(TileKey)

// Downloads raster tiles (through an HTTP disk cache), decodes them off the
// GUI thread and keeps the most recently used ones in memory.
class TileLoader : public QObject
{
    Q_OBJECT

public:
    explicit TileLoader(QObject *parent = nullptr);

    void setUserAgent(const QString &userAgent);
    // Number of decoded tiles kept in memory.
    void setCapacity(int tiles);

    // Both of these mark the tile as recently used.
    bool contains(const TileKey &key) const {return m_cache.object(key) != nullptr;}
    // The decoded tile, or a null image if it is not in memory.
    QImage image(const TileKey &key) const;
    // Starts loading a tile, unless it is available, on its way, or failed recently.
    void request(const TileKey &key, const QUrl &url);
    // Aborts the downloads which are not part of the given set.
    void cancelAllExcept(const QSet<TileKey> &wanted);

    static constexpr int RetryDelay = 10000; // ms

Q_SIGNALS:
    void tileLoaded(const TileKey &key);
    void tileFailed(const TileKey &key);

private:
    struct Failure {
        qint64 time = 0;
        int count = 0;
    };

    void downloadFinished(const TileKey &key, QNetworkReply *reply);
    void decodeFinished(const TileKey &key, const QImage &image);
    void setFailed(const TileKey &key);

    QNetworkAccessManager *m_network;
    QByteArray m_userAgent;
    mutable QCache<TileKey, QImage> m_cache;
    QHash<TileKey, QNetworkReply *> m_downloads;
    QSet<TileKey> m_decoding;
    QHash<TileKey, Failure> m_failures;
};

#endif // TILELOADER_H
