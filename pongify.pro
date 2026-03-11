QT       += core gui widgets quick quickcontrols2 network printsupport

CONFIG += c++17

include(3rd_party/quickflux/quickflux.pri)
include(3rd_party/qt-qml-models/QtQmlModels.pri)

INCLUDEPATH += src 3rd_party/qrcodegen

SOURCES += \
    src/BroadcastModel.cpp \
    src/BroadcastWindow.cpp \
    src/CloudflareSetup.cpp \
    src/DialogAbout.cpp \
    src/DialogAddDoublePlayers.cpp \
    src/DialogAddPlayer.cpp \
    src/DialogBroadcastOpts.cpp \
    src/DialogChangePlayer.cpp \
    src/DialogCloudflareSetup.cpp \
    src/DialogCreatePools.cpp \
    src/DialogEditInfo.cpp \
    src/DialogEditScore.cpp \
    src/DialogNewSerie.cpp \
    src/DialogNewTournament.cpp \
    src/DialogPlayerDispatch.cpp \
    src/DialogPlayerList.cpp \
    src/DialogPlayers.cpp \
    src/DialogPlayersHtml.cpp \
    src/DialogPodiumRR.cpp \
    src/DialogPodiumSingle.cpp \
    src/DialogQualification.cpp \
    src/DialogRestoreBackup.cpp \
    src/DialogTournamentRoster.cpp \
    src/PlayerModel.cpp \
    src/PlayerSync.cpp \
    src/TSerie.cpp \
    src/TStorage.cpp \
    src/TTable.cpp \
    src/TableMatchModel.cpp \
    src/Tournament.cpp \
    src/Utils.cpp \
    src/WidgetTextEdit.cpp \
    src/main.cpp \
    src/MainWindow.cpp \
    src/WebPublisher.cpp \
    src/DialogWebPublish.cpp \
    src/DialogTournamentQr.cpp \
    3rd_party/qrcodegen/qrcodegen.cpp

HEADERS += \
    src/BroadcastModel.h \
    src/BroadcastPreviewProvider.h \
    src/BroadcastWindow.h \
    src/CloudflareSetup.h \
    src/DialogAbout.h \
    src/DialogAddDoublePlayers.h \
    src/DialogAddPlayer.h \
    src/DialogBroadcastOpts.h \
    src/DialogChangePlayer.h \
    src/DialogCloudflareSetup.h \
    src/DialogCreatePools.h \
    src/DialogEditInfo.h \
    src/DialogEditScore.h \
    src/DialogNewSerie.h \
    src/DialogNewTournament.h \
    src/DialogPlayerDispatch.h \
    src/DialogPlayerList.h \
    src/DialogPlayers.h \
    src/DialogPlayersHtml.h \
    src/DialogPodiumRR.h \
    src/DialogPodiumSingle.h \
    src/DialogQualification.h \
    src/DialogRestoreBackup.h \
    src/DialogTournamentRoster.h \
    src/MainWindow.h \
    src/PlayerModel.h \
    src/PlayerSync.h \
    src/Score.h \
    src/TSerie.h \
    src/TStorage.h \
    src/TTable.h \
    src/TableMatchModel.h \
    src/Tournament.h \
    src/Utils.h \
    src/WidgetTextEdit.h \
    src/qqmlhelpers.h \
    src/WebPublisher.h \
    src/DialogWebPublish.h \
    src/DialogTournamentQr.h \
    src/QrCodeProvider.h \
    3rd_party/qrcodegen/qrcodegen.hpp \
    version.h

FORMS += \
    src/DialogAbout.ui \
    src/DialogAddDoublePlayers.ui \
    src/DialogAddPlayer.ui \
    src/DialogBroadcastOpts.ui \
    src/DialogChangePlayer.ui \
    src/DialogCloudflareSetup.ui \
    src/DialogCreatePools.ui \
    src/DialogEditInfo.ui \
    src/DialogEditScore.ui \
    src/DialogNewSerie.ui \
    src/DialogNewTournament.ui \
    src/DialogPlayerDispatch.ui \
    src/DialogPlayerList.ui \
    src/DialogPlayers.ui \
    src/DialogPlayersHtml.ui \
    src/DialogPodiumRR.ui \
    src/DialogPodiumSingle.ui \
    src/DialogQualification.ui \
    src/DialogRestoreBackup.ui \
    src/DialogTournamentRoster.ui \
    src/MainWindow.ui \
    src/WidgetTextEdit.ui \
    src/DialogWebPublish.ui

win32 {
    RC_FILE = windows_res.rc
    LIBS += -lbcrypt
}

RESOURCES += \
    res.qrc
