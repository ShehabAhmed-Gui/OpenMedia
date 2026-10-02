import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import com.qt.openmedia 1.0

import "../components"
import "../theme"

Rectangle {
    id: root
    width: parent.width
    height: 132
    color: "transparent"
    anchors.bottom: parent.bottom

    property alias audioType: audioControl
    property alias playBackSpeedType: playBackSpeed
    property alias bottomMA: bottomControlsMouseArea

    property string videoSource: MediaPlayerController.source
    property bool isMediaSliderPressed: videoSlider.pressed
                                        || audioControl.volumeSlider.pressed
                                        || playBackSpeed.playbackSlider.pressed

    property bool userChangingSlider: false

    function stepBackward() {
        MediaPlayerController.seekBy(-10000)
    }

    function stepForward() {
        MediaPlayerController.seekBy(10000)
    }

    function formatTime(milliseconds) {
        var total = Math.max(0, Math.floor(milliseconds / 1000));
        var seconds = total % 60;
        var minutes = Math.floor((total % 3600) / 60);
        var hours = Math.floor(total / 3600);

        if (hours > 0)
            return hours + ":" + String(minutes).padStart(2, "0") + ":" + String(seconds).padStart(2, "0");

        return minutes + ":" + String(seconds).padStart(2, "0");
    }

    property Timer userInteractionTimer: Timer {
        interval: 300
        repeat: false
        onTriggered: userChangingSlider = false
    }

    property Timer extractedThumbnailsTimer: Timer {
        interval: 2000
        repeat: false
        onTriggered: extractingStatus.visible = false
    }

    Connections {
        target: MediaPlayerController

        function onMutedChanged() {
            audioControl.muted = MediaPlayerController.muted
        }
    }

    Connections {
        target: VideoController

        function onExtractingInProgress() {
            extractingStatus.visible = true
            extractingStatus.text = "Extracting video thumbnails..."
        }

        function onExtractedVideoThumbnails() {
            extractingStatus.text = "Extracted video thumbnails"
            extractedThumbnailsTimer.start()
        }
    }

    // Sits behind the controls and absorbs anything they do not take, so a
    // click on the bar never reaches the video underneath.
    MouseArea {
        id: bottomControlsMouseArea
        anchors.fill: parent
        preventStealing: true
    }

    // Reads over bright and dark footage alike, unlike a flat plate.
    Rectangle {
        anchors.fill: parent
        gradient: Gradient {
            GradientStop { position: 0.0; color: Qt.rgba(0.027, 0.035, 0.047, 0.0) }
            GradientStop { position: 0.55; color: Qt.rgba(0.027, 0.035, 0.047, 0.80) }
            GradientStop { position: 1.0; color: Qt.rgba(0.027, 0.035, 0.047, 0.94) }
        }
    }

    Image {
        id: framePreview
        asynchronous: true
        cache: false // same URL means a different picture after the file changes
        retainWhileLoading: true
        width: 175
        height: 125

        // Mouse x on the bar, and the time under it.
        readonly property real hoverX: Math.max(0, Math.min(videoSlider.availableWidth,
                                                            previewHover.point.position.x))
        readonly property real hoverMs: hoverX / videoSlider.availableWidth * videoSlider.to

        // Changing this URL is what triggers FrameProvider::requestImage().
        source: previewHover.hovered
                ? "image://framesprovider/" + Math.floor(hoverMs / 1000) * 1000
                : ""

        // Only shown once the provider has returned a real image.
        visible: previewHover.hovered && status === Image.Ready

        // Centred on the cursor, kept inside the bar.
        x: Math.max(0, Math.min(root.width - width,
                                videoSlider.mapToItem(root, hoverX, 0).x - width / 2))
        anchors.bottom: previewTime.top
        anchors.bottomMargin: Theme.xs
    }

    Text {
        id: previewTime
        // Only shown once the provider has returned a real image.
        visible: previewHover.hovered && framePreview.status === Image.Ready
        anchors.bottom: seekRow.top
        anchors.horizontalCenter: framePreview.horizontalCenter

        text: formatTime(framePreview.hoverMs)

        font.family: Theme.fontFamily
        font.pixelSize: Theme.captionSize
        font.weight: Font.Medium
        color: Theme.text
    }

    RowLayout {
        id: seekRow
        anchors.top: parent.top
        anchors.topMargin: Theme.xl
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.leftMargin: Theme.xl
        anchors.rightMargin: Theme.xl
        spacing: Theme.md

        Text {
            text: formatTime(videoSlider.value)
            color: Theme.textMuted
            font.family: Theme.fontFamily
            font.pixelSize: Theme.captionSize
            font.weight: Font.Medium
            font.features: { "tnum": 1 }
        }

        CustomSliderType {
            id: videoSlider

            Layout.fillWidth: true

            from: 0
            to: Math.max(1, MediaPlayerController.duration)
            stepSize: 0

            onMoved: {
                userChangingSlider = true
                userInteractionTimer.restart()
                MediaPlayerController.seek(videoSlider.value)
            }

            HoverHandler { id: previewHover }

            Connections {
                target: MediaPlayerController
                function onPositionChanged() {
                    if (!videoSlider.pressed && !userChangingSlider)
                        videoSlider.value = MediaPlayerController.position
                }
            }
        }

        Text {
            text: formatTime(MediaPlayerController.duration)
            color: Theme.textMuted
            font.family: Theme.fontFamily
            font.pixelSize: Theme.captionSize
            font.weight: Font.Medium
            font.features: { "tnum": 1 }
        }
    }

    // Left cluster
    RowLayout {
        anchors.left: parent.left
        anchors.leftMargin: Theme.xl
        anchors.verticalCenter: transportRow.verticalCenter
        spacing: Theme.lg

        PlaybackSpeedControls {
            id: playBackSpeed
            showPlaybackSpeedIcon: Screen.primaryOrientation === Qt.LandscapeOrientation

            Layout.minimumWidth: isMobileTarget ? 120 : 150
            Layout.maximumWidth: isMobileTarget ? 120 : 150
            Layout.preferredHeight: Theme.hitSize
        }

        Text {
            id: extractingStatus
            visible: false
            color: Theme.textMuted
            font.family: Theme.fontFamily
            font.pixelSize: Theme.captionSize
            font.weight: Font.Medium
        }
    }

    // Centre transport
    RowLayout {
        id: transportRow
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.top: seekRow.bottom
        anchors.topMargin: Theme.md
        spacing: Theme.sm

        CustomButton {
            iconSource: "qrc:/ui/icons/svg/previous.svg"
            iconWidth: 15
            iconHeight: 15

            ToolTipType { toolTipText: "Previous" }

            onClicked: playlist.listView.playPrevious()
        }

        CustomButton {
            iconSource: "qrc:/ui/icons/svg/backward_10s.svg"
            iconWidth: 17
            iconHeight: 17

            ToolTipType { toolTipText: "Back 10 seconds" }

            onClicked: root.stepBackward()
        }

        CustomButton {
            id: startStopButton
            iconSource: videoState === Playback.Playing
                        ? "qrc:/ui/icons/svg/pause.svg"
                        : "qrc:/ui/icons/svg/play.svg"
            iconWidth: 16
            iconHeight: 16

            buttonWidth: 42
            buttonHeight: 42
            buttonRadius: 21

            // The one filled control: the primary action reads first.
            backgroundColor: Theme.text
            onHoverBackgroundColor: Qt.lighter(Theme.text, 1.08)
            iconColor: Theme.base
            iconHoverColor: Theme.base

            ToolTipType { toolTipText: videoState === Playback.Playing ? "Pause" : "Play" }

            onClicked: MediaPlayerController.pause_resume()
        }

        CustomButton {
            iconSource: "qrc:/ui/icons/svg/forward_10s.svg"
            iconWidth: 17
            iconHeight: 17

            ToolTipType { toolTipText: "Forward 10 seconds" }

            onClicked: root.stepForward()
        }

        CustomButton {
            iconSource: "qrc:/ui/icons/svg/next.svg"
            iconWidth: 15
            iconHeight: 15

            ToolTipType { toolTipText: "Next" }

            onClicked: playlist.listView.playNext()
        }

        CustomButton {
            id: loopBtn
            iconSource: "qrc:/ui/icons/svg/loop.svg"
            iconWidth: 14
            iconHeight: 14

            // State is colour, not a different file.
            iconColor: VideoController.loop ? Theme.accent : Theme.textMuted
            iconHoverColor: VideoController.loop ? Theme.accent : Theme.text

            ToolTipType { toolTipText: VideoController.loop ? "Repeat on" : "Repeat off" }

            onClicked: VideoController.loop = !VideoController.loop
        }
    }

    // Right cluster
    RowLayout {
        anchors.right: parent.right
        anchors.rightMargin: Theme.xl
        anchors.verticalCenter: transportRow.verticalCenter
        spacing: Theme.md

        AudioControls {
            id: audioControl
            Layout.minimumWidth: isMobileTarget ? 130 : 165
            Layout.maximumWidth: isMobileTarget ? 130 : 165
            Layout.preferredHeight: Theme.hitSize
        }

        CustomButton {
            iconSource: "qrc:/ui/icons/svg/subtitles.svg"
            iconWidth: 16
            iconHeight: 16
            iconColor: metaDataPopup.visible ? Theme.accent : Theme.textMuted

            ToolTipType { toolTipText: metaDataPopup.visible ? "Hide subtitles" : "Show subtitles" }

            onClicked: metaDataPopup.visible = !metaDataPopup.visible
        }

        CustomButton {
            iconSource: "qrc:/ui/icons/svg/playlist.svg"
            iconWidth: 16
            iconHeight: 16
            iconColor: mainWindow.playlistOpen ? Theme.accent : Theme.textMuted

            ToolTipType { toolTipText: mainWindow.playlistOpen ? "Hide playlist" : "Show playlist" }

            onClicked: mainWindow.playlistOpen = !mainWindow.playlistOpen
        }
    }
}
