QT       += core gui widgets quick quickcontrols2

CONFIG += c++17

include(3rd_party/quickflux/quickflux.pri)
include(3rd_party/qt-qml-models/QtQmlModels.pri)

INCLUDEPATH += src

SOURCES += \
    src/BroadcastModel.cpp \
    src/BroadcastWindow.cpp \
    src/DialogAddDoublePlayers.cpp \
    src/DialogAddPlayer.cpp \
    src/DialogBroadcastOpts.cpp \
    src/DialogChangePlayer.cpp \
    src/DialogEditInfo.cpp \
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
    src/WidgetTextEdit.cpp \
    src/main.cpp \
    src/MainWindow.cpp

HEADERS += \
    src/BroadcastModel.h \
    src/BroadcastWindow.h \
    src/DialogAddDoublePlayers.h \
    src/DialogAddPlayer.h \
    src/DialogBroadcastOpts.h \
    src/DialogChangePlayer.h \
    src/DialogEditInfo.h \
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
    src/WidgetTextEdit.h \
    src/qqmlhelpers.h

FORMS += \
    src/DialogAddDoublePlayers.ui \
    src/DialogAddPlayer.ui \
    src/DialogBroadcastOpts.ui \
    src/DialogChangePlayer.ui \
    src/DialogEditInfo.ui \
    src/DialogEditScore.ui \
    src/DialogNewSerie.ui \
    src/DialogNewTournament.ui \
    src/DialogPlayerList.ui \
    src/DialogPlayers.ui \
    src/MainWindow.ui \
    src/WidgetTextEdit.ui

win32 {
    RC_FILE = windows_res.rc
}

RESOURCES += \
    res.qrc
