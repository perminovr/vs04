import QtQuick 2.11
import QtQuick.Controls 2.4
import QtQuick.Controls.Material 2.4

Item {
	id: root
	property alias panelParams: panelParams
	property alias ethernetParams: ethernetParams
	property alias wifiParams: wifiParams
	property alias netPrmDialog: netPrmDialog
    property alias connectionWindow: wifiParams.connectionWindow

	ParameterizedWindow {
		id: panelParams
		width: parent.width
		height: parent.height
		head: "Параметры панели"
		listModel: PanelParamsModel {}
		listDelegate: PanelParamsDelegate {}
	}
	//MovableParamWindow {
	ScalableParamWindow {
		id: ethernetParams
		width: parent.width
		height: parent.height
		head: "Настройки Ethernet"
		listModel: EthernetParamsModel {}
		listDelegate: EthernetParamsDelegate {}
	}
	//MovableParamWindow {
	ScalableParamWindow {
		id: wifiParams
        property alias connectionWindow: connectionWindow
        property alias apNewDialog: apNewDialog
        property alias apSelectedDialog: apSelectedDialog
		width: parent.width
		height: parent.height
		head: "Настройки Wifi"
		listModel: WifiParamsModel {}
		listDelegate: WifiParamsDelegate {}
        APConnectWindow {
            id: connectionWindow
            width: parent.width
            height: parent.height
            head: "Поиск точки доступа"
        }
        APNewDialog {
            id: apNewDialog
        }
        APSelectedDialog {
            id: apSelectedDialog
        }
	}

	NetParamDialog {
		id: netPrmDialog
	}
	property var saveSlot
	function _saveFunc(val) {
		if (saveSlot !== undefined)
			saveSlot(val) // delegates slot
		saveSlot = undefined
	}
	Component.onCompleted: {
		netPrmDialog.save.connect(_saveFunc)
	}
}
