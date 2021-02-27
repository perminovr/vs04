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

    RoundButton {
        id: backButton
        z: 1
        text: "∆"
        anchors.bottom: root.bottom
        anchors.bottomMargin: 8
        anchors.horizontalCenter: parent.horizontalCenter
        visible: true
        onClicked: {
            root.state = "hide"
            root.signalExit()
        }
    }

    Image {
        x: 0
        y: backButton.y + backButton.height + 8
        sourceSize.width: parent.width
        sourceSize.height: parent.height
        source: "img/shadow_down.png"
        fillMode: Image.PreserveAspectCrop
        visible: root.y != -root.height*2
    }

    states: [
        State {
            name: "hide"
            PropertyChanges {
                target: root
                y: -height*2
            }
        },
        State {
            name: "show"
            PropertyChanges {
                target: root
                y: 0
            }
        }
    ]

    transitions: Transition {
        PropertyAnimation {
            properties: "y"
            easing.type: Easing.Linear
            duration: 300
        }
    }

    Component.onCompleted: {
        root.state = "hide"
    }
}
