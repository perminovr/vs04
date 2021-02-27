import QtQuick 2.11
import QtQuick.Controls 2.4
import QtQuick.Controls.Material 2.4
import QtQuick.Layouts 1.3

HeadedWindow {
	id: root
	signal signalApply(variant values)
	property ListModel listModel: null
	property Component listDelegate: null

	function showApplyButton() {
		applyButton.visible = true
	}

	function updateParams(values) {
		var model = root_priv.paramsListView.model
		for (var i = 0; i < model.count; ++i) {
			var m = model.get(i)
			if (m.mutable) {
				var val = values[m.name]
				if (val !== undefined) {
					var str = val.toString()
					if (str.length) {
						m.value = str
					}
				}
			}
		}
	}

	Item {
		id: root_priv
		property ListView paramsListView: null

		function applyParams() {
			var values = new Object()
			var model = paramsListView.model
			for (var i = 0; i < model.count; ++i) {
				var m = model.get(i)
				if (m.mutable) {
					if (m.newValue.length) {
						values[m.name] = m.newValue
					}
				}
			}
			root.signalApply(values)
		}
	}

	Button {
		id: applyButton
		z: 1
		text: "Применить"
		anchors.right: parent.right
		anchors.top: parent.top
		anchors.rightMargin: 7
		visible: false
		onClicked: {
			root_priv.applyParams()
			applyButton.visible = false
		}
	}

	contentSource: Item {
		property alias paramsListView: paramsListView
		ListView {
			id: paramsListView
			anchors.fill: parent
			clip: true
			boundsBehavior: Flickable.StopAtBounds
		}
	}

	onLoaded: {
		root_priv.paramsListView = root.content.paramsListView
		root_priv.paramsListView.model = root.listModel
		root_priv.paramsListView.delegate = root.listDelegate
	}
}
