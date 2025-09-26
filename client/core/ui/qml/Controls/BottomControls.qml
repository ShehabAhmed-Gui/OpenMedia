import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtMultimedia

import "../components"

Rectangle {
    id: root
    width: parent.width
    height: 120
    color: "transparent"
    radius: 7
    anchors.bottom: parent.bottom

    property alias audioType: audioControl
    property alias playBackSpeedType: playBackSpeed

    property bool isMediaSliderPressed: videoSlider.pressed || audioControl.volumeSlider.pressed || playBackSpeed.playbackSlider.pressed
    property alias bottomOpacityRect: bottomOpacity
    property alias bottomMA: bottomControlsMouseArea

    function seekBackward() {
        mediaPlayer.position = mediaPlayer.position -= 10000
    }

    function seekForward() {
        mediaPlayer.position = mediaPlayer.position += 10000
    }

    function formatTime(fullVideoDuration, currentDuration) {
        // convert current time
        var spentSeconds = Math.floor(currentDuration / 1000);
        var remainingSecs = spentSeconds % 60;
        var spentMinutes = Math.floor((spentSeconds % 3600) / 60);
        var spentHours = Math.floor(spentSeconds / 3600);

        // format current time
        var formattedCurrent;
        if (spentHours > 0) {
            formattedCurrent =
                spentHours + ":" +
                String(spentMinutes).padStart(2, "0") + ":" +
                String(remainingSecs).padStart(2, "0");
        } else {
            formattedCurrent =
                spentMinutes + ":" +
                String(remainingSecs).padStart(2, "0");
        }

        // convert full time
        var totalSeconds = Math.floor(fullVideoDuration / 1000);
        var totalRemainingSecs = totalSeconds % 60;
        var totalMinutes = Math.floor((totalSeconds % 3600) / 60);
        var totalHours = Math.floor(totalSeconds / 3600);

        // format full time
        var formattedFull;
        if (totalHours > 0) {
            formattedFull =
                totalHours + ":" +
                String(totalMinutes).padStart(2, "0") + ":" +
                String(totalRemainingSecs).padStart(2, "0");
        } else {
            formattedFull =
                totalMinutes + ":" +
                String(totalRemainingSecs).padStart(2, "0");
        }

        // final YouTube style: spent / full
        return formattedCurrent + " / " + formattedFull;
    }

    property bool userChangingSlider: false
    property Timer userInteractionTimer: Timer {
        interval: 300
        repeat: false
        onTriggered: userChangingSlider = false
    }

    MouseArea {
        id: bottomControlsMouseArea
        anchors.fill: parent
        preventStealing: true
    }

    Rectangle {
        id: bottomOpacity

        anchors.fill: parent
        opacity: 0.40
        color: "#111111"
    }

    RowLayout {
        id: videoSliderContainer
        width: parent.width - 10
        anchors.top: parent.top
        anchors.topMargin: 20

        Item {
            Layout.minimumWidth: 10
        }

        Text {
            id: videoTime
            text: formatTime(mediaPlayer.duration, videoSlider.value)
            color: "#ffffff"

            font.pixelSize: 13
            font.family: "Poppins"
            font.weight: Font.Medium
        }

        Item {
            Layout.minimumWidth: 10
        }

        CustomSliderType {
            id: videoSlider

            property int videoDuration: mediaPlayer.duration

            sliderWidth: parent.width - 120
            sliderHeight: 7

            sliderBackgroundRect.width: parent.width - 120

            onMoved: () =>{
                if (userChangingSlider) {
                    mediaPlayer.position = videoSlider.value
                }
            }

            from: 0
            to: videoDuration

            onValueChanged: {
                userChangingSlider = true
                userInteractionTimer.restart()
                mediaPlayer.videoOutput.focus = true
            }

            Connections {
                target: mediaPlayer
                function onPositionChanged() {
                    // Playing next mediaPlayer once current mediaPlayer ends
                    if (mediaPlayer.position === mediaPlayer.duration){
                        playlist.listView.playNext()
                    }


                    videoSlider.value = mediaPlayer.position
                }
            }
        }
    }

    RowLayout {
        id: playerControls
        anchors.fill: root
        width: parent.width
        anchors.centerIn: parent

        Item {
            visible: Screen.primaryOrientation === Qt.LandscapeOrientation
            Layout.fillWidth: true
            Layout.minimumWidth: 40
            Layout.maximumWidth: 70
        }

        PlaybackSpeedControls {
            id: playBackSpeed
            showPlaybackSpeedIcon: Screen.primaryOrientation === Qt.LandscapeOrientation

            Layout.minimumWidth: isMobileTarget? 120 : 150
            Layout.maximumWidth: isMobileTarget? 120 : 150
            Layout.fillHeight: true
            Layout.fillWidth: true
        }

        Item {
            Layout.fillWidth: true
        }

        RowLayout {
            id: controlButtons
            spacing: Screen.primaryOrientation === Qt.LandscapeOrientation? 17 : 10
            anchors.horizontalCenter: parent.horizontalCenter
            Layout.alignment: Qt.AlignHCenter | Qt.AlignVCenter
            Layout.fillWidth: true

            CustomButton {
                id: skipBackward
                buttonRadius: 0

                iconSource: "qrc:/ui/icons/svg/previous.svg"
                iconWidth: 16
                iconHeight: 16

                ToolTipType {
                    toolTipText: "Skip To Previous Video"
                }

                MouseArea {
                    anchors.fill: parent
                    cursorShape: parent.hovered? Qt.PointingHandCursor : Qt.ArrowCursor

                    onClicked: {
                        playlist.listView.playPrevious()
                    }
                }
            }

            CustomButton {
                id: seekBackward
                iconSource: "qrc:/ui/icons/backward_10s.png"
                iconWidth: 17
                iconHeight: 17

                ToolTipType {
                    toolTipText: "Seek 10s backward"
                }

                MouseArea {
                    anchors.fill: parent
                    onClicked: root.seekBackward()
                    cursorShape: parent.hovered? Qt.PointingHandCursor : Qt.ArrowCursor
                }
            }

            CustomButton {
                id: startStopButton
                iconSource: mediaPlayer.playbackState === MediaPlayer.PlayingState
                            ? "qrc:/ui/icons/svg/stop.svg"
                            : "qrc:/ui/icons/svg/play.svg"
                iconWidth: 30
                iconHeight: 30

                ToolTipType {
                    toolTipText: mediaPlayer.playbackState === MediaPlayer.PlayingState? "Stop" : "Play"
                }

                onHoverBackgroundColor: "transparent"

                MouseArea {
                    anchors.fill: parent
                    cursorShape: parent.hovered? Qt.PointingHandCursor : Qt.ArrowCursor

                    onClicked: mediaPlayer.playbackState === MediaPlayer.PlayingState
                               ? mediaPlayer.pause()
                               : mediaPlayer.play()
                }
            }

            CustomButton {
                id: seekForward
                iconSource: "qrc:/ui/icons/forward_10s.png"
                iconWidth: 17
                iconHeight: 17

                ToolTipType {
                    toolTipText: "Seek 10s forward"
                }

                MouseArea {
                    anchors.fill: parent
                    onClicked: root.seekForward()
                    cursorShape: parent.hovered? Qt.PointingHandCursor : Qt.ArrowCursor
                }
            }

            CustomButton {
                id: skipForward
                buttonRadius: 0
                iconSource: "qrc:/ui/icons/svg/next.svg"
                iconWidth: 16
                iconHeight: 16

                ToolTipType {
                    toolTipText: "Skip To Next Video"
                }

                MouseArea {
                    anchors.fill: parent
                    cursorShape: parent.hovered? Qt.PointingHandCursor : Qt.ArrowCursor
                    onClicked: {
                        playlist.listView.playNext()
                    }
                }
            }

            CustomButton {
                id: loopBtn
                iconSource: "qrc:/ui/icons/svg/loop_disabled.svg"
                iconHeight: 20
                iconWidth: 20

                ToolTipType {
                    toolTipText: "Enable repeat"
                }

                MouseArea {
                    anchors.fill: parent
                    cursorShape: parent.hovered? Qt.PointingHandCursor : Qt.ArrowCursor

                    //onClicked:
                }
            }
        }

        AudioControls {
            id: audioControl
            Layout.minimumWidth: isMobileTarget? 120 : 150
            Layout.maximumWidth: isMobileTarget? 120 : 150
            Layout.fillHeight: true
            Layout.fillWidth: true
        }

        Item {
            Layout.fillWidth: true
            Layout.minimumWidth: 30
            Layout.maximumWidth: 30
        }

        CustomButton {
            id: playBackMetaDataBtn
            Layout.alignment: Qt.AlignVCenter
            buttonRadius: 5

            backgroundColor: "transparent"

            iconSource: "qrc:/ui/icons/svg/subtitles.svg"
            iconWidth: 23
            iconHeight: 23

            ToolTipType {
                toolTipText: metaDataPopup.visible? "Hide Subtitles" : "Show Subtitles"
            }

            MouseArea {
                anchors.fill: parent
                cursorShape: parent.hovered? Qt.PointingHandCursor : Qt.ArrowCursor

                onClicked: metaDataPopup.visible = !metaDataPopup.visible
            }
        }

        Item {
            Layout.minimumWidth: 10
            Layout.maximumWidth: 10
        }


        CustomButton {
            id: playListBtn
            Layout.alignment: Qt.AlignVCenter
            buttonRadius: 5

            backgroundColor: playlist.visible? "gray" : "transparent"

            iconSource: "qrc:/ui/icons/svg/playlist.svg"
            iconWidth: 16
            iconHeight: 16

            ToolTipType {
                toolTipText: playlist.visible? "Hide Playlist" : "Open Playlist"
            }

            MouseArea {
                anchors.fill: parent
                cursorShape: parent.hovered? Qt.PointingHandCursor : Qt.ArrowCursor

                onClicked: {
                    playlist.visible? hidePlaylist.start() : showPlayList.start()
                }
            }
        }

        Item {
            Layout.fillWidth: true
            Layout.minimumWidth: 20
            Layout.maximumWidth: 20
        }
    }
}
