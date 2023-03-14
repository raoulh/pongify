import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Rectangle {
    color: "red"

    Text {
        anchors.centerIn: parent
        textFormat: Text.RichText
        text: currentTournament.infoText
    }
}
