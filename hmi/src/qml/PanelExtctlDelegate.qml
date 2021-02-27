import QtQuick 2.11
import QtQuick.Controls 2.4
import QtQuick.Controls.Material 2.4
import QtQuick.Layouts 1.11
import QtQuick.Window 2.11

ItemDelegate {
	id: extctlDelegate
	width: parent.width
	checkable: true

	function getImgSource(name) {
		switch (name) {
		case "version": return (Material.background == "#303030")? "icons/version_d.png" : "icons/version_l.png"
		case "disptest": return (Material.background == "#303030")? "icons/test_d.png" : "icons/test_l.png"
		case "origreset": return (Material.background == "#303030")? "icons/reset_d.png" : "icons/reset_l.png"
		}
		return ""
	}

	onClicked: {
		ListView.view.currentIndex = index
		switch (model.name) {
		case "disptest":
			winPanelExtCtl.runDisplayTest()
			break
		case "origreset":
			winPanelExtCtl.openOriginResetDialog()
			break
		}
	}

	contentItem: ColumnLayout {
		spacing: 0
		visible: model.visible
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
		Flow {
			Layout.fillWidth: true
			Layout.leftMargin: img.width + img.Layout.leftMargin + content.spacing * 2
			visible: extctlDelegate.checked && model.name === "version"
			ColumnLayout {
				Label {
					text: "<b>build: </b>" + winPanelExtCtl.verBuild
				}
				Label {
					text: "<b>uboot: </b>" + winPanelExtCtl.verUboot
				}
				Label {
					text: "<b>project: </b>" + winPanelExtCtl.verProject
				}
				Label {
					text: "<b>meta: </b>" + winPanelExtCtl.verMeta
				}
				Label {
					text: "<b>kernel: </b>" + winPanelExtCtl.verKernel
				}
			}
		}
	}
}
