import QtQuick
import QtQuick.Window
import QtQuick.Controls.Fusion
import QtMultimedia
import com.qt.openmedia

import "Controls"
import "Components"

ApplicationWindow {
    id: root
    readonly property bool isMobileTarget: Qt.platform.os === "android" || Qt.platform.os === "ios"
    readonly property bool soundMuted: AppSettings.getSetting("Audio", "muted")

    property alias videoState: video.playbackState

    flags: Qt.Window

    width: 1500
    height: 800
    minimumHeight: 460
    minimumWidth: 640
    visible: true
    title: qsTr("OpenMedia")
    color: "#000000"
    Component.onCompleted: {
        console.debug("Application Started")

        // load settings
        bottomControls.audioType.setMuted(soundMuted)
        video.volume = AppSettings.getSetting("Audio", "volume") / 100

        video.source = Qt.url(AppSettings.getSetting("Video", "video"))
        video.seek(AppSettings.getSetting("Video", "position") * 1000)
    }

    onClosing: {
        console.log("Writing Settings. Settings saved in:", AppSettings.getSettLocation())

        //save audio settings
        AppSettings.saveSettings("Audio", "volume", (video.volume * 100).toFixed());
        AppSettings.saveSettings("Audio", "muted", video.muted);

        //save video settings
        AppSettings.saveSettings("Video", "position", video.position / 1000)
        AppSettings.saveSettings("Video", "video", video.source.toString())
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
            if(bottomControls.isMediaSliderPressed) {
                afkTimer.restart()
            } else {
                changeMouseCursor(false)
                hideControls.start()
            }
        }
    }

    VideoPlayerType {
        id: video
        width: 500
        height: 500

        anchors.fill: parent
        source: AppSettings.getSetting("Video", "video")
        volume: 0.3

        cursorWidth: 40
        cursorHeight: 40

        Component.onCompleted: afkTimer.start()

        onSourceChanged: AppSettings.saveSettings("Video", "video", source.toString())

        MouseArea {
            id: videoMouseArea
            anchors.fill: parent
            hoverEnabled: true
            preventStealing: true

            onEntered: {
                hoverTimer.start()
            }

            onExited: {
                hoverTimer.stop();
            }

            onClicked: {
                videoState === MediaPlayer.PlayingState? video.pause() : video.play()
            }

            onPositionChanged: {
                if (!isMobileTarget) {
                    changeMouseCursor(true);
                    afkTimer.restart()
                    showControls.start()
                }
            }
        }

        Timer {
            id: videoFocusTimer
            interval: 300
            repeat: true
            onTriggered: video.focus = true
        }

        Keys.onSpacePressed: videoState === MediaPlayer.PlayingState? video.pause() : video.play()
        Keys.onLeftPressed: bottomControls.seekBackward()
        Keys.onRightPressed: bottomControls.seekForward()

        Keys.onPressed: (key) => {
            if (key.key === Qt.Key_K) {
                 if(videoState === MediaPlayer.PlayingState) {
                    video.pause()
                 } else {
                    video.play()
                }
            }
        }
    }

    BottomControls {
        id: bottomControls
        anchors.bottom: video.bottom

        width: parent.width
        color: "transparent"
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
            targets: bottomControls
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
            easing.type: Easing.InOutQuad
        }
    }

    ParallelAnimation {
        id: showPlayList

        NumberAnimation {
            target: playlist
            property: "width"
            to: 300
            duration: 300
            easing.type: Easing.InCurve
        }
    }

    ParallelAnimation {
        id: hidePlaylist

        NumberAnimation {
            target: playlist
            property: "width"
            to: 0
            duration: 300
            easing.type: Easing.InCurve
        }
    }

    function changeMouseCursor(state) {
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

}
