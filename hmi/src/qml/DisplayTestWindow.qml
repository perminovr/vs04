import QtQuick 2.11
import QtQuick.Controls 2.4
import QtQuick.Controls.Material 2.4
import QtQuick.Layouts 1.3

Rectangle {
	id: window

	enum State {
		Red,
		Green,
		Blue,
		Border
	}

	Rectangle {
		id: bgRect
		property int bgState: DisplayTestWindow.State.Red
		anchors.fill: parent

		MouseArea {
			anchors.fill: parent
			onClicked: {
				window.state = "hide"
				wTimer.running = false
				bgRect.bgState = DisplayTestWindow.State.Red
			}
		}
	}

	Timer {
		id: wTimer
		repeat: true
		onTriggered: {
			wTimer.interval = 2500
			switch (bgRect.bgState) {
			case DisplayTestWindow.State.Red:
				bgRect.color = "Red"
				bgRect.border.width = 0
				bgRect.bgState = DisplayTestWindow.State.Green
				break
			case DisplayTestWindow.State.Green:
				bgRect.color = "Green"
				bgRect.bgState = DisplayTestWindow.State.Blue
				break
			case DisplayTestWindow.State.Blue:
				bgRect.color = "Blue"
				bgRect.bgState = DisplayTestWindow.State.Border
				break
			case DisplayTestWindow.State.Border:
				bgRect.color = "Black"
				bgRect.border.color = "White"
				bgRect.border.width = 10
				bgRect.bgState = DisplayTestWindow.State.Red
				break
			}
		}
	}

	states: [
		State {
			name: "hide"
			PropertyChanges {
				target: window
				y: -height*2
			}
		},
		State {
			name: "show"
			PropertyChanges {
				target: window
				y: 0
			}
			PropertyChanges {
				target: wTimer
				interval: 1
				running: true
			}
		}
	]
}
