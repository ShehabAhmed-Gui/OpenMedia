import QtQuick

import "../theme"

Rectangle {
    id: root

    width: ListView.view ? ListView.view.width : 0
    height: 34
    radius: Theme.radiusControl
    color: hoverHandler.hovered ? Theme.raised : "transparent"

    signal metadataSelected()

    Behavior on color {
        ColorAnimation { duration: Theme.durFast }
    }

    HoverHandler {
        id: hoverHandler
        cursorShape: Qt.PointingHandCursor
    }

    TapHandler {
        onTapped: metadataSelected()
    }

    Text {
        anchors.left: parent.left
        anchors.leftMargin: Theme.sm
        anchors.right: parent.right
        anchors.rightMargin: Theme.sm
        anchors.verticalCenter: parent.verticalCenter

        text: name
        color: Theme.textMuted
        elide: Text.ElideRight

        font.family: Theme.fontFamily
        font.pixelSize: Theme.labelSize
        font.weight: Font.Medium
    }
}
