import QtQuick 2.11
import QtQuick.Controls 2.4
import QtQuick.Controls.Material 2.4
import QtQuick.Layouts 1.3

import Hmi 1.0

Item {
	id: root
	property bool inConfigAvailable
	property int status: 0
	property string panelName: ""

	ColumnLayout {
		width: parent.width
		height: parent.height
		anchors.fill: parent
		spacing: -10
		Image {
			id: image
			Layout.alignment: Qt.AlignCenter
			Layout.topMargin: 20
			Layout.preferredWidth: parent.width/2
			Layout.preferredHeight: parent.height/2.2
			fillMode: Image.PreserveAspectFit
			sourceSize.width: 700
			sourceSize.height: 400
			source: "img/logo-v.png"
		}
		BorderedParam {
			Layout.alignment: Qt.AlignCenter
			header: "Имя:"
			contentText: root.panelName
			contentColor: Material.foreground
		}
		BorderedParam {
			function getContentText(status) {
				var ret = ""
				switch (status) {
				case UIPanelCtl.InWork:
					ret = "В работе"
					break
				case UIPanelCtl.NoConf:
					ret = "Конфигурация отсутствует"
					break
				case UIPanelCtl.ErrConf:
					ret = "Ошибка загрузки конфигурации"
					break
				}
				return "<b>" + ret + "</b>"
			}
			function getContentColor(status) {
				switch (status) {
				case UIPanelCtl.InWork:
					return "Green"
				case UIPanelCtl.NoConf:
					return "Yellow"
				case UIPanelCtl.ErrConf:
					return "Red"
				}
			}
			Layout.alignment: Qt.AlignCenter
			header: "Состояние:"
			contentText: getContentText(root.status)
			contentColor: getContentColor(root.status)
		}
	}
    FlatRoundButton {
		function isConfigAvailable() {
			if (root.status === UIPanelCtl.InWork)
				return true
			return false
		}

		id: roundButton
		anchors.top: parent.top
		anchors.right: parent.right
		anchors.margins: 5
		enabled: isConfigAvailable()
		text: "X"
		onClicked: uiPanelCtl.openUserConfig()
		//scale: roundButton.pressed ? 0.9 : 1.0
	}
}
