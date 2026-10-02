import QtQuick
import QtQuick.Layouts
import QtQuick.Effects

import "../delegates"
import "../theme"

Item {
    id: root

    property alias listView: listView

    MultiEffect {
        source: panel
        anchors.fill: panel
        shadowEnabled: true
        shadowColor: Qt.rgba(0, 0, 0, 0.55)
        shadowBlur: 0.6
        shadowVerticalOffset: 6
    }

    Rectangle {
        id: panel
        anchors.fill: parent
        color: Qt.rgba(Theme.surface.r, Theme.surface.g, Theme.surface.b, 0.94)
        radius: Theme.radiusPanel
        border.color: Theme.hairline
        border.width: 1

        MouseArea {
            anchors.fill: parent
            preventStealing: true
            hoverEnabled: true
            onEntered: afkTimer.stop()
            onExited: afkTimer.start()
        }

        ColumnLayout {
            anchors.fill: parent
            anchors.margins: Theme.lg
            spacing: Theme.md

            RowLayout {
                Layout.fillWidth: true
                spacing: Theme.sm

                Text {
                    text: qsTr("Playlist")
                    color: Theme.text
                    font.family: Theme.fontFamily
                    font.pixelSize: Theme.titleSize
                    font.weight: Font.DemiBold
                }

                Text {
                    text: PlaylistModel.count
                    color: Theme.textFaint
                    font.family: Theme.fontFamily
                    font.pixelSize: Theme.captionSize
                    font.weight: Font.Medium
                    font.features: { "tnum": 1 }
                    Layout.alignment: Qt.AlignVCenter
                }

                Item { Layout.fillWidth: true }

                CustomButton {
                    id: addItems
                    iconSource: "qrc:/ui/icons/svg/plus.svg"
                    iconWidth: 15
                    iconHeight: 15
                    buttonWidth: 30
                    buttonHeight: 30

                    ToolTipType { toolTipText: qsTr("Add files") }

                    onClicked: PlaylistModel.loadVideos()
                }

                CustomButton {
                    id: clearPlaylist
                    iconSource: "qrc:/ui/icons/svg/trash.svg"
                    iconWidth: 15
                    iconHeight: 15
                    buttonWidth: 30
                    buttonHeight: 30
                    iconHoverColor: Theme.danger

                    ToolTipType { toolTipText: qsTr("Remove all") }

                    onClicked: PlaylistModel.clearPlaylist()
                }
            }

            Rectangle {
                Layout.fillWidth: true
                Layout.preferredHeight: 1
                color: Theme.hairline
            }

            ListView {
                id: listView

                Layout.fillHeight: true
                Layout.fillWidth: true

                signal playNext()
                signal playPrevious()

                model: PlaylistModel
                spacing: Theme.xs
                clip: true
                boundsBehavior: Flickable.StopAtBounds

                delegate: PlaylistDelegate {}

                add: Transition {
                    NumberAnimation { properties: "opacity"; from: 0; to: 1; duration: Theme.durBase }
                    NumberAnimation { properties: "x"; from: 24; duration: Theme.durBase; easing.type: Theme.easeOut }
                }
                remove: Transition {
                    NumberAnimation { properties: "opacity"; to: 0; duration: Theme.durFast }
                }
                displaced: Transition {
                    NumberAnimation { properties: "x,y"; duration: Theme.durBase; easing.type: Theme.easeOut }
                }

                onPlayNext: {
                    var source = PlaylistModel.getNext();
                    if (source === "")
                        return;

                    MediaPlayerController.start(source)
                }

                onPlayPrevious: {
                    var source = PlaylistModel.getPrevious();
                    if (source === "")
                        return;

                    MediaPlayerController.start(source)
                }

                ColumnLayout {
                    anchors.centerIn: parent
                    width: parent.width - Theme.xl
                    spacing: Theme.md
                    visible: PlaylistModel.count === 0

                    Image {
                        source: "qrc:/ui/icons/logo_mono.svg"
                        sourceSize: Qt.size(56, 56)
                        opacity: 0.14
                        Layout.alignment: Qt.AlignHCenter
                    }

                    Text {
                        text: qsTr("Nothing queued yet")
                        color: Theme.textFaint
                        font.family: Theme.fontFamily
                        font.pixelSize: Theme.labelSize
                        font.weight: Font.Medium
                        Layout.alignment: Qt.AlignHCenter
                    }
                }
            }
        }
    }
}
