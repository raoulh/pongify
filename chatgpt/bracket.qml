import QtQuick 2.0

Item {
    property TSerie serie

    Rectangle {
        anchors.fill: parent
        color: "lightgray"
    }

    Component {
        id: matchItem

        Rectangle {
            id: matchRect
            width: 80
            height: 40
            border.width: 1
            border.color: "black"
            radius: 10
            color: "white"

            Text {
                text: model.player1.name + " vs " + model.player2.name
                anchors.centerIn: parent
            }
        }
    }

    Component {
        id: bracketItem

        Item {
            property int level: 0

            Repeater {
                model: serie.getMatchesByLevel(level)

                delegate: matchItem
            }

            Rectangle {
                width: 80
                height: 40 * count
                anchors.right: parent.right
                anchors.top: parent.top
                color: "transparent"
                border.width: 1
                border.color: "black"
            }

            Item {
                anchors.centerIn: parent
                width: 40
                height: 40
                Rectangle {
                    width: parent.width
                    height: parent.height
                    radius: parent.width / 2
                    color: "white"
                    border.width: 1
                    border.color: "black"
                }
                Text {
                    text: (level == 0) ? "Final" : ("Round " + (level + 1))
                    anchors.centerIn: parent
                }
            }
        }
    }

    Column {
        anchors.fill: parent
        Repeater {
            model: serie.getLevels()

            delegate: bracketItem
        }
    }
}

/*

This code defines an Item that displays a bracket-style representation of the matches in a TSerie instance. The serie property should be set to the TSerie instance to display.

The matchItem component defines a rectangular item with a border and text showing the names of the players in the match. The bracketItem component defines an Item that displays a set of matches at a given level in the bracket, along with a title showing the level. The matches are obtained from the TSerie instance using the getMatchesByLevel() method, which returns a list of matches at the given level. The level property of the Item is used to specify the level to display.

The Column element contains a Repeater that displays a bracketItem for each level in the TSerie instance. The levels are obtained using the getLevels() method, which returns a list of level indices. The bracketItem component is instantiated for each level using the delegate property of the Repeater.

Note that this example code assumes that the Player class has a name property that can be used to display the names of the players in each match. If this is not the case, the code will need to be modified accordingly.

*/