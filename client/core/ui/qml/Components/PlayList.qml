import QtQuick
import QtQuick.Layouts

import "../delegates"

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

                iconSource: "qrc:/ui/icons/svg/trash.svg"

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

                font.family: "Poppins"
                font.pixelSize: 13
                font.weight: Font.Normal

                anchors.centerIn: parent
                anchors.horizontalCenter: parent.horizontalCenter
            }

            CustomButton {
                id: addItems
                iconSource: "qrc:/ui/icons/svg/plus.svg"
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

            delegate: PlaylistDelegate {
            }

            contentHeight: PlaylistModel.count * (delegate.height + spacing) + 10

            onPlayNext: {
                var source = Qt.url(PlaylistModel.getNext());
                if (source.toString() === "") {
                    return;
                }

                mediaPlayer.source = source
                Qt.callLater(() => {
                    mediaPlayer.play()
                });
            }

            onPlayPrevious: {
                var source = Qt.url(PlaylistModel.getPrevious());
                if (source.toString() === "") {
                    return;
                }

                mediaPlayer.source = source
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
