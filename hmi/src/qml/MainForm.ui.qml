import QtQuick 2.11
import QtQuick.Controls 2.4
import QtQuick.Window 2.11
import QtQuick.Controls.Material 2.4
import QtQuick.Layouts 1.3

import Hmi 1.0

Item {
	id: mainWindowUI
	property bool withWifi
	property alias winStatus: winStatus
    property alias mainSwipeView: mainSwipeView
	property alias panelParams: winPanelParams.panelParams
	property alias ethernetParams: winPanelParams.ethernetParams
	property alias wifiParams: winPanelParams.wifiParams
    property alias connectionWindow: winPanelParams.connectionWindow
    property alias winPanelExtCtl: winPanelExtCtl
    property alias sysLog: sysLog

	Item {
		anchors.fill: parent

		SwipeView {
			id: mainSwipeView
			anchors.fill: parent

			Item {
				id: win1
				width: window.width
				height: window.height
				WindowStatus {
					id: winStatus
					width: parent.width
					height: parent.height
				}
			}

			Item {
				id: win2
				width: window.width
				height: window.height
                WindowSysLog {
                    id: sysLog
                    width: parent.width
                    height: parent.height
                    listModel: ListModel {}
                    listDelegate: SysLogListDelegate {}
                }
			}

			Item {
				id: win3
				width: window.width
				height: window.height
				WindowParams {
					id: winPanelParams
					width: parent.width
					height: parent.height
				}
			}

			Item {
				id: win4
				width: window.width
				height: window.height
				WindowExtctl {
					id: winPanelExtCtl
					anchors.fill: parent
					head: "Расширенное управление"
					property string verBuild
					property string verUboot
					property string verProject
					property string verMeta
					property string verKernel
					contentSource: ListView {
						clip: true
						model: PanelExtctlModel {}
						delegate: PanelExtctlDelegate {}
						boundsBehavior: Flickable.StopAtBounds
					}
				}
			}
		}
	}
}

/*##^##
Designer {
	D{i:0;autoSize:true;height:480;width:640}
}
##^##*/


/*##^## Designer {
	D{i:0;autoSize:true;height:480;width:640}
}
 ##^##*/

