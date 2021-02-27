import QtQuick 2.11
import QtQuick.Controls 2.4
import QtQuick.Controls.Material 2.4
import QtQuick.Layouts 1.3

import Hmi 1.0

Item {
	id: root
	property int maxRecords: 100
	property ListModel listModel
	property Component listDelegate

	Item {
		id: root_priv
		property alias storage: storage

		property IconButton noteBtn
		property IconButton warnBtn
		property IconButton errBtn
		property IconButton dbgBtn
		property CheckBox autoUpdChkBx
		property ListView logListView

		ListModel {
			id: storage
		}

		ListModel {
			id: backup
		}

		function assignProperties() {
			noteBtn = hwin.content.noteBtn
			warnBtn = hwin.content.warnBtn
			errBtn = hwin.content.errBtn
			dbgBtn = hwin.content.dbgBtn
			autoUpdChkBx = hwin.content.autoUpdChkBx
			logListView = hwin.content.logListView
			logListView.model = root.listModel
			logListView.delegate = root.listDelegate
		}

		function makeBackup() {
			backup.clear()
			for (var i = 0; i < storage.count; ++i) {
				backup.append(storage.get(i))
			}
		}

		function modelAddItem(model, type, ts, text) {
			if (model.count === maxRecords) {
				var idx = model.count-1
				model.move(0, idx, 1)
				model.set(idx, {
					"text" : text,
					"type" : type,
					"ts"   : ts
				})
			} else {
				model.append({
					"text" : text,
					"type" : type,
					"ts"   : ts
				})
			}
		}

		function isVisibleItem(type) {
			switch (type) {
			case UIPanelCtl.Note:
				return (noteBtn.activated)
			case UIPanelCtl.Warning:
				return (warnBtn.activated)
			case UIPanelCtl.Error:
				return (errBtn.activated)
			case UIPanelCtl.Debug:
				return (dbgBtn.activated)
			}
		}

		function addViewedRecord(model, type, ts, text) {
			if (autoUpdChkBx.checked) {
				if (isVisibleItem(type)) {
					modelAddItem(model, type, ts, text)
				}
				logListView.positionViewAtEnd()
			}
		}

		function updateViewedModel() {
			var visibleModel = logListView.model
			var realModel = (autoUpdChkBx.checked)? storage : backup
			visibleModel.clear()
			for (var i = 0; i < realModel.count; ++i) {
				var m = realModel.get(i)
				if (isVisibleItem(m.type)) {
					visibleModel.append(m)
				}
			}
			logListView.positionViewAtEnd()
		}
	}

	function addRecord(type, ts, text) {
		root_priv.modelAddItem(root_priv.storage, type, ts, text)
		root_priv.addViewedRecord(root_priv.logListView.model, type, ts, text)
	}

	HeadedWindow {
		id: hwin
		anchors.fill: parent
		head: "Журнал работы"

		contentSource: ColumnLayout {
			property alias noteBtn: noteBtn
			property alias warnBtn: warnBtn
			property alias errBtn: errBtn
			property alias dbgBtn: dbgBtn
			property alias logListView: logListView
			property alias autoUpdChkBx: autoUpdChkBx
			RowLayout {
				Layout.rightMargin: 20
				Layout.alignment: Qt.AlignRight
				IconButton {
					id: noteBtn
                    sourceActive: "icons/info_s.png"
                    sourceDisable: "icons/info_s_sh.png"
					onClicked: root_priv.updateViewedModel()
				}
				IconButton {
					id: warnBtn
                    sourceActive: "icons/warn_s.png"
                    sourceDisable: "icons/warn_s_sh.png"
					onClicked: root_priv.updateViewedModel()
				}
				IconButton {
					id: errBtn
                    sourceActive: "icons/err_s.png"
                    sourceDisable: "icons/err_s_sh.png"
					onClicked: root_priv.updateViewedModel()
				}
				IconButton {
					id: dbgBtn
                    sourceActive: "icons/debug_s.png"
                    sourceDisable: "icons/debug_s_sh.png"
					onClicked: root_priv.updateViewedModel()
				}
			}
			Rectangle {
				color: "White"
				radius: 4
				Layout.fillWidth: true
				Layout.fillHeight: true
				Layout.alignment: Qt.AlignVCenter |  Qt.AlignHCenter
				Layout.leftMargin: 20
				Layout.rightMargin: 20
				ListView {
					id: logListView
					anchors.fill: parent
					clip: true
					spacing: -17
					cacheBuffer: 50
					interactive: !autoUpdChkBx.checked
					boundsBehavior: Flickable.StopAtBounds
				}
			}
			CheckBox {
				id: autoUpdChkBx
				text: "Автоматическое обновление"
				Layout.leftMargin: 20
				Layout.bottomMargin: 20
				checked: true
				onCheckStateChanged: {
					if (autoUpdChkBx.checked) {
						root_priv.updateViewedModel()
					} else {
						root_priv.makeBackup()
					}
				}
			}
		}

		onLoaded: root_priv.assignProperties()
	}
}
