#ifndef FILE_H
#define FILE_H

#include <QObject>
#include <QVariantList>
#include <QXmlStreamReader>
#include <QGeoRectangle>

class File : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString name READ name NOTIFY opened)
    Q_PROPERTY(QList<QObject*> pois READ pois NOTIFY poiChanged)
    Q_PROPERTY(QList<QObject*> tracks READ tracks NOTIFY tracksChanged)
    Q_PROPERTY(QGeoRectangle boundingBox READ boundingBox)
public:
    explicit File(QObject *parent = nullptr);
    Q_INVOKABLE bool open(const QString &fileName);
    void parseKml(QXmlStreamReader *reader);
    void parseGpx(QXmlStreamReader *reader);
    QString name() const {return m_name;}
    QObjectList pois() const {return m_pois;}
    QObjectList tracks() const {return m_tracks;}
    QGeoRectangle boundingBox() const {return m_bbox;}
    Q_INVOKABLE void addPoi(const QGeoCoordinate &coordinate, const QString &name, const QString &description = QString());
    Q_INVOKABLE void removePoi(int index);
    Q_INVOKABLE void addTrack();
    Q_INVOKABLE void exportToGpx(const QString &fileName) const;

Q_SIGNALS:
    void opened();
    void tracksChanged();
    void poiChanged();

private :
    void updateBoundingBox();
    void updateMinMax();

    QString m_name;
    QObjectList m_pois;
    QObjectList m_tracks;
    QGeoRectangle m_bbox;
};

#endif // FILE_H
