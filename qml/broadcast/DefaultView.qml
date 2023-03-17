import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Rectangle {
    color: "#FAFAFA"

    Text {
        id: title
        text: currentTournament.name
        anchors {
            horizontalCenter: parent.horizontalCenter
            top: parent.top
            topMargin: 50
        }
        font {
            pointSize: 30
            bold: true
        }
    }
    Text {
        text: currentTournament.date.toLocaleString(Qt.locale("fr_FR"), "dddd, d MMMM yyyy")
        anchors {
            horizontalCenter: parent.horizontalCenter
            top: title.bottom
            topMargin: 10
        }
        font {
            pointSize: 20
        }
    }

    Text {
        anchors.centerIn: parent
        textFormat: Text.RichText
        text: currentTournament.infoText
    }
}
