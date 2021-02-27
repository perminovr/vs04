import QtQuick 2.11
import QtQuick.Controls 2.4
import QtQuick.Controls.Material 2.4
import QtQuick.Layouts 1.11
import QtQuick.Window 2.11

ItemDelegate {
	id: ethernetDelegate
	width: parent.width
	checkable: true
	signal valueUpdated()

	function updateNewValue(val) {
		model.newValue = val
		ethernetDelegate.valueUpdated()
	}

	Component.onCompleted: {
		valueUpdated.connect(ethernetParams.showApplyButton)
	}

	onClicked: {
		ListView.view.currentIndex = index
		if (model.settings) {
			model.info = false
			model.settings = false
		} else {
			model.info = !model.info
		}
	}
	onPressAndHold: {
		model.info = false
		model.settings = true

		switch (model.name) {
		case "ip":
		case "mask":
		case "gw":
			model.settings = false
			winPanelParams.saveSlot = ethernetDelegate.updateNewValue
			netPrmDialog.header = model.text
			netPrmDialog.setValue(model.newValue.length?
					model.newValue : model.value)
			netPrmDialog.open()
			break
		}
	}

	contentItem: ColumnLayout {
		spacing: 0

		Label {
			font.pixelSize: Qt.application.font.pixelSize * 2
			text: model.text
		}
		Label {
			id: infoLabel
			visible: model.info
			font.pixelSize: Qt.application.font.pixelSize
			text: model.value
		}
		// Mode
		Flow {
			Layout.fillWidth: true
			visible: model.settings && model.name === "mode"
			FittedComboBox {
				id: modeBox
				model: [
					uiPanelCtl.netModeStatic(),
					uiPanelCtl.netModeDhcp()
				]
				onActivated: updateNewValue(modeBox.currentText)
			}
		}
	}
}
