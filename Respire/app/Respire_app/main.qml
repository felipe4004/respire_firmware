import QtQuick 2.12
import QtQuick.Controls 2.5

ApplicationWindow {
    width: 1024
    height: 600
    visible: true
    title: qsTr("Respire Monitor")

    SwipeView {
        id: swipeView
        anchors.fill: parent
        currentIndex: tabBar.currentIndex

    }

    footer: TabBar {
        id: tabBar
        currentIndex: swipeView.currentIndex
    }
}
