import QtQuick 2.11
import QtQuick.Controls 2.4
import QtQuick.Controls.Material 2.4
import QtQuick.Layouts 1.3

import Hmi 1.0

ScalableLwWindow {
    id: root

    signal signalConnect(variant values)
    signal apSavedChanged(var list)

    Component.onCompleted: {
        root.signalEnter.connect(function(){ uiPanelCtl.wifiSearchWindowVisibleChanged(true) })
        root.signalExit.connect(function(){ uiPanelCtl.wifiSearchWindowVisibleChanged(false) })
    }

    function setApFoundList(list) { // qt 5.14 ok todo
        var sz = list.length
        var model = root_priv.apsListView.model
        model.clear()
        for (var i = 0; i < sz; ++i) {
            var ap = list[i]
            if (ap["fconn"] === true) {
                model.insert(0, root_priv.apToModelFormat(ap))
            } else {
                model.append(root_priv.apToModelFormat(ap))
            }
        }
    }

    function setApSavedList(list) {
        apSavedChanged(list)
    }

    Item {
        id: root_priv

        property ListView apsListView: null
        property ListModel listModel: ListModel {}
        property Component listDelegate: APConnectDelegate {}

        property string ssid: ""
        property string bssid: ""
        property string uname: ""
        property string passw: ""

        function connectTo() {
            var values = new Object()
            values["ssid"] = ssid
            values["bssid"] = bssid
            values["uname"] = uname
            values["passw"] = passw
			if (values["ssid"] === "скрытая_сеть") { values["ssid"] = "" }
            root.signalConnect(values)
        }

        function apToModelFormat(ap) {
			if (ap["ssid"] === "") { ap["ssid"] = "скрытая_сеть" }
            return {
                "ssid" : ap["ssid"],
                "bssid" : ap["bssid"],
                "security" : ap["security"],
                "siglvl" : ap["siglvl"],
                "uname" : ap["uname"],
                "passw" : ap["passw"],
                "fconn" : ap["fconn"]
            }
        }

        Component.onCompleted: {
            apNewDialog.save.connect(function(){
                ssid = apNewDialog.ssid
                bssid = ""
                uname = apNewDialog.uname
                passw = apNewDialog.passw
                connectTo()
            })
            apSelectedDialog.save.connect(function(){
                ssid = apSelectedDialog.ssid
                bssid = apSelectedDialog.bssid
                uname = apSelectedDialog.uname
                passw = apSelectedDialog.passw
                connectTo()
            })
            root.apSavedChanged.connect(apNewDialog.setApSavedList)
            root.apSavedChanged.connect(apSelectedDialog.setApSavedList)
        }
    }

    contentSource: Item {
        property alias apsListView: apsListView
        ColumnLayout {
            width: parent.width
            height: parent.height - (60)
            Rectangle {
                height: parent.height - btnNewAp.height
                color: Material.background
                radius: 4
                Layout.fillWidth: true
                Layout.fillHeight: true
                Layout.alignment: Qt.AlignHCenter
                Layout.leftMargin: 20
                Layout.rightMargin: 20
                ListView {
                    id: apsListView
                    anchors.fill: parent
                    clip: true
                    cacheBuffer: 50
                    boundsBehavior: Flickable.StopAtBounds
                }
            }
            MouseArea {
                id: btnNewAp
                height: 50
                Layout.fillWidth: true
                Layout.alignment: Qt.AlignHCenter

                Rectangle {
                    anchors.fill: parent
                    color: Material.background
                    border.color: "White"
                    border.width: 1
                    Label {
                        anchors.centerIn: parent
                        text: "Новая точка доступа"
                        color: Material.foreground
                        font.pixelSize: Qt.application.font.pixelSize * 1.4
                    }
                }

                onClicked: {
                    apNewDialog.open()
                }
            }
        }
    }

    onLoaded: {
        root_priv.apsListView = root.content.apsListView
        root_priv.apsListView.model = root_priv.listModel
        root_priv.apsListView.delegate = root_priv.listDelegate
    }

}
