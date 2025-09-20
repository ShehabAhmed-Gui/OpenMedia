import QtQuick
import QtQuick.Layouts
import "../Components"

Item {
    id: root
    Layout.minimumWidth: 150
    Layout.maximumWidth: 150

    property alias volumeSlider: volumeSlider
    property alias muteButton: muteButton
    property int volumeLevel

    function setMuted(muted) {
        mediaPlayer.audioOutput.muted = muted
    }

    RowLayout {
        id: audioRL
        anchors.fill: root
        spacing: 10

        CustomButton {
            id: muteButton

            ToolTipType {
                toolTipText: mediaPlayer.audioOutput.muted? "Unmute" : "Mute"
            }

              iconSource: ( mediaPlayer.audioOutput.muted || volumeLevel === 0? "qrc:/images/svg/muted.svg" : volumeLevel < 70
                   ? "qrc:/images/svg/volume_low.svg"
                   : "qrc:/images/svg/volume_high.svg")
            iconWidth: 13
            iconHeight: 13

            MouseArea {
                anchors.fill: parent
                cursorShape: parent.hovered? Qt.PointingHandCursor : Qt.ArrowCursor
                onClicked: {
                    setMuted(!mediaPlayer.audioOutput.muted)
                }
            }
        }

        CustomSliderType {
            id: volumeSlider

            Layout.minimumWidth: 100
            Layout.maximumWidth: 100

            enableGradiant: true
            gradiantFirstColor: "#04471c"
            gradiantSecondColor: "#054a29"
            subPageColor: "#111d13"
            handleColor: "#00563B"
            sliderHeight: 7

            from: 0
            to: 100
            value: (mediaPlayer.audioOutput.volume * 100).toFixed()

            onValueChanged: {
                volumeLevel = value
                mediaPlayer.audioOutput.volume = volumeSlider.value / 100
                // mediaPlayer = true
            }

            Connections {
                target: mediaPlayer.audioOutput
                property int videoVolume
                function onVolumeChanged() {
                    videoVolume = mediaPlayer.audioOutput.volume * 100
                    volumeSlider.value = videoVolume
                }
            }

            Layout.fillWidth: true
            Layout.alignment: Qt.AlignVCenter
        }

        Text {
            id: volumeLevelText
            text: volumeSlider.value
            font.pixelSize: 13

            color: "#f3eff5"
        }
    }
}
