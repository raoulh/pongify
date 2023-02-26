QT       += core gui widgets quick quickcontrols2

CONFIG += c++17

include(3rd_party/quickflux/quickflux.pri)
include(3rd_party/qt-qml-models/QtQmlModels.pri)

SOURCES += \
    src/DialogAddPlayer.cpp \
    src/DialogEditScore.cpp \
    src/DialogNewSerie.cpp \
    src/DialogNewTournament.cpp \
    src/DialogPlayerList.cpp \
    src/DialogPlayers.cpp \
    src/PlayerModel.cpp \
    src/PlayerSync.cpp \
    src/TSerie.cpp \
    src/TStorage.cpp \
    src/Tournament.cpp \
    src/Utils.cpp \
    src/main.cpp \
    src/MainWindow.cpp

HEADERS += \
    src/DialogAddPlayer.h \
    src/DialogEditScore.h \
    src/DialogNewSerie.h \
    src/DialogNewTournament.h \
    src/DialogPlayerList.h \
    src/DialogPlayers.h \
    src/MainWindow.h \
    src/PlayerModel.h \
    src/PlayerSync.h \
    src/TSerie.h \
    src/TStorage.h \
    src/Tournament.h \
    src/Utils.h \
    src/qqmlhelpers.h

FORMS += \
    src/DialogAddPlayer.ui \
    src/DialogEditScore.ui \
    src/DialogNewSerie.ui \
    src/DialogNewTournament.ui \
    src/DialogPlayerList.ui \
    src/DialogPlayers.ui \
    src/MainWindow.ui

win32 {
    RC_FILE = windows_res.rc
}

RESOURCES += \
    res.qrc
