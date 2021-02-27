import QtQuick 2.11
import QtQuick.Controls 2.4
import QtQuick.Controls.Material 2.4

Button {
    id: root
	property string sourceActive
	property string sourceDisable
	property bool activated: true
    property int explicitHeight: 20
    property int explicitWidth: 20
    height: explicitHeight
    width: explicitWidth

	background: Image {
		id: img
		fillMode: Image.PreserveAspectFit
        sourceSize.width: root.explicitWidth
        sourceSize.height: root.explicitHeight
		source: root.sourceActive
	}
	onClicked: {
		root.activated = !root.activated
		img.source = root.activated?
                root.sourceActive : root.sourceDisable
	}
}
