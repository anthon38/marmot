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

#include "maptype.h"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QJsonDocument>
#include <QJsonObject>
#include <QDebug>

#include <algorithm>

namespace {

// Same order as the map types of the QtLocation OSM plugin.
const QStringList &knownKeys()
{
    static const QStringList keys = {
        QStringLiteral("street"), QStringLiteral("satellite"), QStringLiteral("cycle"),
        QStringLiteral("transit"), QStringLiteral("night-transit"), QStringLiteral("terrain"),
        QStringLiteral("hiking")
    };
    return keys;
}

QString displayName(const QString &key)
{
    if (key == QLatin1String("street"))
        return MapType::tr("Street Map");
    if (key == QLatin1String("satellite"))
        return MapType::tr("Satellite Map");
    if (key == QLatin1String("cycle"))
        return MapType::tr("Cycle Map");
    if (key == QLatin1String("transit"))
        return MapType::tr("Transit Map");
    if (key == QLatin1String("night-transit"))
        return MapType::tr("Night Transit Map");
    if (key == QLatin1String("terrain"))
        return MapType::tr("Terrain Map");
    if (key == QLatin1String("hiking"))
        return MapType::tr("Hiking Map");
    return key.left(1).toUpper()+key.mid(1);
}

int sortRank(const QString &key)
{
    const int index = knownKeys().indexOf(key);
    return index < 0 ? knownKeys().length() : index;
}

}

MapType::MapType(const QString &key, const QString &urlTemplate, int maximumZoomLevel,
                 const QString &copyright, QObject *parent)
    : QObject(parent)
    , m_key(key)
    , m_name(displayName(key))
    , m_urlTemplate(urlTemplate)
    , m_copyright(copyright)
    , m_maximumZoomLevel(maximumZoomLevel)
{
    static int nextId = 0;
    m_id = nextId++;
}

QList<MapType *> MapType::load(const QString &directory, QObject *parent)
{
    QList<MapType *> types;

    if (!directory.isEmpty()) {
        const QFileInfoList files = QDir(directory).entryInfoList(QDir::Files | QDir::Readable);
        for (const QFileInfo &info : files) {
            QFile file(info.filePath());
            if (!file.open(QIODevice::ReadOnly))
                continue;
            QJsonParseError error;
            const QJsonObject json = QJsonDocument::fromJson(file.readAll(), &error).object();
            const QString urlTemplate = json.value(QLatin1String("UrlTemplate")).toString();
            if (urlTemplate.isEmpty()) {
                qWarning() << "Ignoring invalid tile provider" << info.filePath() << error.errorString();
                continue;
            }
            if (!json.value(QLatin1String("Enabled")).toBool(true))
                continue;
            QStringList credits;
            const QString mapCredit = json.value(QLatin1String("MapCopyRight")).toString();
            if (!mapCredit.isEmpty())
                credits << tr("Map © %1").arg(mapCredit);
            const QString dataCredit = json.value(QLatin1String("DataCopyRight")).toString();
            if (!dataCredit.isEmpty())
                credits << tr("Data © %1").arg(dataCredit);
            types << new MapType(info.completeBaseName(), urlTemplate,
                                 json.value(QLatin1String("MaximumZoomLevel")).toInt(19),
                                 credits.join(QStringLiteral(" | ")), parent);
        }
    }

    const bool hasStreet = std::any_of(types.cbegin(), types.cend(), [](const MapType *type) {
        return type->key() == QLatin1String("street");
    });
    if (!hasStreet) {
        types << new MapType(QStringLiteral("street"),
                             QStringLiteral("https://tile.openstreetmap.org/%z/%x/%y.png"), 19,
                             tr("Map © %1").arg(QStringLiteral("<a href='https://www.openstreetmap.org/copyright'>OpenStreetMap</a> contributors")),
                             parent);
    }

    std::stable_sort(types.begin(), types.end(), [](const MapType *a, const MapType *b) {
        const int rankA = sortRank(a->key());
        const int rankB = sortRank(b->key());
        return rankA != rankB ? rankA < rankB : a->key() < b->key();
    });
    return types;
}

QUrl MapType::tileUrl(int zoom, int x, int y) const
{
    QString url = m_urlTemplate;
    url.replace(QLatin1String("%z"), QString::number(zoom));
    url.replace(QLatin1String("%x"), QString::number(x));
    url.replace(QLatin1String("%y"), QString::number(y));
    url.replace(QLatin1String("{z}"), QString::number(zoom));
    url.replace(QLatin1String("{x}"), QString::number(x));
    url.replace(QLatin1String("{y}"), QString::number(y));
    return QUrl(url);
}
