import QtQuick
import QtQuick.Layouts
import QtMultimedia

Rectangle {
    id: root
    color: "#121212"
    anchors.horizontalCenter: parent? parent.horizontalCenter : undefined

    width: parent? parent.width - 20 : 20
    height: 40

    border.color: isCurrentlyPlaying? "#2E7D32" : "transparent"
    border.width: 0.5

    radius: 11

    property bool isCurrentlyPlaying: mediaPlayer.source.toString().toLowerCase() === path.toLowerCase()

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

        iconSource: isCurrentlyPlaying && videoState === MediaPlayer.PlayingState
            ? "qrc:/images/svg/pause.svg"
            : "qrc:/images/svg/play.svg"

        iconWidth: 22
        iconHeight: 22
        width: 24
        height: 24

        MouseArea {
            anchors.fill: parent
            cursorShape: parent.hovered? Qt.PointingHandCursor : Qt.ArrowCursor

            onClicked: {
                setCurrentIndex();

                if (isCurrentlyPlaying && mediaPlayer.playbackState === MediaPlayer.PlayingState) {
                    mediaPlayer.pause()
                } else {
                    mediaPlayer.source = path
                    mediaPlayer.play()
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

            font.pixelSize: 13
            font.weight: 400

            wrapMode: Text.NoWrap
        }
    }

    CustomButton {
        id: deleteItem
        anchors.verticalCenter: parent.verticalCenter
        anchors.right: parent.right
        anchors.rightMargin: 10

        iconSource: "qrc:/images/svg/trash.svg"

        ToolTipType {
            toolTipText: "Delete mediaPlayer"
        }

        MouseArea {
            anchors.fill: parent
            cursorShape: parent.hovered? Qt.PointingHandCursor : Qt.ArrowCursor

            onClicked: PlaylistModel.deleteItem(index);
        }
    }
}
