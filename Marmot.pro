TARGET = marmot
QT += qml quick positioning quickcontrols2 widgets
CONFIG += c++11

HEADERS += \
    src/file.h \
    src/filesmodel.h \
    src/poi.h \
    src/chart.h \
    src/settings.h \
    src/sortfilterproxymodel.h \
    src/track.h \
    src/utils.h \
    src/iconimage.h

SOURCES += \
    src/file.cpp \
    src/filesmodel.cpp \
    src/main.cpp \
    src/poi.cpp \
    src/chart.cpp \
    src/settings.cpp \
    src/sortfilterproxymodel.cpp \
    src/track.cpp \
    src/utils.cpp \
    src/iconimage.cpp

# Additional import path used to resolve QML modules in Qt Creator's code model
QML_IMPORT_PATH =

# Default rules for deployment.
include(deployment.pri)

RESOURCES += \
    assets.qrc \
    src/qml/qml.qrc \
    translations.qrc

lupdate_only{
    SOURCES += src/qml/*.qml
    TRANSLATIONS = translations/marmot_fr.ts
}
