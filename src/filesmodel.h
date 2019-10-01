#ifndef FILESMODEL_H
#define FILESMODEL_H

#include <QAbstractListModel>

#include "file.h"
#include "track.h"
#include "poi.h"

class FilesModel : public QAbstractListModel
{
    Q_OBJECT
    Q_PROPERTY(int count READ count NOTIFY countChanged)
    Q_PROPERTY(qreal xMax READ xMax NOTIFY extremaChanged)
    Q_PROPERTY(qreal yMin READ yMin NOTIFY extremaChanged)
    Q_PROPERTY(qreal yMax READ yMax NOTIFY extremaChanged)
public:
    enum DataRoles {
        NameRole = Qt::UserRole + 1,
        TracksRole,
        PoisRole,
        BoundingBoxRole
    };

    explicit FilesModel(QObject *parent = nullptr);
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QHash<int, QByteArray> roleNames() const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

    int count();
    qreal xMax() const {return m_xMax;}
    qreal yMin() const {return m_yMin;}
    qreal yMax() const {return m_yMax;}
    Q_INVOKABLE File *get(int index);
    Q_INVOKABLE void append(File* file);
    Q_INVOKABLE void remove(int index);

Q_SIGNALS:
    void countChanged();
    void extremaChanged();
    void fileAppened(File* file);
    void fileRemoved(File* file);


private:
    int createUniqueKey(QList<int> keys) const;
    void updateExtrema();

    QVector<File*> m_files;
    QMap<Track *,int> m_trackMap;
    QMap<Poi *,int> m_poiMap;
    QList<QColor> m_trackColors;
    qreal m_xMin;
    qreal m_xMax;
    qreal m_yMin;
    qreal m_yMax;
};

#endif // FILESMODEL_H
