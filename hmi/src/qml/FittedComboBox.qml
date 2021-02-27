import QtQuick 2.11
import QtQuick.Controls 2.4
import QtQuick.Controls.Material 2.4

ComboBox {
    id: root
    Component.onCompleted: {
        if (root.count !== 0) {
            var maxWidth = root.width
            for (var i = 0; i < root.count; ++i) {
                var textSize = (root.model[i].length+1)*Qt.application.font.pixelSize
                maxWidth = Math.max(textSize, maxWidth)
            }
            root.implicitWidth = root.width = maxWidth*1.3
        }
    }
}
