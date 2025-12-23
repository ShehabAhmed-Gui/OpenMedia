import QtQuick
import QtQuick.Layouts
import QtMultimedia

import "../components"

Rectangle {
    id: root
    color: "#2C3930"
    anchors.horizontalCenter: parent? parent.horizontalCenter : undefined

    width: parent? parent.width - 20 : 20
    height: 45
    border.color: isCurrentlyPlaying? "white" : "transparent"
    border.width: 0.5
    radius: 7

    property bool isCurrentlyPlaying: MediaPlayerController.source().toString().toLowerCase() === path.toLowerCase()
    property bool isMusicFile: path.endsWith(".mp3")

    function setCurrentIndex() {
        listView.currentIndex = index
        PlaylistModel.currentIndex = index
    }

    Component.onCompleted: {
        // Schedule the code for the next available event loop iteration
        Qt.callLater(() => {
            if (isCurrentlyPlaying) {
                if (listView) {
                    setCurrentIndex();
                }
            }
        });
    }

    Connections {
        target: listView
        function onPlayNext() {
            if (isCurrentlyPlaying) {
                setCurrentIndex();
            }
        }
    }

    Connections {
        target: listView
        function onPlayPrevious() {
            if (isCurrentlyPlaying) {
                setCurrentIndex();
            }
        }
    }

    CustomButton {
        id: playVideo
        anchors.left: parent.left
        anchors.leftMargin: 5
        anchors.verticalCenter: parent.verticalCenter

        iconSource: {
            if (isMusicFile && (!isCurrentlyPlaying || videoState !== MediaPlayer.PlayingState)) {
                return "qrc:/ui/icons/svg/music_media.svg"
            } else if (isCurrentlyPlaying && videoState === MediaPlayer.PlayingState) {
                return "qrc:/ui/icons/svg/pause.svg"
            } else {
                return "qrc:/ui/icons/svg/play.svg"
            }
        }

        iconWidth: isCurrentlyPlaying && videoState === Playback.Playing? 22 : 24
        iconHeight: isCurrentlyPlaying && videoState === Playback.Playing? 22 : 24
        width: 24
        height: 24

        ToolTipType {
            toolTipText: {
                if (isMusicFile && (!isCurrentlyPlaying || videoState !== Playback.Playing)) {
                    "Play this music"
                } else if (path.endsWith(".mp3") && videoState === Playback.Playing && isCurrentlyPlaying) {
                    "Stop this music"
                } else if (videoState === MediaPlayer.PlayingState && isCurrentlyPlaying) {
                    "Stop this video"
                } else {
                    "Play this video"
                }
            }
        }

        MouseArea {
            anchors.fill: parent
            cursorShape: parent.hovered? Qt.PointingHandCursor : Qt.ArrowCursor

            onClicked: {
                setCurrentIndex();

                if (isCurrentlyPlaying && MediaPlayerController.playbackState() === Playback.Playing) {
                    MediaPlayerController.stop()
                } else {
                    MediaPlayerController.open(path)
                    MediaPlayerController.play()
                }
            }
        }
    }

    Flickable {
        width: parent.width
        height: videoName.font.pixelSize + 5

        contentWidth: videoName.contentWidth
        contentHeight: height

        contentY: 0
        contentX: 0

        anchors {
            verticalCenter: parent.verticalCenter
            horizontalCenter: parent.horizontalCenter
            left: playVideo.right
            leftMargin: 7
        }

        clip: true

        Text {
            id: videoName
            text: name
            color: "#FFFFFF"

            width: videoName.contentWidth
            height: parent.height

            anchors {
                left: parent.left
                verticalCenter: parent.verticalCenter
            }

            font.family: "Poppins"
            font.pixelSize: 13
            font.weight: Font.Medium

            wrapMode: Text.NoWrap
        }
    }

    CustomButton {
        id: deleteItem
        anchors.verticalCenter: parent.verticalCenter
        anchors.right: parent.right
        anchors.rightMargin: 10

        iconSource: "qrc:/ui/icons/svg/trash.svg"

        ToolTipType {
            toolTipText: "Remove video"
        }

        MouseArea {
            anchors.fill: parent
            cursorShape: parent.hovered? Qt.PointingHandCursor : Qt.ArrowCursor

            onClicked: PlaylistModel.deleteItem(index);
        }
    }
}
