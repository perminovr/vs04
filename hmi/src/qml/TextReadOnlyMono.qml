import QtQuick 2.11
import QtQuick.Controls 2.4
import QtQuick.Controls.Material 2.4
import QtQuick.Layouts 1.11

Rectangle {
    id: root
    property int maximumLength: 16
    property alias text: textSpace.text
    property alias cursorPosition: textSpace.cursorPosition
    property bool cursPosFlag: false

    signal onTextChanged(string str)
    function setText(str) {
        textSpace.text = str
    }

    signal onCursorPositionChanged(int pos)
    function setCursorPosition(pos) {
        textSpace.cursorPosition = pos
    }

    color: Material.background
    border.color: color == "#303030"? "White" : "Black"
    border.width: 1
    width: textSpace.font.pixelSize * root.maximumLength * 0.63
    height: textSpace.height
    radius: 4
    TextEdit {
        id: textSpace
        width: parent.width
        color: Material.foreground
        font.pixelSize: Qt.application.font.pixelSize * 1.5
        font.family: "FreeMono"
        horizontalAlignment: Qt.AlignHCenter
        cursorVisible: true
        activeFocusOnPress: false
        onCursorPositionChanged: { if (!root.cursPosFlag) root.onCursorPositionChanged(textSpace.cursorPosition); root.cursPosFlag = false }
        onTextChanged: { root.cursPosFlag = true } // иначе по изменению текста сброс курсора в 0
    }
}
