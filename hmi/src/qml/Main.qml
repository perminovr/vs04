import QtQuick 2.11
import QtQuick.Window 2.2
import QtQuick.Controls 2.4
import QtQuick.Controls.Material 2.4
import QtQuick.Layouts 1.3

import Hmi 1.0

/* onCompleted order:
  1) parent
  2) the lowest child
  3) children of the child
  4) higher child...

*/

ApplicationWindow {
	id: window
	visible: true
    width: 800
    height: 480

	Material.theme: Material.Dark
//	Material.theme: Material.Light
	Material.accent: Material.Red

    flags: Qt.Window | Qt.FramelessWindowHint
    visibility: Window.FullScreen

	MainForm {
		id: mainWindow
        anchors.fill: parent

		function updatePanelReadOnlyOnceParams(values) {
			var val = values["ver_build"]
			if (val !== undefined && val.length) {
				winPanelExtCtl.verBuild = val
			}
			val = values["ver_uboot"]
			if (val !== undefined && val.length) {
				winPanelExtCtl.verUboot = val
			}
			val = values["ver_project"]
			if (val !== undefined && val.length) {
				winPanelExtCtl.verProject = val
			}
			val = values["ver_meta"]
			if (val !== undefined && val.length) {
				winPanelExtCtl.verMeta = val
			}
			val = values["ver_kernel"]
			if (val !== undefined && val.length) {
				winPanelExtCtl.verKernel = val
			}
		}

		function updatePanelReadOnlyParams(values) {
			var val = values["name"]
			if (val !== undefined && val.length) {
				winStatus.panelName = val
			}
			val = values["status"]
			if (val !== undefined) {
				winStatus.status = val
			}
			val = values["logflags"]
			if (val !== undefined) {
			}
			val = values["hmidat"]
			if (val !== undefined) {
			}
			val = values["eth_av"]
			if (val !== undefined) {
			}
			val = values["eth_link"]
			if (val !== undefined) {
			}
			val = values["wifi_av"]
			if (val !== undefined) {
			}
			val = values["wifi_link"]
			if (val !== undefined) {
			}
		}

//        Timer {
//            property int cnt: 0
//            id: t1
//            running: true
//            repeat: true
//            interval: 3000
//            onTriggered: {
//                mainSwipeView.currentIndex=2
//                wifiParams.open()
//                connectionWindow.open()
////				t1.cnt = t1.cnt + 1
////				uiPanelCtl.writeLog(UIPanelCtl.Note, "abc " + t1.cnt)
//            }
//        }

		Component.onCompleted: {
            uiPanelCtl.onWriteToLogList.connect(function(type, ts, msg){
                sysLog.addRecord(type, ts, msg)
            })
			uiPanelCtl.onSetReadOnlyOnceParams.connect(updatePanelReadOnlyOnceParams)
			uiPanelCtl.onSetCommonParams.connect(updatePanelReadOnlyParams)
			uiPanelCtl.onSetCommonParams.connect(panelParams.updateParams)
			uiPanelCtl.onSetEthernetParams.connect(ethernetParams.updateParams)
			uiPanelCtl.onSetWifiParams.connect(wifiParams.updateParams)
            uiPanelCtl.onSetWifiFoundApList.connect(connectionWindow.setApFoundList)
            uiPanelCtl.onSetWifiSavedApList.connect(connectionWindow.setApSavedList)
			panelParams.signalApply.connect(uiPanelCtl.applyCommonParams)
			ethernetParams.signalApply.connect(uiPanelCtl.applyEthernetParams)
			wifiParams.signalApply.connect(uiPanelCtl.applyWifiParams)
            winPanelExtCtl.requestOriginReset.connect(uiPanelCtl.originReset)
            connectionWindow.signalConnect.connect(uiPanelCtl.applyWifiApParams)
        }
    }

    CustomInputPanel {
        id: inputPanel
	}
}
