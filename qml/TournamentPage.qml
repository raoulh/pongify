import QtQuick
import QtQuick.Layouts
import QtQuick.Controls

SplitView {

    SerieList {
        id: left

        SplitView.minimumWidth: 260
        SplitView.preferredWidth: 260
        SplitView.maximumWidth: 600
    }

    SerieView {
        id: center
    }
}
