import QtQuick
import QtMultimedia
import "../Controls"

Video {
    id: video
    fillMode: VideoOutput.Stretch

    property int cursorWidth: 20
    property int cursorHeight: 20

    width: 500
    height: 500
    focus: true
}
