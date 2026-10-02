import QtQuick
import QtQuick.Layouts

import "../delegates"
import "../theme"

Rectangle {
    id: root

    width: 360
    height: 300

    color: Qt.rgba(Theme.surface.r, Theme.surface.g, Theme.surface.b, 0.94)
    radius: Theme.radiusPanel
    border.color: Theme.hairline
    border.width: 1

    MouseArea {
        anchors.fill: parent
        preventStealing: true
        hoverEnabled: true
        onEntered: afkTimer.stop()
        onExited: afkTimer.start()
    }

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: Theme.lg
        spacing: Theme.md

        RowLayout {
            Layout.fillWidth: true
            spacing: 0

            Text {
                Layout.fillWidth: true
                text: qsTr("Audio")
                color: Theme.text
                font.family: Theme.fontFamily
                font.pixelSize: Theme.labelSize
                font.weight: Font.DemiBold
            }

            Text {
                Layout.fillWidth: true
                text: qsTr("Subtitles")
                color: Theme.text
                font.family: Theme.fontFamily
                font.pixelSize: Theme.labelSize
                font.weight: Font.DemiBold
            }
        }

        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: 1
            color: Theme.hairline
        }

        RowLayout {
            Layout.fillWidth: true
            Layout.fillHeight: true
            spacing: Theme.md

            ListView {
                Layout.fillWidth: true
                Layout.fillHeight: true
                clip: true
                model: audioProxyModel
                delegate: SubtitlesDelegate {
                    onMetadataSelected: root.visible = false
                }
            }

            ListView {
                Layout.fillWidth: true
                Layout.fillHeight: true
                clip: true
                model: subtitleProxyModel
                delegate: SubtitlesDelegate {
                    onMetadataSelected: root.visible = false
                }
            }
        }

        Text {
            Layout.fillWidth: true
            visible: audioProxyModel.count === 0 && subtitleProxyModel.count === 0
            text: qsTr("Track switching is not wired to the ffmpeg backend yet.")
            color: Theme.textFaint
            wrapMode: Text.WordWrap
            font.family: Theme.fontFamily
            font.pixelSize: Theme.captionSize
            font.weight: Font.Medium
        }
    }
}
