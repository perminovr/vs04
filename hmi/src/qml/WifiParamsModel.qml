import QtQuick 2.11

ListModel {
	id: wifiModel

    // connect
	ListElement {
        name: "connect"
        text: "Поиск точки доступа"
        mutable: false
		info: false
		settings: false
        value: ""
		newValue: ""
    }
	// static/dhcp
	ListElement {
		name: "mode"
		text: "Режим работы"
		mutable: true
		info: false
		settings: false
		value: ""
		newValue: ""
	}
	// ip
	ListElement {
		name: "ip"
		text: "IP адрес"
		mutable: true
		info: false
		settings: false
		value: ""
		newValue: ""
	}
	// mask
	ListElement {
		name: "mask"
		text: "Маска сети"
		mutable: true
		info: false
		settings: false
		value: ""
		newValue: ""
	}
	// gateway
	ListElement {
		name: "gw"
		text: "Шлюз"
		mutable: true
		info: false
		settings: false
		value: ""
		newValue: ""
	}
}
