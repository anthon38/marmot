#include <QApplication>
#include <QIcon>
#include <QTranslator>
#include <QSurfaceFormat>
#include <QQmlApplicationEngine>
//#include <QPalette>
//#include <QQuickStyle>

#include "file.h"
#include "track.h"
#include "poi.h"
#include "chart.h"
#include "filesmodel.h"

int main(int argc, char *argv[])
{
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QCoreApplication::setOrganizationName(QStringLiteral("avital"));
    QCoreApplication::setApplicationName(QStringLiteral("HikeManager"));
    QCoreApplication::setApplicationVersion(QStringLiteral("1.0"));

    QApplication app(argc, argv);
    app.setWindowIcon(QIcon(QStringLiteral(":/images/hm-icon.svg")));
    qmlRegisterType<File>("HikeManager", 1, 0, "File");
    qmlRegisterType<Chart>("HikeManager", 1, 0, "Chart");
    qmlRegisterType<FilesModel>("HikeManager", 1, 0, "FilesModel");

    QTranslator translator;
    if (translator.load(QLocale(), QStringLiteral("hikemanager"), QStringLiteral("_"), QStringLiteral(":/translations/")))
        app.installTranslator(&translator);

    QSurfaceFormat format = QSurfaceFormat::defaultFormat();
    format.setSamples(8);
    QSurfaceFormat::setDefaultFormat(format);
//QQuickStyle::setStyle("Default");
//    QPalette palette = app.palette();
//    palette.setColor(QPalette::Base, QColor("#31363b"));
//    palette.setColor(QPalette::Text, QColor("#eff0f1"));
//    palette.setColor(QPalette::Highlight, QColor("#3daee9"));
//    app.setPalette(palette);

    QQmlApplicationEngine engine;
    engine.load(QUrl(QStringLiteral("qrc:/main.qml")));

    return app.exec();
}
