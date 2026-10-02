import QtQuick
import QtQuick.Window
import QtQuick.Controls

import com.qt.openmedia 1.0

import "controls"
import "components"
import "theme"

ApplicationWindow {
    id: mainWindow

    readonly property bool isMobileTarget: Qt.platform.os === "android" || Qt.platform.os === "ios"
    readonly property string os: Qt.platform.os

    property var videoState: MediaPlayerController.playbackState
    property bool controlsVisible: true
    property bool playlistOpen: false

    flags: Qt.Window

    width: 1500
    height: 800
    minimumHeight: 480
    minimumWidth: 640
    visible: true
    title: "OpenMedia"
    color: Theme.base

    Component.onCompleted: {
        MediaPlayerController.muted = SettingsController.isMuted()
    }

    onClosing: {
        subtitleProxyModel.sourceModel = null
        audioProxyModel.sourceModel = null

        SettingsController.saveSetting("Audio", "volume", MediaPlayerController.volume * 100);
        SettingsController.saveSetting("Audio", "muted", MediaPlayerController.muted);
        SettingsController.saveSetting("Video", "position", MediaPlayerController.position / 1000)
        SettingsController.saveSetting("Video", "video", MediaPlayerController.source)
    }

    function setMouseCursorVisible(state) {
        videoMouseArea.cursorShape = state ? Qt.ArrowCursor : Qt.BlankCursor
        bottomControls.bottomMA.cursorShape = state ? Qt.ArrowCursor : Qt.BlankCursor
    }

    function revealControls() {
        setMouseCursorVisible(true)
        controlsVisible = true
        afkTimer.restart()
    }

    Connections {
        target: MediaPlayerController

        function onPlaybackStateChanged() {
            videoState = MediaPlayerController.playbackState

            if (videoState !== Playback.Stopped) {
                var fileName = MediaPlayerController.source.split(/[\\/]/).pop();
                var parts = fileName.split(".");
                if (parts.length > 1)
                    parts.pop();

                mainWindow.title = "OpenMedia — " + parts.join(".")
            } else {
                mainWindow.title = "OpenMedia"
            }
        }

        function onVideoFrameReady(frame) {
            // The item takes ownership of the frame, so it must always be passed on.
            video.updateYUVFrame(frame);
        }

        function onAudioArtworkReady(frame) {
            video.updateArtworkFrame(frame);
        }

        function onEndOfMedia() {
            video.clear();
            playlist.listView.playNext();
        }

        function onSourceChanged() {
            // Nothing of the previous file stays on screen while this one opens.
            video.clear();
            // Keeps next/previous relative to what is actually playing.
            PlaylistModel.setCurrentPath(MediaPlayerController.source);
        }
    }

    Connections {
        target: VideoController

        function onPlayMediaFile(path) {
            playlistOpen = false
            MediaPlayerController.start(path);
        }
    }

    Timer {
        id: afkTimer
        interval: 2600
        repeat: false
        onTriggered: {
            if (bottomControls.isMediaSliderPressed || playlistOpen) {
                afkTimer.restart()
                return
            }

            setMouseCursorVisible(false)
            controlsVisible = false
        }
    }

    Item {
        id: mediaPlayerContainer
        anchors.fill: parent

        Component.onCompleted: afkTimer.start()

        VideoItem {
            id: video
            anchors.fill: parent
        }

        MouseArea {
            id: videoMouseArea
            anchors.fill: parent
            hoverEnabled: true

            onPositionChanged: revealControls()

            TapHandler {
                onDoubleTapped: mainWindow.visibility === Window.FullScreen
                                ? showNormal() : showFullScreen()
                onTapped: MediaPlayerController.pause_resume()
            }
        }
    }

    BottomControls {
        id: bottomControls
        anchors.bottom: parent.bottom
        width: parent.width
        color: "transparent"

        opacity: controlsVisible ? 1 : 0
        visible: opacity > 0.01

        Behavior on opacity {
            NumberAnimation { duration: Theme.durBase; easing.type: Theme.easeOut }
        }
    }

    SubtitlePopup {
        id: metaDataPopup
        visible: false
        anchors.bottom: bottomControls.top
        anchors.right: parent.right
        anchors.rightMargin: Theme.lg
    }

    PlayList {
        id: playlist
        width: 300
        anchors.top: parent.top
        anchors.bottom: bottomControls.top
        anchors.topMargin: Theme.lg
        anchors.bottomMargin: Theme.sm
        anchors.right: parent.right

        // Slides out of frame rather than toggling a bool through an animation.
        anchors.rightMargin: playlistOpen ? Theme.lg : -width
        opacity: playlistOpen ? 1 : 0
        visible: opacity > 0.01

        Behavior on anchors.rightMargin {
            NumberAnimation { duration: Theme.durSlow; easing.type: Theme.easeOut }
        }
        Behavior on opacity {
            NumberAnimation { duration: Theme.durSlow; easing.type: Theme.easeOut }
        }
    }

    Shortcut {
        sequences: ["Space", "K"]
        onActivated: { revealControls(); MediaPlayerController.pause_resume() }
    }
    Shortcut {
        sequence: "Right"
        onActivated: { revealControls(); MediaPlayerController.seekBy(5000) }
    }
    Shortcut {
        sequence: "Left"
        onActivated: { revealControls(); MediaPlayerController.seekBy(-5000) }
    }
    Shortcut {
        sequence: "Up"
        onActivated: { revealControls(); MediaPlayerController.volume = Math.min(MediaPlayerController.maxVolume, MediaPlayerController.volume + 0.05) }
    }
    Shortcut {
        sequence: "Down"
        onActivated: { revealControls(); MediaPlayerController.volume = Math.max(0, MediaPlayerController.volume - 0.05) }
    }
    Shortcut {
        sequence: "M"
        onActivated: { revealControls(); MediaPlayerController.muted = !MediaPlayerController.muted }
    }
    Shortcut {
        sequence: "L"
        onActivated: { revealControls(); VideoController.loop = !VideoController.loop }
    }
    Shortcut {
        sequence: "P"
        onActivated: { revealControls(); playlistOpen = !playlistOpen }
    }
    Shortcut {
        sequence: "F"
        onActivated: mainWindow.visibility === Window.FullScreen ? showNormal() : showFullScreen()
    }
    Shortcut {
        sequence: "Esc"
        onActivated: if (mainWindow.visibility === Window.FullScreen) showNormal()
    }

    MetaDataFilterProxyModel {
        id: audioProxyModel
        sourceModel: MetaDataModel
        filterMetaData: "audio"
    }

    MetaDataFilterProxyModel {
        id: subtitleProxyModel
        sourceModel: MetaDataModel
        filterMetaData: "subtitle"
    }
}
