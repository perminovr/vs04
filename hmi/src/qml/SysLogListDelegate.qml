import QtQuick 2.11
import QtQuick.Controls 2.4
import QtQuick.Controls.Material 2.4
import QtQuick.Layouts 1.11

import Hmi 1.0

ItemDelegate {
	id: sysLogDelegate
	width: parent.width
	anchors.margins: 0

	function getImgSource(type) {
		switch (type) {
		case UIPanelCtl.Note: return "icons/info_s.png"
		case UIPanelCtl.Warning: return "icons/warn_s.png"
		case UIPanelCtl.Error: return "icons/err_s.png"
		case UIPanelCtl.Debug: return "icons/debug_s.png"
		}
		return ""
	}

	function paddy(ts) {
		var sec = ~~(ts/1000)
		var ms = ts%1000
		var pad = "000";
		var sms = (pad + ms).slice(-pad.length);
		var sts = sec + "." + sms
		pad = "			";
		return (pad + sts).slice(-pad.length);
	}

	contentItem: Rectangle {
		id: container
		Layout.fillWidth: true
		height: content.height
		border.color: "Black"
		border.width: 1
		RowLayout {
			id: content
			anchors.verticalCenter: parent.verticalCenter
			spacing: 5
			Image {
				id: img
				width: 20
				height: 20
				Layout.leftMargin: 5
				source: getImgSource(model.type)
				fillMode: Image.Stretch
				sourceSize.width: 20
				sourceSize.height: 20
			}
			Rectangle {
				height: content.height
				width: lblTs.width
				Label {
					id: lblTs
					width: 75
					anchors.verticalCenter: parent.verticalCenter
					font.pixelSize: Qt.application.font.pixelSize
					horizontalAlignment: Qt.AlignRight
					color: "Black"
					text: "[ " + paddy(model.ts) + " ]";
				}
			}
			Label {
				width: container.width
				font.pixelSize: Qt.application.font.pixelSize
				color: "Black"
				text: model.text
			}
		}
	}
}
