/*************************************************************************
 *  Copyright (c) 2019 Anthony Vital <anthony.vital@gmail.com>           *
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

#include "file.h"
#include "poi.h"

#include <QFile>
#include <QCoreApplication>
#include <QFileInfo>
#include <QUrl>
#include <QXmlStreamReader>
#include <QDateTime>
#include <QGeoPolygon>
#include <QDebug>

File::File(QObject *parent)
    : QObject(parent)
    , m_climb(0.0)
    , m_altitudeMax(0.0)
    , m_altitudeMin(0.0)
    , m_distance3D(0.0)
    , m_distance2D(0.0)
{
    connect(this,&File::tracksChanged, &File::updateBoundingBox);
}

bool File::open(const QString &fileName)
{
    QUrl url(fileName);
    QString path = url.scheme().isEmpty() ? fileName : url.toLocalFile();
    QFile file(path);
    // Which format?
    if(!file.open(QFile::ReadOnly | QFile::Text)) {
        qDebug() << QLatin1String("Cannot read file") << file.errorString();
        return false;
    }
    QXmlStreamReader reader(&file);
    if (reader.readNextStartElement()) {
        if (reader.name() == QLatin1String("kml")) {
            parseKml(&reader);
        } else if (reader.name() == QLatin1String("gpx")) {
            parseGpx(&reader);
        } else {
            qDebug()<<QLatin1String("Unknowned file type: ")<<reader.name();
            file.close();
            return false;
        }
    }
    file.close();
    if (reader.hasError()) {
        qDebug()<<reader.errorString();
        return false;
    }
    m_name = url.fileName();
    Q_EMIT(opened());
    Q_EMIT(climbChanged());
    Q_EMIT(altitudeMaxChanged());
    Q_EMIT(altitudeMinChanged());
    Q_EMIT(distance3DChanged());
    Q_EMIT(distance2DChanged());

    return true;
}

void File::parseKml(QXmlStreamReader *reader)
{
    QString name;
    QString description;
    bool parsingPlacemark = false;
    bool parsingPoint = false;
    bool parsingLineString = false;
    while (reader->readNext() != QXmlStreamReader::EndDocument) {
        if (reader->isStartElement()) {
            if (reader->name() == QLatin1String("Placemark")) {
                parsingPlacemark = true;
            }
            else if (parsingPlacemark && reader->name() == QLatin1String("name")) {
                name = reader->readElementText();
            }
            else if (parsingPlacemark && reader->name() == QLatin1String("description")) {
                description = reader->readElementText();
            }
            else if (reader->name() == QLatin1String("Point")) {
                parsingPoint = true;
            }
            else if (reader->name() == QLatin1String("LineString")) {
                parsingLineString = true;
            }
            else if (reader->name() == QLatin1String("coordinates")) {
                if (parsingPoint) {
                    QStringList coords = reader->readElementText().split(',');
                    if (coords.length() != 3) return;
                    addPoi(QGeoCoordinate(coords.at(1).toDouble(), coords.at(0).toDouble(), coords.at(2).toDouble()), name, description);
                }
                else if (parsingLineString) {
                    Track *track = new Track(this);
                    track->setName(name);
                    track->setDescription(description);
                    QStringList points = reader->readElementText().split(' ');
                    for (int i = 0; i < points.length(); ++i) {
                        QStringList coords = points[i].split(',');
                        if (coords.length() != 3) break;
                        track->addPoint(QGeoCoordinate(coords.at(1).toDouble(), coords.at(0).toDouble(), coords.at(2).toDouble()));
                    }
                    track->computeStatistics();
                    track->updateBoundingBox();
                    appendTrack(track);
                }
            }
        } else if (reader->isEndElement()) {
            if (reader->name() == QLatin1String("Placemark")) {
                parsingPlacemark = false;
            }
            else if (reader->name() == QLatin1String("Point")) {
                parsingPoint = false;
            }
            else if (reader->name() == QLatin1String("LineString")) {
                parsingLineString = false;
            }
        }
    }
    Q_EMIT(tracksChanged());
}

void File::parseGpx(QXmlStreamReader *reader)
{
    QString name;
    QString description;
    bool parsingTrk = false;
    bool parsingTrkPoint = false;
    bool parsingWayPoint = false;
    double latitude = 0.0;
    double longitude = 0.0;
    double elevation = 0.0;
    bool hasTime = false;
    QString firstTime;
    QString lastTime;
    Track *track = nullptr;
    while (reader->readNext() != QXmlStreamReader::EndDocument) {
        if (reader->isStartElement()) {
            if (reader->name() == QLatin1String("extension")) {
                reader->skipCurrentElement();
            }
            if (reader->name() == QLatin1String("rte")) {
                reader->skipCurrentElement();
            }
            if (reader->name() == QLatin1String("wpt")) {
                parsingWayPoint = true;
                latitude = reader->attributes().value("lat").toDouble();
                longitude = reader->attributes().value("lon").toDouble();
            }
            else if (parsingWayPoint && reader->name() == QLatin1String("ele")) {
                elevation = reader->readElementText().toDouble();
            }
            else if (parsingWayPoint && reader->name() == QLatin1String("name")) {
                name = reader->readElementText();
            }
            else if (parsingWayPoint && reader->name() == QLatin1String("desc")) {
                description = reader->readElementText();
            }
            else if (reader->name() == QLatin1String("trk")) {
                parsingTrk = true;
                track = new Track(this);
            }
            else if (parsingTrk && !parsingTrkPoint && reader->name() == QLatin1String("name")) {
                name = reader->readElementText();
            }
            else if (parsingTrk && reader->name() == QLatin1String("desc")) {
                description = reader->readElementText();
            }
            else if (parsingTrk && reader->name() == QLatin1String("trkpt")) {
                parsingTrkPoint = true;
                latitude = reader->attributes().value("lat").toDouble();
                longitude = reader->attributes().value("lon").toDouble();
            }
            else if (parsingTrkPoint && reader->name() == QLatin1String("ele")) {
                elevation = reader->readElementText().toDouble();
            }
            else if (parsingTrkPoint && reader->name() == QLatin1String("time")) {
                hasTime = true;
                if (firstTime.isEmpty()) {
                    firstTime = reader->readElementText();
                } else {
                    lastTime = reader->readElementText();
                }
            }
        } else if (reader->isEndElement()) {
            if (reader->name() == QLatin1String("wpt")) {
                addPoi(QGeoCoordinate(latitude, longitude, elevation), name, description);

                elevation = 0.0;
                parsingWayPoint = false;
            }
            else if (reader->name() == QLatin1String("trkpt")) {
                track->addPoint(QGeoCoordinate(latitude, longitude, elevation), lastTime);

                elevation = 0.0;
                parsingTrkPoint = false;
            }
            else if (reader->name() == QLatin1String("trk")) {
                track->setName(name);
                track->setDescription(description);
                if (hasTime) {
                    QDateTime t1 = QDateTime::fromString(firstTime, Qt::ISODate);
                    QDateTime t2 = QDateTime::fromString(lastTime, Qt::ISODate);
                    qint64 diff = t1.secsTo(t2);
                    track->setDuration(diff);

                    hasTime = false;
                    firstTime.clear();
                    lastTime.clear();
                }
                track->computeStatistics();
                track->updateBoundingBox();
                appendTrack(track);

                name = "";
                description = "";
                parsingTrk = false;
            }
        }
    }
    Q_EMIT(tracksChanged());
}

void File::addPoi(const QGeoCoordinate &coordinate, const QString &name, const QString &description)
{
    Poi *poi = new Poi(this);
    poi->setName(name);
    poi->setDescription(description);
    poi->setCoordinate(coordinate);
    m_pois.append(poi);
    Q_EMIT(poiChanged());
}

void File::removePoi(int index)
{
    m_pois.at(index)->deleteLater();
    m_pois.removeAt(index);
}

void File::addTrack()
{
    Track *track = new Track(this);
    appendTrack(track);
    Q_EMIT(tracksChanged());
}

void File::exportToGpx(const QString &fileName) const
{
    QUrl url(fileName);
    QString path = url.scheme().isEmpty() ? fileName : url.toLocalFile();
    QFile file(path);
    if(!file.open(QIODevice::WriteOnly | QIODevice::Text)){
        qDebug() << QLatin1String("Cannot write file") << file.errorString();
        return;
    }
    QXmlStreamWriter writer(&file);
    writer.setAutoFormatting(true);
    writer.setAutoFormattingIndent(2);
    writer.writeStartDocument();
    writer.writeDefaultNamespace(QStringLiteral("http://www.topografix.com/GPX/1/1"));
    writer.writeStartElement(QStringLiteral("gpx"));
    writer.writeAttribute(QStringLiteral("creator"), QCoreApplication::applicationName());
    writer.writeAttribute(QStringLiteral("version"), QStringLiteral("1.1"));
    writer.writeAttribute("xmlns:xsi", "http://www.w3.org/2001/XMLSchema-instance");
    writer.writeAttribute("xsi:schemaLocation", "http://www.topografix.com/GPX/1/1 http://www.topografix.com/GPX/1/1/gpx.xsd");
    writer.writeStartElement(QStringLiteral("metadata"));
    QFileInfo fileInfo(file);
    writer.writeTextElement(QStringLiteral("name"), fileInfo.completeBaseName());
    writer.writeTextElement(QStringLiteral("time"), QDateTime::currentDateTimeUtc().toString(Qt::ISODate));
    writer.writeEndElement();
    for (const auto &p : m_pois) {
        Poi *poi = static_cast<Poi*>(p);
        writer.writeStartElement(QStringLiteral("wpt"));
        writer.writeAttribute(QStringLiteral("lat"), QString::number(poi->coordinate().latitude(), 'f', QLocale::FloatingPointShortest));
        writer.writeAttribute(QStringLiteral("lon"), QString::number(poi->coordinate().longitude(), 'f', QLocale::FloatingPointShortest));
        if (!qIsNaN(poi->coordinate().altitude()))
            writer.writeTextElement(QStringLiteral("ele"), QString::number(poi->coordinate().altitude(), 'f', QLocale::FloatingPointShortest));
        writer.writeTextElement(QStringLiteral("name"), poi->name());
        writer.writeEndElement(); // wpt
    }
    for (const auto &l : m_tracks) {
        Track *track = static_cast<Track*>(l);
        writer.writeStartElement(QStringLiteral("trk"));
        QString name = track->name();
        if (!name.isEmpty())
            writer.writeTextElement(QStringLiteral("name"), fileInfo.fileName().chopped(4));
        writer.writeStartElement(QStringLiteral("trkseg"));
        for (int i=0; i < track->length(); ++i) {
            writer.writeStartElement(QStringLiteral("trkpt"));
            QGeoCoordinate coordinate = track->path().at(i);
            writer.writeAttribute(QStringLiteral("lat"), QString::number(coordinate.latitude(), 'f', QLocale::FloatingPointShortest));
            writer.writeAttribute(QStringLiteral("lon"), QString::number(coordinate.longitude(), 'f', QLocale::FloatingPointShortest));
            writer.writeTextElement(QStringLiteral("ele"), QString::number(coordinate.altitude(), 'f', QLocale::FloatingPointShortest));
            QString timeStamp = track->timeStamps().at(i);
            if (!timeStamp.isEmpty())
                writer.writeTextElement(QStringLiteral("time"), timeStamp);
            writer.writeEndElement(); // trkpt
        }
        writer.writeEndElement(); // trkseg
        writer.writeEndElement(); // trk
    }
    writer.writeEndElement(); // gpx
    writer.writeEndDocument();

    file.close();
    if (writer.hasError()) {
        qDebug()<<QLatin1String("Error writing ")+fileName;
        return;
    }
}

void File::appendTrack(Track *track)
{
    m_climb += track->climb();
    connect(track, &Track::climbChanged, this, [=](){
        m_climb = 0.0;
        for (auto trck : m_tracks) {
            m_climb += static_cast<Track*>(trck)->climb();
        }
        Q_EMIT(climbChanged());
    });
    m_altitudeMax = qMax(m_altitudeMax, track->altitudeMax());
    connect(track, &Track::altitudeMaxChanged, this, [=](){
        m_altitudeMax = track->altitudeMax();
        for (auto trck : m_tracks) {
            m_altitudeMax = qMax(m_altitudeMax, static_cast<Track*>(trck)->altitudeMax());
        }
        Q_EMIT(altitudeMaxChanged());
    });
    if (m_tracks.isEmpty()) {
        m_altitudeMin = track->altitudeMin();
    } else {
        m_altitudeMin = qMin(m_altitudeMin, track->altitudeMin());
    }
    connect(track, &Track::altitudeMinChanged, this, [=](){
        m_altitudeMin = track->altitudeMin();
        for (auto trck : m_tracks) {
            m_altitudeMin = qMin(m_altitudeMin, static_cast<Track*>(trck)->altitudeMin());
        }
        Q_EMIT(altitudeMinChanged());
    });
    m_distance3D += track->distance3D();
    connect(track, &Track::distance3DChanged, this, [=](){
        m_distance3D = 0.0;
        for (auto trck : m_tracks) {
            m_distance3D += static_cast<Track*>(trck)->distance3D();
        }
        Q_EMIT(distance3DChanged());
    });
    m_distance2D += track->distance2D();
    connect(track, &Track::distance2DChanged, this, [=](){
        m_distance2D = 0.0;
        for (auto trck : m_tracks) {
            m_distance2D += static_cast<Track*>(trck)->distance2D();
        }
        Q_EMIT(distance2DChanged());
    });
    m_tracks.append(track);
}

void File::updateBoundingBox()
{
    QGeoRectangle bbox;
    for (const auto &l : m_tracks) {
        Track *track = static_cast<Track*>(l);
        if (!bbox.isValid()) {
            bbox = track->boundingBox();
        } else {
            bbox.united(track->boundingBox());
        }
    }
    m_bbox = bbox;
}
