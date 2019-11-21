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

#include <QApplication>
#include <QIcon>
#include <QTranslator>
#include <QSurfaceFormat>
#include <QQmlApplicationEngine>

#include "file.h"
#include "chart.h"
#include "filesmodel.h"
#include "sortfilterproxymodel.h"
#include "iconimage.h"
#include "settings.h"
#include "utils.h"

int main(int argc, char *argv[])
{
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QCoreApplication::setOrganizationName(QStringLiteral("avital"));
    QCoreApplication::setApplicationName(QStringLiteral("Marmot"));
    QCoreApplication::setApplicationVersion(QStringLiteral("1.0"));

    QApplication app(argc, argv);
    app.setWindowIcon(QIcon(QStringLiteral(":/images/marmot.svg")));
    qmlRegisterType<File>("Marmot", 1, 0, "File");
    qmlRegisterType<Chart>("Marmot", 1, 0, "Chart");
    qmlRegisterType<FilesModel>("Marmot", 1, 0, "FilesModel");
    qmlRegisterType<SortFilterProxyModel>("Marmot", 1, 0, "SortFilterProxyModel");
    qmlRegisterType<IconImage>("Marmot", 1, 0, "IconImage");
    qmlRegisterSingletonType<Settings>("Marmot", 1, 0, "Settings", &Settings::qmlInstance);
    qmlRegisterSingletonType<Utils>("Marmot", 1, 0, "Utils", &Utils::qmlInstance);

    QTranslator translator;
    if (translator.load(QLocale(), QStringLiteral("marmot"), QStringLiteral("_"), QStringLiteral(":/translations/")))
        app.installTranslator(&translator);

    QSurfaceFormat format = QSurfaceFormat::defaultFormat();
    format.setSamples(8);
    QSurfaceFormat::setDefaultFormat(format);

    QQmlApplicationEngine engine;
    engine.load(QUrl(QStringLiteral("qrc:/main.qml")));

    return app.exec();
}
