#include "poi.h"

Poi::Poi(QObject *parent)
    : QObject(parent)
{
}

void Poi::setName(const QString &name)
{
    m_name = name;
    Q_EMIT(nameChanged());
}

void Poi::setDescription(const QString &description)
{
    m_description = description;
    Q_EMIT(descriptionChanged());
}

void Poi::setCoordinate(const QGeoCoordinate &coordinate)
{
    m_coordinate = coordinate;
    Q_EMIT(coordinateChanged());
}
