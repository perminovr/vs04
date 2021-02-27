import QtQuick 2.11
import QtQuick.Controls 2.4
import QtQuick.Controls.Material 2.4
import QtQuick.Layouts 1.3

CustomDialog {
	id: root
	property string header: ""
	property string value: ""
	signal save(string val)

	function setValue(val) {
		if (keypad != null) {
			root.value = val
			keypad.setValue(root.value)
		}
    }

	explicitClose: true
    explicitWidth: parent.width * 0.4
    explicitHeight: parent.height * 0.65
    animate: false

	scale: 1.2

	property NetKeypad keypad: null
	contentSource: MouseArea {
		property alias keypad: keypad
		ColumnLayout {
			anchors.fill: parent
			Label {
				Layout.alignment: Qt.AlignCenter
				Layout.topMargin: 5
				text: root.header
				color: Material.foreground
				font.pixelSize: Qt.application.font.pixelSize * 2
            }

			NetKeypad {
				Layout.alignment: Qt.AlignCenter
				id: keypad
			}

			DialogButtonBox {
				Layout.alignment: Qt.AlignRight | Qt.AlignBottom
				Layout.rightMargin: 5
				Layout.bottomMargin: 5
				background: Rectangle {
					anchors.fill: parent
					color: Material.background
				}
				Button {
					text: "Сброс"
					DialogButtonBox.buttonRole: DialogButtonBox.ResetRole
				}
				Button {
					text: "Сохранить"
					DialogButtonBox.buttonRole: DialogButtonBox.AcceptRole
				}
				Button {
					text: "Отмена"
					DialogButtonBox.buttonRole: DialogButtonBox.RejectRole
				}
				function formatVal(val) {
					var ret = ""
					val.split('.').forEach(function(v) {
						var i = parseInt(v, 10)
						v = i.toString()
						ret = ret + v + '.'
					})
					return ret.substr(0, ret.length-1)
				}
				onAccepted: {
					root.save(formatVal(keypad.value))
                    root.close()
				}
				onRejected: {
                    root.close()
				}
				onReset: {
					keypad.reset()
				}
			}
		}
	}
	onLoaded: {
		root.keypad = root.content.keypad
	}
}
