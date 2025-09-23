import QtQuick
import QtQuick.Controls
import QtQuick.Controls.Fusion
import QtQuick.Layouts

import "../controls"

Button {
    id: root

    property string backgroundColor: "transparent"
    property string onHoverBackgroundColor: "darkslategray"
    property string buttonBorderColor: "transparent"

    property double backgroundOpacity: 1
    property double buttonRadius: width / 2

    property bool isHovered: false

    property int iconWidth: isMobileTarget? 10 : 18
    property int iconHeight: isMobileTarget? 10 : 18
    property string iconSource

    property int buttonWidth: iconWidth + 10
    property int buttonHeight: iconHeight + 10

    signal controlHovered(bool hovered)

    onControlHovered: (hovered) => {
        switch (hovered) {
            case true: afkTimer.stop();
            break;
            case false: afkTimer.start();
            break;
        }
    }

    Layout.minimumWidth: buttonWidth
    Layout.minimumHeight: buttonHeight
    width: buttonWidth
    height: buttonHeight

    contentItem: Item {
        anchors.fill: parent

        Image {
            id: buttonIcon
            anchors.centerIn: parent

            smooth: true
            width: iconWidth
            height: iconHeight
            source: iconSource
        }
    }

    background: Rectangle {
        id: backgroundRec
        anchors.fill: parent
        width: parent.width
        height: parent.height

        MouseArea {
            id: mouseArea
            anchors.fill: parent
            preventStealing: true
            propagateComposedEvents: false
            hoverEnabled: true
            cursorShape: isHovered
                         ? Qt.PointingHandCursor : Qt.ArrowCursor

            onEntered: {
                controlHovered(true)
                isHovered = true
            }
            onExited: {
                controlHovered(false)
                isHovered = false
            }
        }

        radius: buttonRadius
        opacity: backgroundOpacity
        border.color: buttonBorderColor

        color: root.isHovered? onHoverBackgroundColor : backgroundColor
    }
}
