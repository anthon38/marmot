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
    Q_INVOKABLE File *get(int index);
    Q_INVOKABLE void append(File* file);
    Q_INVOKABLE void remove(int index);

Q_SIGNALS:
    void countChanged();
    void fileAppened(File* file);
    void fileRemoved(File* file);


private:
    int createUniqueKey(QList<int> keys) const;

    QVector<File*> m_files;
    QMap<Track *,int> m_trackMap;
    QMap<Poi *,int> m_poiMap;
    QList<QColor> m_trackColors;
};

#endif // FILESMODEL_H
