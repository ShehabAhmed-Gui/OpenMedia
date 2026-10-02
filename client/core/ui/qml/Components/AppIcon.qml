import QtQuick
import QtQuick.Effects

import "../theme"

// Monochrome SVG tinted at runtime, so one file serves idle, hover and active.
Item {
    id: root

    property url source
    property color color: Theme.textMuted
    property int size: 18

    implicitWidth: size
    implicitHeight: size

    Image {
        id: bitmap
        anchors.fill: parent
        source: root.source
        sourceSize: Qt.size(root.size * 2, root.size * 2)
        fillMode: Image.PreserveAspectFit
        smooth: true
        visible: false
    }

    MultiEffect {
        anchors.fill: parent
        source: bitmap
        colorization: 1.0
        colorizationColor: root.color

        Behavior on colorizationColor {
            ColorAnimation { duration: Theme.durFast }
        }
    }
}
