#include "track.h"

#include <QFontInfo>
#include <QGeoCoordinate>
#include <QtMath>
#include <QDateTime>
#include <QDebug>

#include <algorithm>

Track::Track(QObject *parent)
    : QObject(parent)
    , m_climb(0.0)
    , m_descent(0.0)
    , m_altitudeMax(0.0)
    , m_altitudeMin(0.0)
    , m_distance3D(0.0)
    , m_distance2D(0.0)
{
//    qDebug()<<"created";
}

//Track::~Track()
//{
//    qDebug()<<"destroyed";
//}

QString Track::statistics() const
{
    QFont font;
    QFontInfo sysFont(font);
    return QString()
        +QStringLiteral("<table style=\"font-size: ")+QString::number(int((sysFont.pointSizeF()*0.90)))+QStringLiteral("pt\">")
        +QStringLiteral("<tr><td><b>")+QObject::tr("Distance: ")+QStringLiteral("</b></td><td>")+QString::number(distance3D()/1000.0, 'f', 2)+QStringLiteral(" km</td></tr>")
        +QStringLiteral("<tr><td><b>")+QObject::tr("Climb: ")+QStringLiteral("</b></td><td>")+QString::number(climb(), 'f', 0)+QStringLiteral(" m</td></tr>")
        +(duration().isEmpty() ? "" : "<tr><td><b>"+QObject::tr("Duration: ")+QStringLiteral("</b></td><td>")+duration()+QStringLiteral("</td></tr>"))
        +QStringLiteral("<tr><td><b>")+QObject::tr("Max. elevation: ")+QStringLiteral("</b></td><td>")+QString::number(altitudeMax(), 'f', 0)+QStringLiteral(" m</td></tr>")
        +QStringLiteral("<tr><td><b>")+QObject::tr("Min. elevation: ")+QStringLiteral("</b></td><td>")+QString::number(altitudeMin(), 'f', 0)+QStringLiteral(" m< m/td></tr>")
        +QStringLiteral("<tr><td><b>")+QObject::tr("Elevation diff.: ")+QStringLiteral("</b></td><td>")+QString::number(altitudeMax()-altitudeMin(), 'f', 0)+QStringLiteral(" m< m/td></tr>")
        +QStringLiteral("</table>")
            ;
}

void Track::setDuration(qint64 s)
{
    m_duration = secondsToFormattedString(s);
    Q_EMIT(statisticsChanged());
}

QVariantList Track::variantPath() const
{
    QVariantList list;
    for (const auto &coordinate : m_path)
        list.append(QVariant::fromValue(coordinate));
    return list;
}

void Track::updateBoundingBox()
{
    m_bbox = QGeoRectangle(m_path.toList());
}

int Track::indexFromDistance(qreal distance) const
{
    return int(std::lower_bound(m_distances2D.constBegin(), m_distances2D.constEnd(), distance) - m_distances2D.constBegin());
}

QGeoCoordinate Track::coordinateFromDistance(qreal distance) const
{
    auto index = indexFromDistance(distance);
    return m_path.at(index);
}

void Track::addPoint(const QGeoCoordinate &point, const QString &timeStamp)
{
    if (m_path.isEmpty()) {
        // We're adding the first point
        m_altitudeMax = point.altitude();
        m_altitudeMin = point.altitude();
    } else {
        m_altitudeMax = qMax(m_altitudeMax, point.altitude());
        m_altitudeMin = qMin(m_altitudeMin, point.altitude());

        QGeoCoordinate lastPoint(m_path.last());
        qreal dist2D = point.distanceTo(lastPoint);
        qreal diff = point.altitude()-lastPoint.altitude();
        m_distance2D += dist2D;
        m_distance3D += qSqrt(dist2D*dist2D+diff*diff);
    }

    m_path.append(point);
    m_averagedAltitudes.append(point.altitude());
    m_distances2D.append(m_distance2D);
    m_timeStamps.append(timeStamp);
}

void Track::movePoint(int index, const QGeoCoordinate &point)
{
    if (index >= 0 && index < m_path.length()) {
        QGeoCoordinate previousPoint, nextPoint;
        qreal dist2D, diff;
        qreal distance2DBefore = m_distance2D;
        if (index > 0) {
            previousPoint = m_path.at(index-1);
            dist2D = m_path.at(index).distanceTo(previousPoint);
            diff = m_path.at(index).altitude()-previousPoint.altitude();
            m_distance2D -= dist2D;
            m_distance3D -= qSqrt(dist2D*dist2D+diff*diff);
        }
        if (index < m_path.length()-1) {
            nextPoint = m_path.at(index+1);
            dist2D = m_path.at(index).distanceTo(nextPoint);
            diff = m_path.at(index).altitude()-nextPoint.altitude();
            m_distance2D -= dist2D;
            m_distance3D -= qSqrt(dist2D*dist2D+diff*diff);
        }
        m_path[index].setLatitude(point.latitude());
        m_path[index].setLongitude(point.longitude());
        if (index > 0) {
            previousPoint = m_path.at(index-1);
            dist2D = m_path.at(index).distanceTo(previousPoint);
            diff = m_path.at(index).altitude()-previousPoint.altitude();
            m_distance2D += dist2D;
            m_distance3D += qSqrt(dist2D*dist2D+diff*diff);
        }
        if (index < m_path.length()-1) {
            nextPoint = m_path.at(index+1);
            dist2D = m_path.at(index).distanceTo(nextPoint);
            diff = m_path.at(index).altitude()-nextPoint.altitude();
            m_distance2D += dist2D;
            m_distance3D += qSqrt(dist2D*dist2D+diff*diff);
        }
        qreal diffDist2D = m_distance2D-distance2DBefore;
        for (int i = index; i < m_path.length(); ++i) {
            if (i!=0) m_distances2D[i] += diffDist2D;
        }
        Q_EMIT(pathChanged());
        Q_EMIT(statisticsChanged());
    }
}

void Track::removePoint(int index)
{
    if (index >= 0 && index < m_path.length()) {
        m_path.remove(index);
        m_averagedAltitudes.remove(index);
        m_distances2D.remove(index);
        m_timeStamps.remove(index);

        m_distance2D = 0.0;
        m_distance3D = 0.0;
        m_altitudeMin = 0.0;
        m_altitudeMax = 0.0;
        for (int i = 0; i < m_distances2D.length(); ++i) {
            //FIXME: dont recompute minmax if altitude is not an extrema
            if (i == 0) {
                m_altitudeMax = m_path.at(i).altitude();
                m_altitudeMin = m_path.at(i).altitude();
            } else {
                m_altitudeMax = qMax(m_altitudeMax, m_path.at(i).altitude());
                m_altitudeMin = qMin(m_altitudeMin, m_path.at(i).altitude());

                QGeoCoordinate previousPoint(m_path.at(i-1));
                qreal dist2D = m_path.at(i).distanceTo(previousPoint);
                qreal diff = m_path.at(i).altitude()-previousPoint.altitude();
                m_distance2D += dist2D;
                m_distance3D += qSqrt(dist2D*dist2D+diff*diff);
            }
            m_distances2D[i] = m_distance2D;
        }
        computeStatistics();
        if (index == 0 || index == m_timeStamps.length()) {
            QDateTime t1 = QDateTime::fromString(m_timeStamps.first(), Qt::ISODate);
            QDateTime t2 = QDateTime::fromString(m_timeStamps.last(), Qt::ISODate);
            m_duration = secondsToFormattedString(t1.secsTo(t2));
        }
        Q_EMIT(pathChanged());
        Q_EMIT(statisticsChanged());
    }
}

void Track::setPath(const QVariantList &path)
{
    m_distance2D = 0.0;
    m_distance3D = 0.0;
    m_altitudeMin = 0.0;
    m_altitudeMax = 0.0;
    m_duration = QString();
    m_path.clear();
    m_averagedAltitudes.clear();
    m_distances2D.clear();
    m_timeStamps.clear();
    for (const auto &coordinate : path) {
        addPoint(coordinate.value<QGeoCoordinate>());
    }
    computeStatistics(1, 0.0);
    Q_EMIT(pathChanged());
}

void Track::computeStatistics(int nAverage, qreal threshold)
{
    averageAltitudes(nAverage);
    computeClimb(threshold);
    Q_EMIT(statisticsChanged());
}

void Track::computeClimb(qreal threshold)
{
    qreal climb = 0.0;
    qreal descent = 0.0;
    qreal diff = 0.0;
    for (int i = 1; i < m_averagedAltitudes.length(); ++i) {
        diff += m_averagedAltitudes.at(i)-m_averagedAltitudes.at(i-1);
        if (diff > 0.0 && diff > threshold) {
            climb += diff;
            diff = 0.0;
        } else if (diff < 0.0 && diff < threshold) {
            descent -= diff;
            diff = 0.0;
        }
    }
    m_climb = climb;
    m_descent = descent;
}

void Track::averageAltitudes(int N)
{
    // Compute altitudes moving average
    if (N < 0)
        return;
    if (N%2 == 1) {
        QVector<qreal> averagedAltitudes;
        for (int i = 0; i < m_path.length(); ++i) {
            if ((i > (N-1)) && (i < (m_path.length()-N+1))) {
                qreal sum = m_path.at(i).altitude();
                for (int j = 1; j < (N-1)/2+1; ++j) {
                    sum += m_path.at(i-j).altitude()+m_path.at(i+j).altitude();
                }
                averagedAltitudes << (sum/N);
            } else {
                averagedAltitudes << m_path.at(i).altitude();
            }
        }
        m_averagedAltitudes = averagedAltitudes;
    } else {
        qWarning("averageAltitudes(int N) argument must be an odd number.");
    }
}

QString Track::secondsToFormattedString(qint64 s) const
{
    int seconds = s%60;
    s /= 60;
    int minutes = s%60;
    s /= 60;
    int hours = s%24;
    return QString("%1:%2:%3").arg(QString::number(hours), 2, '0').arg(QString::number(minutes), 2, '0').arg(QString::number(seconds), 2, '0');
}
