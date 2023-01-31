QT       += core gui widgets quick

CONFIG += c++17

include(3rd_party/quickflux/quickflux.pri)
include(3rd_party/qt-qml-models/QtQmlModels.pri)

SOURCES += \
    src/DialogPlayers.cpp \
    src/PlayerModel.cpp \
    src/PlayerSync.cpp \
    src/Utils.cpp \
    src/main.cpp \
    src/MainWindow.cpp

HEADERS += \
    src/DialogPlayers.h \
    src/MainWindow.h \
    src/PlayerModel.h \
    src/PlayerSync.h \
    src/Utils.h \
    src/qqmlhelpers.h

FORMS += \
    src/DialogPlayers.ui \
    src/MainWindow.ui

win32 {
    RC_FILE = windows_res.rc
}

RESOURCES += \
    res.qrc
