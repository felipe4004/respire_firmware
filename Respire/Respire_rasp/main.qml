import QtQuick 2.0
import QtQuick.Controls 2.5

ApplicationWindow {
    width: 1024
    height: 600
    visible: true
    title: qsTr("Tabs")

    SwipeView {
        id: swipeView
        anchors.fill: parent
        currentIndex: tabBar.currentIndex

        Page1Form {
        }

        Page2Form {
        }

        Page3Form {

        }
    }


    footer: TabBar {
        id: tabBar
        currentIndex: swipeView.currentIndex

        TabButton {
            text: qsTr("Respire 1")
        }
        TabButton {
            text: qsTr("Respire 2")
        }

        TabButton {
            text: qsTr("Respire 3")
        }
    }
}
