import QtQuick 2.11
import QtQuick.Controls 2.4
import QtQuick.Controls.Material 2.4
import QtQuick.Layouts 1.11

ColumnLayout {
	id: root
	property alias textSpace: textSpace
	property string value: ""
	property int cursPos: 0
	property int maxCursorPos: 0

	signal setValue(string val)

	function moveCursor(val) {
		val = (val > 0)? 1 : -1;
		var tmp = (textSpace.cursorPosition == root.maxCursorPos && val > 0)? 0 : 1
		textSpace.cursorPosition = root.cursPos + val
	}

	spacing: -9
	// text space
	Rectangle {
		Layout.alignment: Qt.AlignCenter
		color: Material.background
		border.color: color == "#303030"? "White" : "Black"
		border.width: 1
		width: textSpace.font.pixelSize * maxCursorPos * 0.53
		height: textSpace.height
		radius: 4
		TextEdit {
			id: textSpace
			text: root.value
			width: parent.width
			color: Material.foreground
			font.pixelSize: Qt.application.font.pixelSize * 1.5
			horizontalAlignment: Qt.AlignHCenter
			cursorVisible: true
			activeFocusOnPress: false
			overwriteMode: true
			onTextChanged: {
				cursorPosition = root.cursPos
			}
		}
	}
	// space
	Item {
		width: 1; height: 20
	}
	// buutons
	RowLayout {
		spacing: 2
		Button {
			id: b1
			text: "1"
			onClicked: setValue(text)
		}
		Button {
			id: b2
			text: "2"
			onClicked: setValue(text)
		}
		Button {
			id: b3
			text: "3"
			onClicked: setValue(text)
		}
	}
	RowLayout {
		spacing: 2
		Button {
			id: b4
			text: "4"
			onClicked: setValue(text)
		}
		Button {
			id: b5
			text: "5"
			onClicked: setValue(text)
		}
		Button {
			id: b6
			text: "6"
			onClicked: setValue(text)
		}
	}
	RowLayout {
		spacing: 2
		Button {
			id: b7
			text: "7"
			onClicked: setValue(text)
		}
		Button {
			id: b8
			text: "8"
			onClicked: setValue(text)
		}
		Button {
			id: b9
			text: "9"
			onClicked: setValue(text)
		}
	}
	RowLayout {
		spacing: 2
		Button {
			id: bLeft
			text: "<"
			onClicked: moveCursor(-1)
		}
		Button {
			id: b0
			text: "0"
			onClicked: setValue(text)
		}
		Button {
			id: bRight
			text: ">"
			onClicked: moveCursor(+1)
		}
	}
}
