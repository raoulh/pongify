import QtQuick
import QtQuick.Layouts
import QtQuick.Controls

SplitView {

    SerieList {
        id: left

        SplitView.minimumWidth: 50
        SplitView.preferredWidth: 200
        SplitView.maximumWidth: 500
    }

    SerieView {
        id: center
    }
}
