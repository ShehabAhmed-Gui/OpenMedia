import QtQuick
import QtQuick.Window
import QtQuick.Controls.Fusion
import QtMultimedia

import com.qt.openmedia 1.0

import "controls"
import "components"

ApplicationWindow {
    id: mainWindow
    readonly property bool isMobileTarget: Qt.platform.os === "android" || Qt.platform.os === "ios"
    readonly property string os: Qt.platform.os
    readonly property bool soundMuted: SettingsController.getSetting("Audio", "muted")

    property alias videoState: mediaPlayer.playbackState

    flags: Qt.Window

    width: 1500
    height: 800
    minimumHeight: 460
    minimumWidth: 700
    visible: true
    title: "OpenMedia"
    color: "#000000"
    Component.onCompleted: {
        console.debug("Application Started")

        // Load settings
        bottomControls.audioType.setMuted(soundMuted)
    }

    onClosing: {
        console.log("Saving app settings")

        subtitleProxyModel.sourceModel = null
        audioProxyModel.sourceModel = null

        // Save audio settings
        SettingsController.saveSetting("Audio", "volume", (audioOutput.volume * 100).toFixed());
        SettingsController.saveSetting("Audio", "muted", audioOutput.muted);

        // Save mediaPlayer settings
        SettingsController.saveSetting("Video", "position", mediaPlayer.position / 1000)
        SettingsController.saveSetting("Video", "video", mediaPlayer.source)
    }

    function setMouseCursorVisible(state) {
        switch (state) {
            case true:
                videoMouseArea.cursorShape = Qt.ArrowCursor;
                bottomControls.bottomMA.cursorShape = Qt.ArrowCursor;
                break;
            case false:
                videoMouseArea.cursorShape = Qt.BlankCursor;
                bottomControls.bottomMA.cursorShape = Qt.BlankCursor;
                break;
            default:
            break;
        }
    }

    Connections {
        id: titleChanger
        target: mediaPlayer

        function onPlaybackStateChanged () {
            if (mediaPlayer.playbackState === MediaPlayer.PlayingState) {
                var fileUrl = mediaPlayer.source.toString();
                var fileName = fileUrl.split("/").pop();  // Extract filename from path

                var parts = fileName.split(".");
                if (parts.length > 1)
                    parts.pop();

                var baseName = parts.join(".");
                mainWindow.title = "OpenMedia -  " + baseName;
            }
        }
    }

    // FIXME: use CoreController here
    // Connections {
    //     id: playVideoFromArg
    //     target: AppManager

    //     function onVideoPassedAsArg (arg) {
    //         hidePlaylist.start()
    //         mediaPlayer.stop();
    //         Qt.callLater(() => {
    //             mediaPlayer.source = Qt.url(arg);
    //             mediaPlayer.play();
    //             mediaPlayer.position = 0;
    //         });
    //     }
    // }

    Timer {
        id: hoverTimer
        interval: 3000
        onTriggered: showControls.start()
    }

    Timer {
        id: afkTimer
        interval: 4000
        repeat: true
        onTriggered: {
            if(bottomControls.isMediaSliderPressed || bottomControls.opacity === 0) {
                afkTimer.restart()
            } else {
                setMouseCursorVisible(false)
                hideControls.start()
            }
        }
    }

    Item {
        id: mediaPlayerContainer
        anchors.fill: parent
        width: 500
        height: 500

        Component.onCompleted: afkTimer.start()

        MouseArea {
            id: videoMouseArea
            anchors.fill: parent
            hoverEnabled: true
            propagateComposedEvents: true

            onPositionChanged: {
               setMouseCursorVisible(true);
               afkTimer.restart()
               if (bottomControls.opacity === 0)
                   showControls.start()
            }

            TapHandler {
                onDoubleTapped: mainWindow.visibility === Window.FullScreen
                                ? showNormal() : showFullScreen()
                onTapped: videoState === MediaPlayer.PlayingState
                          ? mediaPlayer.pause() : mediaPlayer.play()
            }

            function onHoveredChanged(hovered) {
                if (hovered) {
                    hoverTimer.start()
                }

                hoverTimer.stop()
            }
        }

        MediaPlayer {
            id: mediaPlayer

            videoOutput: videoOutput
            audioOutput: audioOutput

            source: Qt.url(SettingsController.getSetting("Video", "video"))

            onSeekableChanged: {
                if (mediaPlayer.seekable) {
                    // Fixes a bug on windows
                    Qt.callLater(() => {
                        mediaPlayer.position = SettingsController.getSetting("Video", "position") * 1000;
                    });
                }
            }

            onTracksChanged: {
                console.log("Available subtitle tracks:", mediaPlayer.subtitleTracks.length)
                console.log("Available Audio tracks:", mediaPlayer.audioTracks.length)

                MetaDataModel.setSubtitles(mediaPlayer.subtitleTracks)
                MetaDataModel.setAudioTracks(mediaPlayer.audioTracks)
            }

            Component.onCompleted: {
                audioOutput.muted = soundMuted

                mediaPlayer.audioOutput.volume = SettingsController.getSetting("Audio", "volume") / 100
                mediaPlayer.source = SettingsController.getSetting("Video", "video")

                // Set loaded mediaPlayer to last-saved frame instead of black screen
                mediaPlayer.play()
                mediaPlayer.pause()
            }

            onSourceChanged: SettingsController.saveSetting("Video", "video", mediaPlayer.source.toString())
        }

        Keys.onSpacePressed: {
            if (!bottomControls.opacity) {
                showControls.start()
                setMouseCursorVisible(true)
            }

            afkTimer.restart()
            videoState === MediaPlayer.PlayingState? mediaPlayer.pause() : mediaPlayer.play()
        }

        Keys.onLeftPressed: {
            if (!bottomControls.opacity) {
                showControls.start()
                setMouseCursorVisible(true)
            }

            afkTimer.restart()
            bottomControls.seekBackward()
        }

        Keys.onRightPressed: {
            if (!bottomControls.opacity) {
                showControls.start()
                setMouseCursorVisible(true)
            }

            afkTimer.restart()
            bottomControls.seekForward()
        }

        Keys.onPressed: (key) => {
            if (key.key === Qt.Key_K) {
                if (!bottomControls.opacity) {
                    showControls.start()
                    setMouseCursorVisible(true)
                }
                afkTimer.restart()

                if(videoState === MediaPlayer.PlayingState) {
                    mediaPlayer.pause()
                } else {
                    mediaPlayer.play()
                }
            }
        }


        MediaDevices {
            id: mediaDevices
        }

        AudioOutput {
            id: audioOutput
            // Use default output device so it follows the new device in case changed
            device: mediaDevices.defaultAudioOutput
            volume: 0.5
        }

        VideoOutput {
            id: videoOutput
            anchors.fill: parent
            anchors.margins: 3

            fillMode: VideoOutput.Stretch
            focus: true
        }

        Timer {
            id: videoFocusTimer
            interval: 300
            repeat: true
            onTriggered: videoOutput.focus = true
        }
    }

    BottomControls {
        id: bottomControls
        anchors.bottom: mediaPlayerContainer.bottom

        width: parent.width
        color: "transparent"
    }

    SubtitlePopup {
        id: metaDataPopup
        visible: false
        anchors.bottom: bottomControls.top
        anchors.right: parent.right
    }

    PlayList {
        id: playlist
        width: 250
        height: parent.height - bottomControls.height - 20
        visible: false

        anchors.right: parent.right
        anchors.top: parent.top
        anchors.margins: 10
    }

    ParallelAnimation {
        id: hideControls

        PropertyAnimation {
            targets: bottomControls
            property: "opacity"
            from: 1
            to: 0
            duration: 500
            easing.type: Easing.InOutQuad
        }
        PropertyAnimation {
            target: bottomControls.bottomOpacityRect
            property: "opacity"
            from: 0.40
            to: 0
            duration: 500
            easing.type: Easing.InOutQuad
        }
        PropertyAnimation {
            target: playlist
            property: "opacity"
            from: 1
            to: 0
            duration: 500
            easing.type: Easing.InOutQuad
        }
    }

    ParallelAnimation {
        id: showControls

        PropertyAnimation {
            targets: bottomControls
            property: "opacity"
            from: 0
            to: 1
            duration: 500
            easing.type: Easing.InOutQuad
        }
        PropertyAnimation {
            target: bottomControls.bottomOpacityRect
            property: "opacity"
            from: 0
            to: 0.40
            duration: 500
            easing.type: Easing.InOutQuad
        }
        PropertyAnimation {
            target: playlist
            property: "opacity"
            from: 0
            to: 1
            duration: 500
            easing.type: Easing.InOutQuad
        }
    }

    ParallelAnimation {
        id: showPlayList

        NumberAnimation {
            target: playlist
            property: "visible"
            from: 0
            to: 1
            duration: 250
            easing.type: Easing.InOutQuad
        }
    }

    ParallelAnimation {
        id: hidePlaylist

        NumberAnimation {
            target: playlist
            property: "visible"
            from: 1
            to: 0
            duration: 250
            easing.type: Easing.InOutQuad
        }
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
