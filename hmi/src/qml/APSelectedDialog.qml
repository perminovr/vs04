import QtQuick 2.11
import QtQuick.Controls 2.4
import QtQuick.Controls.Material 2.4
import QtQuick.Layouts 1.11

CustomDialog {
    id: root
    signal save()

    explicitWidth: parent.width * 0.5
    explicitHeight: parent.height * 0.6
    scale: 1.2

    z: 70

    property var apSavedList: null

    property int security: 0 /* 0 - none, 1 - psk, 2 - eap */
    property string ssid: ""
    property string bssid: ""
    property string uname: ""
    property string passw: ""

    function setApSavedList(list) {
        root.apSavedList = list
    }

    onOpened: {
        var f = false
        if (apSavedList != null) {
            var sz = apSavedList.length
            for (var i = 0; i < sz; ++i) {
                var ap = apSavedList[i]
                if (ssid === ap["ssid"]) {
                    uname = ap["uname"]
                    passw = ap["passw"]
                    f = true
                    break
                }
            }
        }
        if (!f) { passw = uname = "" }
    }

    contentSource: MouseArea {
        ColumnLayout {
            anchors.fill: parent
            Label {
                width: parent.width
                Layout.alignment: Qt.AlignCenter
                Layout.topMargin: 5
                text: "SSID: " + root.ssid
                color: Material.foreground
                font.pixelSize: Qt.application.font.pixelSize * 2
            }
            Label {
                width: parent.width
                Layout.alignment: Qt.AlignCenter
                text: "BSSID: " + root.bssid
                color: Material.foreground
                font.pixelSize: Qt.application.font.pixelSize * 1
            }

            GridLayout {
                Layout.alignment: Qt.AlignCenter
                columns: 2
                Flow {
                    visible: securityBox.currentText === "WPA-EAP"
                    Label {
                    text : "User Name: "
                } }
                Flow {
                    visible: securityBox.currentText === "WPA-EAP"
                    TextInputMono {
						maximumLength: 16
						text: root.uname
						label: "User Name"
						onTextChanged: function() { root.uname = text }
                } }
                Flow {
                    visible: securityBox.currentText === "WPA-EAP" || securityBox.currentText === "WPA-PSK"
                    Label {
                    text : "Password:  "
                } }
                Flow {
                    visible: securityBox.currentText === "WPA-EAP" || securityBox.currentText === "WPA-PSK"
                    TextInputMono {
                        id: passwInput
                        echoMode: TextInput.Password
                        maximumLength: 16
                        text: root.passw
						label: "Password"
                        onTextChanged: function() { root.passw = text }
                } }
                Label {
                    text : "Security:  "
                }
                FittedComboBox {
                    id: securityBox
                    model: [ "None", "WPA-PSK", "WPA-EAP" ]
                    currentIndex: root.security
                    onCurrentIndexChanged: function() { root.security = currentIndex }
                }
            }

            DialogButtonBox {
                id: btnBox
                Layout.alignment: Qt.AlignRight | Qt.AlignBottom
                Layout.rightMargin: 5
                Layout.bottomMargin: 5
                background: Rectangle {
                    anchors.fill: parent
                    color: Material.background
                }
                Button {
                    text: "Добавить"
                    DialogButtonBox.buttonRole: DialogButtonBox.AcceptRole
                }
                Button {
                    text: "Отмена"
                    DialogButtonBox.buttonRole: DialogButtonBox.RejectRole
                }
                onAccepted: {
                    root.close()
                    root.save()
                }
                onRejected: {
                    root.close()
                }
            }
        }
    }
}
