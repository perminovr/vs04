import QtQuick 2.11
import QtQuick.Controls 2.4
import QtQuick.Controls.Material 2.4
import QtQuick.Layouts 1.11

ItemDelegate {
	id: wifiDelegate
	width: parent.width
	checkable: true
	signal valueUpdated()

	function updateNewValue(val) {
		model.newValue = val
		wifiDelegate.valueUpdated()
    }

	Component.onCompleted: {
        valueUpdated.connect(wifiParams.showApplyButton)
	}

	onClicked: {
		ListView.view.currentIndex = index
        switch (model.name) {
        case "connect":
            winPanelParams.saveSlot = wifiDelegate.updateNewValue
            connectionWindow.open()
            break
        default:
            if (model.settings) {
                model.info = false
                model.settings = false
            } else {
                model.info = !model.info
            }
            break
        }
	}
	onPressAndHold: {
		model.info = false
		model.settings = true

		switch (model.name) {
		case "ip":
		case "mask":
		case "gw":
			winPanelParams.saveSlot = wifiDelegate.updateNewValue
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
