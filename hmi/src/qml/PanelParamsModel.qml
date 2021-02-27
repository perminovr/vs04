import QtQuick 2.11

ListModel {
	id: paramsModel

	// panel name
	ListElement {
		name: "name"
        img: (Material.background == "#303030")? "icons/name_d.png" : "icons/name_l.png"
		text: "Имя панели"
		mutable: true
		info: false
		settings: false
		value: ""
		newValue: ""
	}
	// ui mode
	ListElement {
		name: "uimode"
		text: "Режим UI"
		mutable: true
		info: false
		settings: false
		value: ""
		newValue: ""
	}
	// net mode
	ListElement {
		name: "netmode"
		text: "Режим работы сети"
		mutable: true
		info: false
		settings: false
		value: ""
		newValue: ""
	}
	// eth params
	ListElement {
		name: "ethernet"
		text: "Настройки Ethernet"
		mutable: false
	}
	// wifi params
	ListElement {
		name: "wifi"
		text: "Настройки Wifi"
		mutable: false
	}
    // webbrowser params
    ListElement {
        name: "web"
        text: "Настройки WebBrowser"
        mutable: false
    }
	// backlight to
	ListElement {
		name: "backlight"
		text: "Таймаут подсветки"
		mutable: true
		info: false
		settings: false
		value: ""
		newValue: ""
	}
}
