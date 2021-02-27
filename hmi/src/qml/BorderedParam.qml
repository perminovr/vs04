import QtQuick 2.11
import QtQuick.Controls 2.4
import QtQuick.Controls.Material 2.4
import QtQuick.Layouts 1.3

ColumnLayout {
	id: root
	property string header
	property string contentText
	property string contentColor

	Label {
		id: nameLabel
		width: parent.width
		Layout.alignment: Qt.AlignCenter
		text: root.header
		color: Material.foreground
		font.pixelSize: Qt.application.font.pixelSize * 1.4
	}
	Rectangle {
		Layout.alignment: Qt.AlignCenter
		Layout.bottomMargin: 30
		width: panelNameLabel.font.pixelSize * 34 * 0.53
		height: panelNameLabel.height+10
		color: Material.background == "#303030"? "#505050" : "#c8c8c8"
		border.color: Material.background == "#303030"? "White" : "Black"
		border.width: 1
		radius: 6
		Label {
			id: panelNameLabel
			width: parent.width
			anchors.verticalCenter: parent.verticalCenter
			font.pixelSize: Qt.application.font.pixelSize * 1.5
			horizontalAlignment: Qt.AlignHCenter
			text: root.contentText
			color: root.contentColor
		}
	}
}
