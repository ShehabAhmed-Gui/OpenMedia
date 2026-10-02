import QtQuick
import QtQuick.Controls

import "../theme"

ToolTip {
    id: root

    property string toolTipText: ""

    // Long enough that sweeping the pointer across the bar stays quiet.
    delay: 400
    visible: parent.hovered && toolTipText.length > 0
    padding: Theme.sm

    background: Rectangle {
        radius: Theme.radiusControl
        color: Theme.raised
        border.color: Theme.hairline
        border.width: 1
    }

    contentItem: Text {
        text: root.toolTipText
        color: Theme.text

        font.family: Theme.fontFamily
        font.pixelSize: Theme.captionSize
        font.weight: Font.Medium
    }

    enter: Transition {
        NumberAnimation { property: "opacity"; from: 0; to: 1; duration: Theme.durFast }
    }
    exit: Transition {
        NumberAnimation { property: "opacity"; from: 1; to: 0; duration: Theme.durFast }
    }
}
