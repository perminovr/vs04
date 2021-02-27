import QtQuick 2.11
import QtQuick.Controls 2.4
import QtQuick.Controls.Material 2.4
import QtQuick.Layouts 1.3

HeadedWindow {
    id: root
    z: parent.z+1 // for exit button

    signal signalExit()
    signal signalEnter()

    function open() {
        root.state = "show"
        root.signalEnter()
    }

    FlatRoundButton {
        id: backButton
        z: 1
        text: "X"
        anchors.bottom: root.bottom
        anchors.bottomMargin: 8
        anchors.horizontalCenter: parent.horizontalCenter
        visible: true
        onClicked: {
            root.state = "hide"
            root.signalExit()
        }
    }

    states: [
        State {
            name: "hide"
            PropertyChanges {
                target: root
                scale: 0.7
                y: root.height*0.3
                visible: true
            }
        },
        State {
            name: "show"
            PropertyChanges {
                target: root
                scale: 1.0
                y: 0
                visible: true
            }
        }
    ]

    transitions: Transition {
        ParallelAnimation {
            PropertyAnimation {
                properties: "scale"
                easing.type: Easing.Linear
                duration: 100
            }
            PropertyAnimation {
                properties: "y"
                easing.type: Easing.Linear
                duration: 100
            }
        }
        onRunningChanged: {
            if (!running) root.visible = (root.scale > 0.99)? true:false
        }
    }

    Component.onCompleted: {
        root.state = "hide"
    }
}
