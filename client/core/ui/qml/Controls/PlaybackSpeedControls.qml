import QtQuick
import QtQuick.Layouts

import "../components"
import "../theme"

Item {
    id: root

    property bool showPlaybackSpeedIcon: true
    property alias playbackSlider: playBackSpeedSlider

    RowLayout {
        anchors.fill: parent
        spacing: Theme.sm

        AppIcon {
            visible: showPlaybackSpeedIcon
            source: "qrc:/ui/icons/svg/playback_rate.svg"
            size: 16
            color: Theme.textMuted
            Layout.alignment: Qt.AlignVCenter
        }

        CustomSliderType {
            id: playBackSpeedSlider

            Layout.fillWidth: true
            Layout.alignment: Qt.AlignVCenter

            from: 0.5
            to: 2.5
            stepSize: 0.25
            value: MediaPlayerController.playbackRate

            onMoved: MediaPlayerController.playbackRate = value

            Connections {
                target: MediaPlayerController
                function onPlaybackRateChanged() {
                    // Dragging breaks the value binding, so restore it here.
                    if (!playBackSpeedSlider.pressed)
                        playBackSpeedSlider.value = MediaPlayerController.playbackRate
                }
            }
        }

        Text {
            Layout.minimumWidth: 34
            horizontalAlignment: Text.AlignRight

            text: playBackSpeedSlider.value.toFixed(2).replace(/\.?0+$/, "") + "x"
            color: playBackSpeedSlider.value === 1 ? Theme.textMuted : Theme.accent

            font.family: Theme.fontFamily
            font.pixelSize: Theme.captionSize
            font.weight: Font.Medium
            font.features: { "tnum": 1 }
        }
    }
}
