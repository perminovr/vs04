import QtQuick 2.11
import QtQuick.Controls 2.4
import QtQuick.Controls.Material 2.4

HeadedWindow {
	id: root
	signal requestOriginReset()

	function openOriginResetDialog() {
		dialog.open()
	}

	function runDisplayTest() {
		dispTest.state = "show"
	}

	DisplayTestWindow {
		id: dispTest
		width: parent.width
		height: parent.height
		state: "hide"
	}

	CustomDialog {
		id: dialog
		explicitWidth: parent.width / 2
		explicitHeight: parent.height / 2
		contentSource: MouseArea {
			Label {
				text: "Сбросить все настройки?"
				font.pixelSize: Qt.application.font.pixelSize * 1.5
				anchors.centerIn: parent
			}
			DialogButtonBox {
				anchors.bottom: parent.bottom
				anchors.right: parent.right
				anchors.rightMargin: 5
				anchors.bottomMargin: 5
				background: Rectangle {
					anchors.fill: parent
					color: Material.background
				}
				Button {
					text: "Сбросить"
					DialogButtonBox.buttonRole: DialogButtonBox.AcceptRole
				}
				Button {
					text: "Отмена"
					DialogButtonBox.buttonRole: DialogButtonBox.RejectRole
				}
				onAccepted: {
					dialog.close()
					root.requestOriginReset()
				}
				onRejected: {
					dialog.close()
				}
			}
		}
	}
}
