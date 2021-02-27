import QtQuick 2.11
import QtQuick.Controls 2.4
import QtQuick.Controls.Material 2.4
import QtQuick.Layouts 1.11

Item {
	id: root
	width: numKeypad.width
	height: numKeypad.height

	signal valueUpdated

	property string value
	property int maxCursorPos: 15

	function reset() {
		root_priv.textSpace.cursorPosition = 0
		root_priv.cursPos = 0
		root.value = "000.000.000.000"
	}

	function setValue(val) {
		reset()
		if (root_priv.isNetAddress(val))
			root.value = root_priv.formatValue(val)
	}

	Item {
		id: root_priv
		property alias cursPos: numKeypad.cursPos
		property alias textSpace: numKeypad.textSpace

		function setCharAt(str, index, chr) {
			return (index > str.length-1)?
				str :
				str.substr(0,index) + chr + str.substr(index+1);
		}

		function isNetAddress(addr) {
			return /^(25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\.(25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\.(25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\.(25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)$/.test(addr)
		}

		function formatValue(val) {
			var ret = ""
			val.split('.').forEach(function(v) {
				var i = parseInt(v, 10)
				if (i > 255) {
					i = 255
				}
				v = i.toString()
				if (i < 100) {
					if (i < 10) {
						v = '0' + v
					}
					v = '0' + v
				}
				ret = ret + v + '.'
			})
			return ret.substr(0, maxCursorPos)
		}

		function setValue(val) {
			var netaddr = formatValue(setCharAt(root.value, root_priv.textSpace.cursorPosition, val))
			if (root.value === netaddr) {
				numKeypad.moveCursor(+1)
			} else {
				var tmp = (root_priv.textSpace.cursorPosition == maxCursorPos)? 0 : 1
				root_priv.cursPos = root_priv.textSpace.cursorPosition + tmp
				root.value = netaddr
				root.valueUpdated()
			}
		}
	}

	NumKeypad {
		id: numKeypad
		value: root.value
		maxCursorPos: root.maxCursorPos

		textSpace.onCursorPositionChanged: {
			if (root_priv.textSpace.text[root_priv.textSpace.cursorPosition] === '.') {
				var val = (root_priv.textSpace.cursorPosition >= root_priv.cursPos)? 1 : -1;
				root_priv.textSpace.cursorPosition = root_priv.textSpace.cursorPosition + val
			}
			root_priv.cursPos = root_priv.textSpace.cursorPosition
		}
	}

	Component.onCompleted: {
		root.value = root_priv.isNetAddress(root.value)?
					root_priv.formatValue(root.value) : "000.000.000.000"
		numKeypad.setValue.connect(root_priv.setValue)
	}
}

