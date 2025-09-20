import QtQuick
import QtQuick.Layouts

import "../Components"

Rectangle {
    id: root

    color: "#1E1E1E"
    radius: 8
    border.color: "#685752"

    property alias listView: listView

    MouseArea {
        anchors.fill: parent
        preventStealing: true
    }

    ColumnLayout {
        anchors.fill: parent
        spacing: 15

        Rectangle {
            width: parent.width
            height: 50
            color: "transparent"

            CustomButton {
                id: clearPlaylist
                anchors.verticalCenter: addItems.verticalCenter
                anchors.left: parent.left
                anchors.leftMargin: 20

                iconSource: "qrc:/images/svg/trash.svg"

                ToolTipType {
                    toolTipText: qsTr("Remove all videos")
                }

                MouseArea {
                    anchors.fill: parent
                    cursorShape: parent.hovered? Qt.PointingHandCursor : Qt.ArrowCursor
                    onClicked: PlaylistModel.clearPlaylist()
                }
            }

            Text {
                id: playlistTitle
                text: qsTr("Videos Playlist")

                color: "#ffffff"
                font.pixelSize: 13
                font.weight: 400

                anchors.centerIn: parent
                anchors.horizontalCenter: parent.horizontalCenter
            }

            CustomButton {
                id: addItems
                iconSource: "qrc:/images/svg/plus.svg"
                backgroundColor: "transparent"

                width: iconWidth + 5
                height: iconHeight + 5

                anchors.right: parent.right
                anchors.rightMargin: 20
                anchors.verticalCenter: parent.verticalCenter

                ToolTipType {
                    toolTipText: "Add videos"
                }

                MouseArea {
                    anchors.fill: parent
                    cursorShape: parent.hovered? Qt.PointingHandCursor : Qt.ArrowCursor

                    onClicked: PlaylistModel.loadVideos();
                }
            }
        }

        ListView {
            id: listView

            Layout.fillHeight: true
            Layout.fillWidth: true

            signal playNext()
            signal playPrevious()

            model: PlaylistModel

            spacing: 20
            clip: true

            delegate: VideosDelegate {
            }

            contentHeight: PlaylistModel.count * (delegate.height + spacing) + 10

            onPlayNext: {
                mediaPlayer.source = Qt.url(PlaylistModel.getNext())
                Qt.callLater(() => {
                    mediaPlayer.play()
                });
            }

            onPlayPrevious: {
                mediaPlayer.source = Qt.url(PlaylistModel.getPrevious())
                Qt.callLater(() => {
                    mediaPlayer.play()
                });
            }
        }

        Rectangle {
            id: sapcer
            width: listView.width
            height: 5
            color: "transparent"
        }
    }
}
