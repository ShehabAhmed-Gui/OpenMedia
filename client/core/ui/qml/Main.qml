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
    property bool soundMuted: SettingsController.isMuted()

    property var videoState: MediaPlayerController.playbackState

    flags: Qt.Window

    width: 1500
    height: 800
    minimumHeight: 500
    minimumWidth: 850
    visible: true
    title: "OpenMedia"
    color: "#000000"
    Component.onCompleted: {
        console.debug("Application Started")
        // Load settings
        bottomControls.audioType.muted = soundMuted
    }

    onClosing: {
        console.log("Saving app settings")

        subtitleProxyModel.sourceModel = null
        audioProxyModel.sourceModel = null

        // Save Audio settings
        SettingsController.saveSetting("Audio", "volume", MediaPlayerController.volume * 100);
        SettingsController.saveSetting("Audio", "muted", MediaPlayerController.muted);

        // Save Video settings
        // TODO: implement position
        //SettingsController.saveSetting("Video", "position", mediaPlayer.position / 1000)
        SettingsController.saveSetting("Video", "video", MediaPlayerController.source())
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
        id: mediaPlayerConnections
        target: MediaPlayerController

        function onPlaybackStateChanged() {
            videoState = MediaPlayerController.playbackState()
            if (videoState === Playback.Playing) {
                var fileUrl = MediaPlayerController.source();
                var fileName = fileUrl.split("/").pop();  // Extract filename from path

                var parts = fileName.split(".");
                if (parts.length > 1)
                    parts.pop();

                var baseName = parts.join(".");
                mainWindow.title = "OpenMedia -  " + baseName;
            }
        }

        function onVideoFrameReady(frame) {
            video.updateFrame(frame);
        }
    }

    Connections {
        id: loopConnections
        target: VideoController

        // TODO: implement loop
        // function onLoopStateChanged() {
        //     switch (VideoController.loopState) {
        //     case 1: mediaPlayer.loops = MediaPlayer.Infinite
        //         break;
        //     case 2: mediaPlayer.loops = 1;
        //         break;
        //     }
        // }
    }

    Connections {
        target: VideoController

        function onPlayMediaFile(path) {
            hidePlaylist.start()
            MediaPlayerController.stop();
            Qt.callLater(() => {
                MediaPlayerController.source = Qt.url(path);
                MediaPlayerController.play();
            });
        }
    }

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
                onTapped: videoState === Playback.Playing
                          ? MediaPlayerController.stop() : MediaPlayerController.play()
            }

            function onHoveredChanged(hovered) {
                if (hovered) {
                    hoverTimer.start()
                }

                hoverTimer.stop()
            }
        }

        VideoItem {
            id: video
            anchors.fill: parent
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
