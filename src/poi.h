#ifndef POI_H
#define POI_H

#include <QObject>
#include <QString>
#include <QGeoCoordinate>
//#include <QVariantMap>

class Poi : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString name READ name WRITE setName NOTIFY nameChanged)
    Q_PROPERTY(QString description READ description WRITE setDescription NOTIFY descriptionChanged)
    Q_PROPERTY(QGeoCoordinate coordinate READ coordinate NOTIFY coordinateChanged)
//    Q_PROPERTY(QVariantMap attributes READ coordinate)

public:
    explicit Poi(QObject *parent = nullptr);
    QString name() const {return m_name;}
    void setName(const QString &name);
    QString description() const {return m_description;}
    void setDescription(const QString &description);
    QGeoCoordinate coordinate() const {return m_coordinate;}
    void setCoordinate(const QGeoCoordinate &coordinate);
//    QVariantMap attributes() const {return m_attributes;}

Q_SIGNALS:
    void nameChanged();
    void descriptionChanged();
    void coordinateChanged();

private:
    QString m_name;
    QString m_description;
    QGeoCoordinate m_coordinate;
//    QVariantMap m_attributes;
};

#endif // POI_H
