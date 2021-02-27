import QtQuick 2.11
import QtQuick.Window 2.2
import QtQuick.Controls 2.4
import QtQuick.Controls.Material 2.4
import QtQuick.Layouts 1.3
import QtQuick.VirtualKeyboard 2.0
import QtQuick.VirtualKeyboard.Settings 2.1

Item {
    id: root
    parent: Overlay.overlay
    z: 99
    x: 0
    y: parent.height
    width: parent.width
    height: backgr.height

    property TextInputMono inputObject: null
    function _ti_setText(str) { textInput.setText(str) }
    function _ti_setPos(pos) { textInput.setCursorPosition(pos) }
    function _io_setPos(pos) { inputObject.setCursorPosition(pos) }
    function setInputObject(object) {
        inputObject = object
        textInput.maximumLength = inputObject.maximumLength
        textInput.text = inputObject.text
        textInput.setCursorPosition(inputObject.cursorPosition)
        inputObject.onTextChanged.connect(_ti_setText)
        inputObject.onCursorPositionChanged.connect(_ti_setPos)
        textInput.onCursorPositionChanged.connect(_io_setPos)
    }

    Rectangle {
        id: backgr
        width: parent.width
        height: inputPanel.height + infoField.height
        color: "Black"
    }

    MouseArea {
        anchors.fill: backgr
    }

    RowLayout {
        id: infoField
        anchors.horizontalCenter: root.horizontalCenter
        y: 15
        scale: 1.2
        Label {
            text: (root.inputObject != null)? root.inputObject.label : ""
            visible: (root.inputObject != null) && (root.inputObject.label.length > 0)
        }
        TextReadOnlyMono {
            id: textInput
        }
    }

    InputPanel {        
        id: inputPanel
        width: root.width
        y: infoField.y + infoField.height
        scale: 0.95

        Component.onCompleted: {
            VirtualKeyboardSettings.activeLocales = ["en_GB", "ru_RU"]
            keyboard.style.keyboardBackground = null;
            keyboard.style.selectionListBackground = null;
        }
    }

    states: State {
        name: "visible"
        when: inputPanel.active
        PropertyChanges {
            target: root
            y: root.parent.height - backgr.height
        }
    }
    transitions: Transition {
        from: ""
        to: "visible"
        reversible: true
        ParallelAnimation {
            NumberAnimation {
                properties: "y"
                duration: 250
                easing.type: Easing.InOutQuad
            }
        }
    }

}
