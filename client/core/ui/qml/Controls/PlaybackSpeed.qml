import QtQuick
import QtQuick.Layouts
import "../Components"

Item {
    id: root
    Layout.minimumWidth: 100
    Layout.maximumWidth: 200

    property bool showPlaybackSpeedIcon: true
    property alias playbackSlider: playBackSpeedSlider

    RowLayout {
        anchors.fill: parent
        spacing: 10

        Image {
            visible: showPlaybackSpeedIcon
            Layout.minimumHeight: 13
            Layout.minimumWidth: 13
            source: "qrc:/images/svg/playback_rate.svg"
        }

        CustomSliderType {
            id: playBackSpeedSlider

            enableGradiant: true
            gradiantFirstColor: "#06D001"
            gradiantSecondColor: "#06D001"
            subPageColor: "#365E32"
            enableHandler: false

            sliderHeight: 7

            from: 0.5
            to: 2.5
            stepSize: 0.5
            value: 1.0

            onValueChanged: video.playbackRate = value

            Layout.fillWidth: true
        }

        Text {
            text: playBackSpeedSlider.value.toFixed(1) + "x"
            color: "#41CD52"
        }
    }
}
