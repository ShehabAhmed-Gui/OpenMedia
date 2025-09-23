import QtQuick.Controls.Fusion
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import "../delegates"

Rectangle {
    id: root

    width: parent.width / 3
    height: parent.height / 2

    color: "#000000"

    anchors.margins: 10

    RowLayout {
        id: titlesContainer
        width: parent.width
        height: 50

        Text {
            id: audioTitle
            Layout.leftMargin: 20
            font.pixelSize: 17

            verticalAlignment: Text.AlignVCenter
            text: "Audio"
            color: "#ffffff"

            font.family: "Poppins"
            font.weight: Font.Bold
            font.bold: true
        }

        Item {
            Layout.minimumWidth: 5

        }

        Text {
            id: subtitleTitle

            verticalAlignment: Text.AlignVCenter
            text: "Subtitles"
            color: "#ffffff"

            font.bold: true
            font.pixelSize: 17
            font.family: "Poppins"
            font.weight: Font.Bold
        }
    }

    RowLayout {
        id: listsRow
        anchors.top: titlesContainer.bottom
        anchors.right: parent.right
        anchors.left: parent.left
        anchors.bottom: parent.bottom

        ListView {
            id: audioList
            Layout.fillWidth: true
            Layout.fillHeight: true

            model: audioProxyModel

            delegate: SubtitlesDelegate {
                onMetadataSelected: root.visible = false
            }
        }

        ListView {
            id: subtitleList
            Layout.fillWidth: true
            Layout.fillHeight: true

            model: subtitleProxyModel

            delegate: SubtitlesDelegate {
                onMetadataSelected: root.visible = false
            }
        }
    }
}
