import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtMultimedia

import com.qt.openmedia

import "../Components"

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
    property alias bottomOpacity: bottomOpacity
    property alias bottomMA: bottomControlsMouseArea

    function seekBackward() {
        video.seek(video.position -= 10000)
    }

    function seekForward() {
        video.seek(video.position += 10000)
    }

    function formatTime(seconds) {
        var day = Math.floor(seconds / (24 * 3600));
        seconds %= 24 * 3600;
        var hour = Math.floor(seconds / 3600);
        seconds %= 3600;
        var minute = Math.floor(seconds / 60);
        var second = Math.floor(seconds % 60);
        return (day > 0 ? day + ":" : "") +
               (hour > 0 ? hour + ":" : "") +
               (minute > 0 ? (minute < 10 ? "0" + minute : minute) + ":" : "00:") +
               (second < 10 ? "0" + second : second);
    }

    property bool userChangingSlider: false
    property Timer userInteractionTimer: Timer {
        interval: 300
        repeat: false
        onTriggered: userChangingSlider = false
    }

    Connections {
        target: videoSlider
        function onMoved() {
            if (userChangingSlider) {
                video.seek(videoSlider.value * 1000)
            }
        }
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

    Rectangle {
        id: videoSliderContainer
        width: parent.width - 80
        color: "transparent"
        anchors.top: parent.top
        anchors.topMargin: 20

        CustomSliderType {
            id: videoSlider

            property int videoDuration: video.duration / 1000

            sliderWidth: parent.width - 30
            sliderHeight: 7

            anchors.verticalCenter: parent.verticalCenter
            anchors.left: parent.left
            anchors.leftMargin: 30

            from: 0
            to: videoDuration

            onValueChanged: {
                userChangingSlider = true
                userInteractionTimer.restart()
                video.focus = true
            }

            Connections {
                target: video
                function onPositionChanged() {
                    // Playing next video once current video ends
                    if (video.position === video.duration){
                        playlist.listView.playNext()
                    }

                    if (!userChangingSlider) {
                        videoSlider.value = video.position / 1000
                    }
                }
            }
        }
    }

    Text {
        id: videoTime
        text: formatTime(videoSlider.value)
        color: "#ffffff"
        font.pixelSize: 13

        anchors.left: videoSliderContainer.right
        anchors.verticalCenter: videoSliderContainer.verticalCenter
        anchors.leftMargin: 10
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

        PlaybackSpeed {
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
            spacing: Screen.primaryOrientation === Qt.LandscapeOrientation ? 17 : 10

            Layout.alignment: Qt.AlignHCenter | Qt.AlignVCenter
            Layout.fillWidth: true

            CustomButton {
                id: skipBackward
                buttonRadius: 0

                iconSource: "qrc:/images/svg/previous.svg"
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
                iconSource: "qrc:/images/backward_10s.png"
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
                iconSource: video.playbackState === MediaPlayer.PlayingState? "qrc:/images/svg/stop.svg" : "qrc:/images/svg/play.svg"
                iconWidth: 30
                iconHeight: 30

                ToolTipType {
                    toolTipText: video.playbackState === MediaPlayer.PlayingState? "Stop" : "Play"
                }

                onHoverBackgroundColor: "transparent"

                MouseArea {
                    anchors.fill: parent
                    hoverEnabled: true

                    onEntered: cursorShape = Qt.PointingHandCursor
                    onExited: cursorShape = Qt.ArrowCursor

                    onClicked: video.playbackState === MediaPlayer.PlayingState? video.pause() : video.play()
                }
            }

            CustomButton {
                id: seekForward
                iconSource: "qrc:/images/forward_10s.png"
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
                iconSource: "qrc:/images/svg/next.svg"
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

            // CustomButton {
            //     id: loop

            //     visible: Screen.primaryOrientation === Qt.LandscapeOrientation
            //     iconSource: "qrc:/images/svg/Loop_Icon_Dark.svg"
            // }
        }

        Item {
            Layout.fillWidth: true
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
            id: playListBtn
            Layout.alignment: Qt.AlignVCenter
            buttonRadius: 5

            backgroundColor: "transparent"

            iconSource: "qrc:/images/svg/playlist.svg"
            iconWidth: 16
            iconHeight: 16

            ToolTipType {
                toolTipText: playlist.width > 0? "Hide Playlist" : "Open Playlist"
            }

            MouseArea {
                anchors.fill: parent
                cursorShape: parent.hovered? Qt.PointingHandCursor : Qt.ArrowCursor

                onClicked: {
                    playlist.width > 0? hidePlaylist.start() : showPlayList.start()
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
