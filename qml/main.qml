import QtQuick

Item {
    anchors.fill: parent

    DashboardPage {
        id: dashboardPage
        anchors.centerIn: parent

        visible: !mainWindow.tournamentOpened
    }

    TournamentPage {
        id: tournamentPage
        anchors.fill: parent

        visible: mainWindow.tournamentOpened
    }
}
