import QtQuick 2.11

ListModel {
	id: extctlModel

	// version
	ListElement {
		name: "version"
		text: "Версия устройства"
		visible: true
	}
	// Common test
	ListElement {
		name: "disptest"
		text: "Проверка экрана"
		visible: true
	}
	// Original reset
	ListElement {
		name: "origreset"
		text: "Заводской сброс"
		visible: true
	}
}
