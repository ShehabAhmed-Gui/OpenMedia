import QtQuick

import QtQuick.Layouts

Rectangle {
    id: root
    height: 40
    color: "transparent"

    Component.onCompleted: root.width = parent.width

    signal metadataSelected()

    MouseArea {
        anchors.fill: parent
        hoverEnabled: true

        onEntered: parent.color = "#023020"
        onExited: parent.color = "transparent"
        cursorShape: Qt.PointingHandCursor

        onClicked: {
            metadataSelected()

            if (name === "Off" && type === "subtitle") {
                mediaPlayer.activeSubtitleTrack = -1
            } else if (type === "subtitle") {
                mediaPlayer.activeSubtitleTrack = index - 1 // Substract "Off" index
            } else {
                mediaPlayer.activeAudioTrack = index
            }
        }
    }

    RowLayout {
        id: contentRow
        anchors.fill: parent

        Image {
            id: currentlySelected
            Layout.leftMargin: type === "audio" &&
                               index == mediaPlayer.activeAudioTrack? 15 : 0

            Component.onCompleted: {
                if (type === "subtitle" && mediaPlayer.activeSubtitleTrack === index)
                    visible = rtue
                else if (type === "audio" && mediaPlayer.activeAudioTrack === index)
                    visible = true
            }

            source: "qrc:/ui/icons/svg/selected.svg"
            Layout.minimumWidth: 20
            Layout.maximumWidth: 20

            Layout.minimumHeight: 20
            Layout.maximumHeight: 20
            smooth: true
        }

        Text {
            id: metadataName
            Layout.minimumWidth: 20
            Layout.minimumHeight: 20

            text: name
            color: "#ffffff"

            font.pixelSize: 17
            font.family: "Poppins"
            font.weight: Font.Medium
            font.bold: true
        }

        Item {
            Layout.fillWidth: true
        }
    }
}
