import QtQuick 2.11
import QtQuick.Controls 2.4
import QtQuick.Controls.Material 2.4
import QtQuick.Layouts 1.3

Item {
	id: root
	property string head
	property alias contentSource: contentFrame.sourceComponent
	property alias content: contentFrame.item
	signal loaded()

    Rectangle {
        id: headerSpace
        Layout.fillWidth: true
        Layout.alignment: Qt.AlignCenter
        anchors.top: root.top
        color: Material.background
        height: headLabel.height + 10
        width: parent.width
        Label {
            id: headLabel
            width: parent.width
            text: root.head
            font.pixelSize: Qt.application.font.pixelSize * 2
            horizontalAlignment: Qt.AlignHCenter
        }
        Rectangle {
            width: headLabel.paintedWidth + headLabel.paintedWidth / headLabel.text.length * 1.5
            height: 1
            color: Material.background == "#303030"? "White" : "Black"
            anchors.top: headLabel.bottom
            anchors.horizontalCenter:  headLabel.horizontalCenter
        }
    }
	Rectangle {
		color: Material.background
		width: root.width
        height: root.height - headerSpace.height
		anchors.bottom: root.bottom
		anchors.bottomMargin: 1
		Loader {
			id: contentFrame
			anchors.fill: parent
			onLoaded: root.loaded()
		}
	}
}
