import QtQuick

import QtQuick.Layouts

Rectangle {
    id: root
    height: 40
    color: "transparent"

    Component.onCompleted: root.width = parent.width

    MouseArea {
        anchors.fill: parent
        hoverEnabled: true

        onEntered: parent.color = "#023020"
        onExited: parent.color = "transparent"
        cursorShape: Qt.PointingHandCursor

        onClicked: {
            if (name === "Off" && type === "subtitle") {
                mediaPlayer.activeSubtitleTrack = -1
            } else if (type === "subtitle") {
                mediaPlayer.activeSubtitleTrack = index - 1 // Substract "Off" index
            } else {
                mediaPlayer.activeAudioTrack = index - 1 // Substract "Off" index
            }
        }
    }

    RowLayout {
        id: contentRow
        anchors.fill: parent

        Image {
            id: currentlySelected

            // TODO: add "success" icon
        }

        Text {
            id: metadataName
            Layout.minimumWidth: 20
            Layout.minimumHeight: 20
            Layout.leftMargin: type === "audio"? 15 : 0

            text: name
            font.pixelSize: 15
            color: "#ffffff"

            font.bold: true
        }
    }
}
