import QtQuick 2.11
import QtQuick.Controls 2.4
import QtQuick.Controls.Material 2.4

RoundButton {
    id: root
    Material.elevation: 0
    implicitWidth: root.implicitHeight
    background: Rectangle {
        anchors.fill: parent
        radius: (parent.height+parent.width)/4
        color: Material.background == "#303030"? "#505050" : "#c8c8c8"
    }
}
