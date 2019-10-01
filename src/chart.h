#ifndef CHART_H
#define CHART_H

#include <QQuickPaintedItem>

#include "track.h"

class Chart : public QQuickPaintedItem
{
    Q_OBJECT
    Q_PROPERTY(qreal xMin READ xMin WRITE setXmin NOTIFY xMinChanged)
    Q_PROPERTY(qreal xMax READ xMax WRITE setXmax NOTIFY xMaxChanged)
    Q_PROPERTY(qreal yMin READ yMin WRITE setYmin NOTIFY yMinChanged)
    Q_PROPERTY(qreal yMax READ yMax WRITE setYmax NOTIFY yMaxChanged)
    Q_PROPERTY(qreal count READ count NOTIFY countChanged)

public:
    explicit Chart(QQuickItem *parent = nullptr);
    qreal xMin() const {return m_xMin;}
    void setXmin(qreal xMin);
    qreal xMax() const {return m_xMax;}
    void setXmax(qreal xMax);
    qreal yMin() const {return m_yMin;}
    void setYmin(qreal yMin);
    qreal yMax() const {return m_yMax;}
    void setYmax(qreal yMax);
    int count() const {return m_trackList.length();}

    void paint(QPainter *painter);
    Q_INVOKABLE void createSeries(Track *track);
    Q_INVOKABLE qreal mapToDistance(int x) const;
    Q_INVOKABLE QPoint mapToPosition(const QPointF &point) const;

Q_SIGNALS:
    void xMinChanged();
    void xMaxChanged();
    void yMinChanged();
    void yMaxChanged();
    void countChanged();
    void trackAdded(Track *track);
    void trackRemoved(QString trackName);

private:
    qreal m_xMin;
    qreal m_xMax;
    qreal m_yMin;
    qreal m_yMax;
    QVector<Track *> m_trackList;
};

#endif // CHART_H
