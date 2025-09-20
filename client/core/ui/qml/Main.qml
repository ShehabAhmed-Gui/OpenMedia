import QtQuick
import QtQuick.Window
import QtQuick.Controls.Fusion
import QtMultimedia

import com.qt.openmedia 1.0

import "Controls"
import "Components"

ApplicationWindow {
    id: root
    readonly property bool isMobileTarget: Qt.platform.os === "android" || Qt.platform.os === "ios"
    readonly property string os: Qt.platform.os
    readonly property bool soundMuted: SettingsController.getSetting("Audio", "muted")

    property alias videoState: mediaPlayer.playbackState

    flags: Qt.Window

    width: 1500
    height: 800
    minimumHeight: 460
    minimumWidth: 640
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

    Connections {
        id: titleChanger
        target: mediaPlayer

        // FIXME: remove file extension from name
        function onPlaybackStateChanged () {
            if (mediaPlayer.playbackState === MediaPlayer.PlayingState) {
                var fileUrl = mediaPlayer.source.toString();
                var fileName = fileUrl.split("/").pop();  // Extract filename from path
                root.title = "OpenMedia -  " + decodeURIComponent(fileName);
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
        interval: 5000
        repeat: true
        onTriggered: {
            if(bottomControls.isMediaSliderPressed) {
                afkTimer.restart()
            } else {
                changeMouseCursor(false)
                hideControls.start()
            }
        }
    }

    Item {
        id: mediaPlayerContainer
        anchors.fill: parent
        width: 500
        height: 500

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
                afkTimer.start()

                mediaPlayer.audioOutput.volume = SettingsController.getSetting("Audio", "volume") / 100

                mediaPlayer.source = SettingsController.getSetting("Video", "video")

                // Set loaded mediaPlayer to last-saved frame instead of black screen
                mediaPlayer.play()
                mediaPlayer.pause()
            }

            onSourceChanged: SettingsController.saveSetting("Video", "video", mediaPlayer.source.toString())
        }

        Keys.onSpacePressed: videoState === MediaPlayer.PlayingState? mediaPlayer.pause() : mediaPlayer.play()
        Keys.onLeftPressed: bottomControls.seekBackward()
        Keys.onRightPressed: bottomControls.seekForward()

        MediaDevices {
            id: mediaDevices
        }

        AudioOutput {
            id: audioOutput
            // Use default output device so it follows the device in case changed
            device: mediaDevices.defaultAudioOutput
            volume: 0.5
        }

        VideoOutput {
            id: videoOutput
            anchors.fill: parent
            anchors.margins: 3

            Keys.onPressed: (key) => {
                if (key.key === Qt.Key_K) {
                     if(videoState === MediaPlayer.PlayingState) {
                        mediaPlayer.pause()
                     } else {
                        mediaPlayer.play()
                    }
                }
            }

            fillMode: VideoOutput.Stretch
            focus: true
        }

        Timer {
            id: videoFocusTimer
            interval: 300
            repeat: true
            onTriggered: videoOutput.focus = true
        }

        TapHandler {
            id: mediaPlayerMouseArea

            onDoubleTapped: root.visibility === Window.FullScreen
                            ? showNormal() : showFullScreen()
            onTapped: videoState === MediaPlayer.PlayingState
                      ? mediaPlayer.pause() : mediaPlayer.play()
        }

        HoverHandler {
            onHoveredChanged: hovered? hoverTimer.start() : hoverTimer.stop()

            onPointChanged: {
                changeMouseCursor(true);
                afkTimer.restart()
                showControls.start()
            }
        }
    }

    BottomControls {
        id: bottomControls
        anchors.bottom: mediaPlayerContainer.bottom

        width: parent.width
        color: "transparent"
    }

    SubtitlePopup {
        id: subtitlePopup
        visible: false
        anchors.bottom: bottomControls.top
        anchors.right: parent.right
    }

    PlayList {
        id: playlist
        width: 250
        height: parent.height - bottomControls.height - 20

        anchors.right: parent.right
        anchors.top: parent.top
        anchors.margins: 10

        NumberAnimation {
            id: changePlaylistHeight
            target: playlist
            property: "height"
            to: bottomControls.opacity === 1? root.height - bottomControls.height - 20 : root.height - 20
            duration: 150
            easing.type: Easing.InCurve
        }
    }

    ParallelAnimation {
        id: hideControls
        onStopped: changePlaylistHeight.start()

        NumberAnimation {
            targets: bottomControls.bottomOpacity
            property: "opacity"
            to: 0
            duration: 1000
            easing.type: Easing.InOutQuad
        }
    }

    ParallelAnimation {
        id: showControls
        onStopped: changePlaylistHeight.start()

        NumberAnimation {
            targets: bottomControls
            property: "opacity"
            to: 1
            duration: 500
            easing.type: Easing.InOutQuad
        }
        NumberAnimation {
            target: bottomControls.bottomOpacity
            property: "opacity"
            to: 0.40
            duration: 500
            easing.type: Easing.InCurve
        }
    }

    ParallelAnimation {
        id: showPlayList

        NumberAnimation {
            target: playlist
            property: "width"
            to: 250
            duration: 250
            easing.type: Easing.InCurve
        }
    }

    ParallelAnimation {
        id: hidePlaylist

        NumberAnimation {
            target: playlist
            property: "width"
            to: 0
            duration: 250
            easing.type: Easing.InCurve
        }
    }

    // FIXME: change function name
    function changeMouseCursor(state) {
        switch (state) {
            case true:
                mediaPlayerMouseArea.cursorShape = Qt.ArrowCursor;
                bottomControls.bottomMA.cursorShape = Qt.ArrowCursor;
                break;
            case false:
                mediaPlayerMouseArea.cursorShape = Qt.BlankCursor;
                bottomControls.bottomMA.cursorShape = Qt.BlankCursor;
                break;
            default:
            break;
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
