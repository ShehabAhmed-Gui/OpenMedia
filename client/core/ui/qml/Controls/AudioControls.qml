import QtQuick
import QtQuick.Layouts

import "../components"
import "../theme"

Item {
    id: root

    property alias volumeSlider: volumeSlider
    property alias muteButton: muteButton
    property int volumeLevel: Math.round(MediaPlayerController.volume * 100)
    property bool muted: MediaPlayerController.muted
    readonly property bool boosted: volumeSlider.value > 100

    RowLayout {
        anchors.fill: parent
        spacing: Theme.sm

        CustomButton {
            id: muteButton

            iconSource: (muted || volumeLevel === 0)
                        ? "qrc:/ui/icons/svg/muted.svg"
                        : volumeLevel < 60 ? "qrc:/ui/icons/svg/volume_low.svg"
                                           : "qrc:/ui/icons/svg/volume_high.svg"
            iconWidth: 14
            iconHeight: 14
            iconColor: muted ? Theme.textFaint : boosted ? Theme.boost : Theme.textMuted

            ToolTipType { toolTipText: muted ? "Unmute" : "Mute" }

            onClicked: MediaPlayerController.muted = !MediaPlayerController.muted
        }

        CustomSliderType {
            id: volumeSlider

            Layout.fillWidth: true
            Layout.alignment: Qt.AlignVCenter

            from: 0
            to: Math.round(MediaPlayerController.maxVolume * 100)
            stepSize: 1
            value: Math.round(MediaPlayerController.volume * 100)

            fillColor: root.boosted ? Theme.boost : Theme.accent

            opacity: muted ? 0.45 : 1.0
            Behavior on opacity { NumberAnimation { duration: Theme.durFast } }

            onMoved: {
                // Detent so 100% is easy to land on.
                if (Math.abs(volumeSlider.value - 100) <= 3)
                    volumeSlider.value = 100
                MediaPlayerController.volume = volumeSlider.value / 100
            }

            Rectangle {
                parent: volumeSlider.sliderBackgroundRect
                x: parent.width * 100 / volumeSlider.to - width / 2
                anchors.verticalCenter: parent.verticalCenter
                width: 2
                height: parent.height + 4
                radius: 1
                color: Theme.textFaint
            }

            Connections {
                target: MediaPlayerController
                function onVolumeChanged() {
                    if (!volumeSlider.pressed)
                        volumeSlider.value = Math.round(MediaPlayerController.volume * 100)
                }
            }
        }

        Text {
            Layout.minimumWidth: 34
            horizontalAlignment: Text.AlignRight

            text: volumeSlider.value + "%"
            color: root.boosted ? Theme.boost : Theme.textMuted

            font.family: Theme.fontFamily
            font.pixelSize: Theme.captionSize
            font.weight: Font.Medium
            font.features: { "tnum": 1 }
        }
    }
}
