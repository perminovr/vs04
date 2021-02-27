import QtQuick 2.11
import QtQuick.Controls 2.4
import QtQuick.Controls.Material 2.4
import QtQuick.Layouts 1.11

Rectangle {
	id: root
    property int maximumLength: 16
	property alias textFocus: textIn.focus
	property alias text: textIn.text
    property alias echoMode: textIn.echoMode
    property alias cursorPosition: textIn.cursorPosition
	property string label: ""

    signal onTextChanged(string str)

    signal onCursorPositionChanged(int pos)
    function setCursorPosition(pos) {
        textIn.cursorPosition = pos
    }

	function forceActiveFocus() {
		textIn.forceActiveFocus()
    }

	Layout.alignment: Qt.AlignLeft
	color: Material.background
	border.color: color == "#303030"? "White" : "Black"
	border.width: 1
	width: textIn.font.pixelSize * root.maximumLength * 0.63
	height: textIn.height
	radius: 4
    TextInput {
		id: textIn
		leftPadding: 5
		Layout.leftMargin: 5
		width: parent.width
		color: Material.foreground
		font.pixelSize: Qt.application.font.pixelSize * 1.5
		focus: root.textFocus
		font.family: "FreeMono"
		maximumLength: root.maximumLength
        Component.onCompleted: {
            textIn.onTextChanged.connect(function(){ root.onTextChanged(textIn.text) })
            textIn.onFocusChanged.connect(function(){ if (textIn.focus) inputPanel.setInputObject(root) })
            textIn.onCursorPositionChanged.connect(function(){ root.onCursorPositionChanged(textIn.cursorPosition) })
        }
	}
}
