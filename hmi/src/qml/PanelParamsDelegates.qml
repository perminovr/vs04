import QtQuick 2.11
import QtQuick.Controls 2.4
import QtQuick.Controls.Material 2.4
import QtQuick.Layouts 1.11

ItemDelegate {
    id: root
	width: parent.width
	height: isVisibleMenuItem()? implicitHeight : 0
	visible: isVisibleMenuItem()
	checkable: true
	signal valueUpdated()

	function getImgSource(name) {
		switch (name) {
		case "name": return (Material.background == "#303030")? "icons/name_d.png" : "icons/name_l.png"
		case "uimode": return (Material.background == "#303030")? "icons/qw_d.png" : "icons/qw_l.png"
		case "netmode": return (Material.background == "#303030")? "icons/netmode_d.png" : "icons/netmode_l.png"
		case "ethernet": return (Material.background == "#303030")? "icons/eth_d.png" : "icons/eth_l.png"
		case "wifi": return (Material.background == "#303030")? "icons/wifi_d.png" : "icons/wifi_l.png"
        case "web": return (Material.background == "#303030")? "icons/web_d.png" : "icons/web_l.png"
		case "backlight": return (Material.background == "#303030")? "icons/backlt_d.png" : "icons/backlt_l.png"
		}
		return ""
	}

	function updateNewValue(val) {
		model.newValue = val
		paramsDelegate.valueUpdated()
	}

	function isVisibleMenuItem() {
//        if (!mainWindow.withWifi)
//            if (model.name === "netmode" || model.name === "wifi")
//                return false
		return true
	}

	Component.onCompleted: {
		valueUpdated.connect(panelParams.showApplyButton)
	}

	onClicked: {
		ListView.view.currentIndex = index
		switch (model.name) {
		case "ethernet":
			ethernetParams.open()
			break
		case "wifi":
			wifiParams.open()
			break
		default:
			if (model.mutable) {
				if (model.settings) {
					model.info = false
					model.settings = false
				} else {
					model.info = !model.info
				}
			}
			break
		}
	}

	onPressAndHold: {
		if (model.mutable) {
			model.info = false
			model.settings = true
		}
	}

	contentItem: ColumnLayout {
		spacing: 0

		RowLayout {
			id: content
			spacing: 5
			Image {
				id: img
				width: label.font.pixelSize * 0.53 * 2
				height: label.height
				Layout.leftMargin: 5
				source: getImgSource(model.name)
				fillMode: Image.Stretch
				sourceSize.width: label.font.pixelSize * 0.53 * 2
				sourceSize.height: label.height
			}
			Label {
				id: label
				font.pixelSize: Qt.application.font.pixelSize * 2
				text: model.text
			}
		}
		Label {
			id: infoLabel
			Layout.leftMargin: img.width + img.Layout.leftMargin + content.spacing * 2
			visible: model.mutable && model.info
			font.pixelSize: Qt.application.font.pixelSize
			text: model.mutable? model.value : ""
		}
		// Panel name
		Flow {
			id: panelNameFlow
			Layout.fillWidth: true
			Layout.leftMargin: img.width + img.Layout.leftMargin + content.spacing * 2
			visible: model.mutable && model.settings && model.name === "name"
			onVisibleChanged: {
				if (visible)
					panelNameText.forceActiveFocus()
			}
			TextInputMono {
				id: panelNameText
				textFocus: panelNameFlow.visible
				maximumLength: uiPanelCtl.panelNameMaxLen()
				onTextChanged: updateNewValue(panelNameText.text)
                label: model.text
			}
		}
		// UI mode
		Flow {
			Layout.fillWidth: true
			Layout.leftMargin: img.width + img.Layout.leftMargin + content.spacing * 2
			visible: model.mutable && model.settings && model.name === "uimode"
			FittedComboBox {
                id: uimodeBox
				model: [
					uiPanelCtl.uiModeQmlConfig(),
					uiPanelCtl.uiModeWebBrowser()
				]
                onActivated: updateNewValue(uimodeBox.currentText)
			}
		}
		// Net mode
		Flow {
			Layout.fillWidth: true
			Layout.leftMargin: img.width + img.Layout.leftMargin + content.spacing * 2
			visible: model.mutable && model.settings && model.name === "netmode"
			FittedComboBox {
                id: netmodeBox
				model: [
					uiPanelCtl.panelNetModeEthernet(),
					uiPanelCtl.panelNetModeWifi(),
					uiPanelCtl.panelNetModeEthWifi()
				]
                onActivated: updateNewValue(netmodeBox.currentText)
			}
        }
		// Backlight timeout
		Flow {
			Layout.fillWidth: true
			Layout.leftMargin: img.width + img.Layout.leftMargin + content.spacing * 2
			visible: model.mutable && model.settings && model.name === "backlight"
			ColumnLayout {
				spacing: -10
				width: 150
				SpinBox {
					id: backLightSpin
					Layout.fillWidth: true
					from: 0
					to: 3600
					stepSize: 5
					onValueChanged: {
						backLightSlider.value = backLightSpin.value
						updateNewValue(backLightSpin.value.toString())
					}
				}
				Slider {
					id: backLightSlider
					Layout.fillWidth: true
					from: 0
					to: 3600
					stepSize: 50
					onValueChanged: {
						backLightSpin.value = backLightSlider.value
						updateNewValue(backLightSlider.value.toString())
					}
				}
			}
		}
	}
}
