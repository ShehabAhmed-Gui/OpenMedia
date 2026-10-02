import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import "../theme"

Button {
    id: root

    property color backgroundColor: "transparent"
    property color onHoverBackgroundColor: Theme.hoverOverlay
    property color buttonBorderColor: "transparent"

    property color iconColor: Theme.textMuted
    property color iconHoverColor: Theme.text

    property double backgroundOpacity: 1
    property double buttonRadius: Theme.radiusControl

    property bool isHovered: root.hovered

    property int iconWidth: isMobileTarget ? 14 : 18
    property int iconHeight: isMobileTarget ? 14 : 18
    property url iconSource

    property int buttonWidth: Math.max(Theme.hitSize, iconWidth + Theme.lg)
    property int buttonHeight: Math.max(Theme.hitSize, iconHeight + Theme.lg)

    hoverEnabled: true
    padding: 0

    signal controlHovered(bool hovered)

    onHoveredChanged: controlHovered(hovered)

    onControlHovered: (hovered) => {
        if (hovered)
            afkTimer.stop();
        else
            afkTimer.start();
    }

    Layout.minimumWidth: buttonWidth
    Layout.minimumHeight: buttonHeight
    implicitWidth: buttonWidth
    implicitHeight: buttonHeight
    width: buttonWidth
    height: buttonHeight

    contentItem: AppIcon {
        source: root.iconSource
        size: Math.max(root.iconWidth, root.iconHeight)
        color: root.enabled ? (root.isHovered ? root.iconHoverColor : root.iconColor)
                            : Theme.textFaint
        anchors.centerIn: parent

        scale: root.down ? 0.88 : 1.0
        Behavior on scale {
            NumberAnimation { duration: Theme.durFast; easing.type: Theme.easeOut }
        }
    }

    background: Rectangle {
        radius: buttonRadius
        opacity: backgroundOpacity
        border.color: buttonBorderColor
        border.width: buttonBorderColor === "transparent" ? 0 : 1

        color: root.down ? Theme.pressOverlay
                         : root.isHovered ? onHoverBackgroundColor
                                          : backgroundColor

        Behavior on color {
            ColorAnimation { duration: Theme.durFast }
        }
    }

    Rectangle {
        anchors.fill: parent
        radius: parent.buttonRadius
        color: "transparent"
        border.color: Theme.accent
        border.width: 2
        visible: root.visualFocus
    }
}
