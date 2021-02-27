import QtQuick 2.11
import QtQuick.Controls 2.4
import QtQuick.Controls.Material 2.4
import QtQuick.Layouts 1.11

ItemDelegate {
    id: apConnectDelegate
    width: parent.width
    checkable: true

    signal apSelected(variant val)

    Component.onCompleted: {
    }

    onClicked: {
        ListView.view.currentIndex = index
        apSelectedDialog.security = model.security
        apSelectedDialog.ssid = model.ssid
        apSelectedDialog.bssid = model.bssid
        apSelectedDialog.uname = model.uname
        apSelectedDialog.passw = model.passw
        apSelectedDialog.open()
    }

/*
    "ssid" : ap["ssid"],
    "bssid" : ap["bssid"],
    "security" : ap["security"],
    "siglvl" : ap["siglvl"], //0..-200: 0 - better
    "fconn" : ap["fconn"]
*/
    contentItem: Item {
        anchors.fill: parent
        ColumnLayout {
            width: parent.width/2
            anchors.left: parent.left
            anchors.verticalCenter: parent.center
            Label {
                Layout.leftMargin: 20
                color: model.fconn? Material.accent : Material.foreground
                font.bold: model.fconn
                font.pixelSize: Qt.application.font.pixelSize * 1.7
                text: "SSID: " + model.ssid
            }
            Label {
                Layout.leftMargin: 20
                color: model.fconn? Material.accent : Material.foreground
                font.bold: model.fconn
                font.pixelSize: Qt.application.font.pixelSize * 0.9
                text: "BSID: " + model.bssid
            }
        }
        ColumnLayout {
            width: parent.width/2
            anchors.right: parent.right
            Label {
                Layout.leftMargin: -40
                color: model.fconn? Material.accent : Material.foreground
                font.bold: model.fconn
                font.pixelSize: Qt.application.font.pixelSize * 1.7
                text:   (model.security === 2)? "WPA-EAP" :
                        (model.security === 1)? "WPA-PSK" :
                        "None"
            }
            Label {
                Layout.leftMargin: -40
                color: model.fconn? Material.accent : Material.foreground
                font.bold: model.fconn
                font.pixelSize: Qt.application.font.pixelSize * 0.9
                text: "signal: " + model.siglvl
            }
        }
    }
}
