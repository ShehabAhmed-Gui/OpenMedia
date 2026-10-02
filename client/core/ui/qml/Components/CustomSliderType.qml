import QtQuick
import QtQuick.Controls

import "../theme"

Slider {
    id: customSlider
    live: true

    property color fillColor: Theme.accent
    property color fillColorDim: Theme.accentDim
    property color trackColor: Theme.hairline
    property color handleColor: Theme.text

    property alias sliderBackgroundRect: backgroundRect

    property int sliderWidth: 150
    property int sliderHeight: 4
    property int activeHeight: 6

    // Kept for call sites that still set them.
    property bool enableGradiant: true
    property bool enableHandler: false

    readonly property bool active: hovered || pressed

    focus: false
    padding: 0
    implicitHeight: Theme.hitSize

    background: Rectangle {
        id: backgroundRect
        x: customSlider.leftPadding
        y: customSlider.topPadding + customSlider.availableHeight / 2 - height / 2

        implicitWidth: sliderWidth
        width: customSlider.availableWidth
        height: customSlider.active ? activeHeight : sliderHeight
        radius: height / 2
        color: trackColor

        Behavior on height {
            NumberAnimation { duration: Theme.durFast; easing.type: Theme.easeOut }
        }

        Rectangle {
            width: customSlider.visualPosition * parent.width
            height: parent.height
            radius: parent.radius

            gradient: Gradient {
                orientation: Gradient.Horizontal
                GradientStop { position: 0.0; color: fillColorDim }
                GradientStop { position: 1.0; color: fillColor }
            }
        }

        MouseArea {
            anchors.fill: parent
            anchors.margins: -6
            hoverEnabled: true
            // Hover only: accepting buttons here would swallow the press and
            // the slider could never be dragged.
            acceptedButtons: Qt.NoButton

            onEntered: afkTimer.stop()
            onExited: afkTimer.start()
        }
    }

    handle: Rectangle {
        x: customSlider.leftPadding + customSlider.visualPosition * (customSlider.availableWidth - width)
        y: backgroundRect.y + backgroundRect.height / 2 - height / 2
        implicitWidth: 12
        implicitHeight: 12
        radius: width / 2
        color: handleColor

        scale: customSlider.active ? 1.0 : 0.0
        opacity: customSlider.active ? 1.0 : 0.0

        Behavior on scale {
            NumberAnimation { duration: Theme.durFast; easing.type: Easing.OutBack }
        }
        Behavior on opacity {
            NumberAnimation { duration: Theme.durFast }
        }

        Rectangle {
            anchors.centerIn: parent
            width: parent.width + 8
            height: parent.height + 8
            radius: width / 2
            color: fillColor
            opacity: 0.28
            z: -1
        }
    }
}
