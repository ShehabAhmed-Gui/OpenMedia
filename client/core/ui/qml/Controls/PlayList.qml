import QtQuick
import QtQuick.Layouts
import com.qt.openmedia

import "../Components"

Rectangle {
    id: root

    color: "#000000"
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
                    toolTipText: "Remove all videos"
                }

                MouseArea {
                    anchors.fill: parent
                    cursorShape: parent.hovered? Qt.PointingHandCursor : Qt.ArrowCursor
                    onClicked: listModel.clearPlaylist()
                }
            }

            Text {
                id: playlistTitle
                text: "Videos Playlist"

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

                    onClicked:  listModel.loadVideos();
                }
            }
        }

        ListView {
            id: listView

            Layout.fillHeight: true
            Layout.fillWidth: true

            signal playNext()
            signal playPrevious()

            model: listModel

            spacing: 20
            clip: true

            delegate: VideosDelegate {
            }

            contentHeight: listModel.count * (delegate.height + spacing) + 10

            onPlayNext: {
                video.source = Qt.url(listModel.getNext())
                if (os === "linux") {
                    Qt.callLater(() => {
                        video.play()
                    });
                } else if (os === "windows") { video.play() }
            }

            onPlayPrevious: {
                video.source = Qt.url(listModel.getPrevious())
                if (os === "linux") {
                    Qt.callLater(() => {
                        video.play()
                    });
                } else if (os === "windows") { video.play() }
            }
        }

        MVideos {
            id: listModel
        }

        Rectangle {
        width: listView.width
        height: 5  // This is the space at the bottom of the list
        color: "transparent"  // Invisible space
    }
    }
}
