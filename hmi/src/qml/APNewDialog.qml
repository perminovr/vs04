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
    property string uname: ""
    property string passw: ""

    function setApSavedList(list) {
        root.apSavedList = list
    }

    contentSource: MouseArea {
        ColumnLayout {
            anchors.fill: parent
            Label {
                id: header
                width: parent.width
                Layout.alignment: Qt.AlignCenter
                Layout.topMargin: 5
                text: "Новая точка доступа"
                color: Material.foreground
                font.pixelSize: Qt.application.font.pixelSize * 2
            }

            GridLayout {
                Layout.alignment: Qt.AlignCenter
                columns: 2
                Label {
                    text : "SSID:      "
                }
                TextInputMono {
                    id: timSsid
                    maximumLength: 16
                    text: root.ssid
					label: "SSID"
                    onTextChanged: function() {
                        root.ssid = text
                        if (root.apSavedList != null) {
                            var sz = root.apSavedList.length
                            for (var i = 0; i < sz; ++i) {
                                var ap = root.apSavedList[i]
                                if (root.ssid === ap["ssid"]) {
                                    root.uname = ap["uname"]
                                    root.passw = ap["passw"]
                                    unameInput.text = root.uname
                                    passwInput.text = root.passw
                                    if (root.passw.length > 0) {
                                        root.security = (root.uname.length > 0)? 2 : 1
                                    }
                                    securityBox.currentIndex = root.security
                                    break
                                }
                            }
                        }
                    }
                }
                Flow {
                    visible: securityBox.currentText === "WPA-EAP"
                    Label {
                        text : "User Name: "
                } }
                Flow {
                    visible: securityBox.currentText === "WPA-EAP"
                    TextInputMono {
						id: unameInput
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
