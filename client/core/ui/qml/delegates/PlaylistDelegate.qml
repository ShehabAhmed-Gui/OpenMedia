import QtQuick

import com.qt.openmedia 1.0

import "../components"
import "../theme"

Rectangle {
    id: root

    width: ListView.view ? ListView.view.width : 0
    height: 52
    radius: Theme.radiusControl

    property bool isCurrentlyPlaying: MediaPlayerController.source === path
    property bool isMusicFile: path.endsWith(".mp3")
    property bool isPlayingNow: isCurrentlyPlaying && videoState === Playback.Playing

    color: hoverHandler.hovered ? Theme.raised
                                : isCurrentlyPlaying ? Qt.rgba(1, 1, 1, 0.04)
                                                     : "transparent"

    Behavior on color {
        ColorAnimation { duration: Theme.durFast }
    }

    HoverHandler {
        id: hoverHandler
        onHoveredChanged: hovered ? afkTimer.stop() : afkTimer.start()
    }

    TapHandler {
        // Exclusive grab: a passive one lets the click through to the video.
        gesturePolicy: TapHandler.ReleaseWithinBounds

        onTapped: {
            PlaylistModel.currentIndex = index

            if (isCurrentlyPlaying && MediaPlayerController.playbackState !== Playback.Stopped)
                MediaPlayerController.pause_resume()
            else
                MediaPlayerController.start(path)
        }
    }

    // Leading accent bar marks the active row.
    Rectangle {
        anchors.left: parent.left
        anchors.verticalCenter: parent.verticalCenter
        width: 3
        height: isCurrentlyPlaying ? parent.height - Theme.md : 0
        radius: 2
        color: Theme.accent

        Behavior on height {
            NumberAnimation { duration: Theme.durBase; easing.type: Theme.easeOut }
        }
    }

    Item {
        id: leading
        anchors.left: parent.left
        anchors.leftMargin: Theme.md
        anchors.verticalCenter: parent.verticalCenter
        width: 20
        height: 20

        // Three bars bouncing: the only place the shell shows it is running.
        Row {
            anchors.centerIn: parent
            spacing: 2.5
            visible: isPlayingNow

            Repeater {
                model: 3

                Rectangle {
                    width: 2.5
                    radius: 1.25
                    color: Theme.accent
                    anchors.verticalCenter: parent.verticalCenter
                    height: 5

                    SequentialAnimation on height {
                        running: isPlayingNow
                        loops: Animation.Infinite

                        PauseAnimation { duration: index * 130 }
                        NumberAnimation { to: 15; duration: 380; easing.type: Easing.InOutSine }
                        NumberAnimation { to: 5;  duration: 380; easing.type: Easing.InOutSine }
                    }
                }
            }
        }

        AppIcon {
            anchors.centerIn: parent
            visible: !isPlayingNow
            size: 16
            source: isMusicFile ? "qrc:/ui/icons/svg/music_media.svg"
                                : "qrc:/ui/icons/svg/play.svg"
            color: hoverHandler.hovered ? Theme.text : Theme.textFaint
        }
    }

    Item {
        id: nameClip
        anchors.left: leading.right
        anchors.leftMargin: Theme.md
        anchors.right: deleteItem.left
        anchors.rightMargin: Theme.sm
        anchors.verticalCenter: parent.verticalCenter
        height: label.height
        clip: true

        Text {
            id: label
            text: name
            color: isCurrentlyPlaying ? Theme.accent : Theme.text
            width: Math.max(implicitWidth, nameClip.width)

            font.family: Theme.fontFamily
            font.pixelSize: Theme.bodySize
            font.weight: isCurrentlyPlaying ? Font.DemiBold : Font.Medium
            elide: hoverHandler.hovered ? Text.ElideNone : Text.ElideRight
            wrapMode: Text.NoWrap

            // Only long names scroll, and only while pointed at.
            SequentialAnimation on x {
                running: hoverHandler.hovered && label.implicitWidth > nameClip.width
                loops: Animation.Infinite

                PauseAnimation { duration: 700 }
                NumberAnimation {
                    to: nameClip.width - label.implicitWidth
                    duration: Math.max(1200, (label.implicitWidth - nameClip.width) * 22)
                    easing.type: Easing.InOutQuad
                }
                PauseAnimation { duration: 900 }
                NumberAnimation { to: 0; duration: 320; easing.type: Theme.easeOut }
            }

            onXChanged: if (!hoverHandler.hovered) x = 0
        }
    }

    CustomButton {
        id: deleteItem
        anchors.verticalCenter: parent.verticalCenter
        anchors.right: parent.right
        anchors.rightMargin: Theme.sm

        iconSource: "qrc:/ui/icons/svg/trash.svg"
        iconWidth: 14
        iconHeight: 14
        buttonWidth: 28
        buttonHeight: 28
        iconHoverColor: Theme.danger

        opacity: hoverHandler.hovered ? 1 : 0
        enabled: hoverHandler.hovered

        Behavior on opacity {
            NumberAnimation { duration: Theme.durFast }
        }

        ToolTipType { toolTipText: qsTr("Remove") }

        onClicked: PlaylistModel.deleteItem(index)
    }
}
