pragma Singleton

import QtQuick

QtObject {
    // ---- surfaces ----
    readonly property color base: "#07090C"
    readonly property color surface: "#101419"
    readonly property color raised: "#191F26"
    readonly property color hairline: "#262E38"

    readonly property color hoverOverlay: Qt.rgba(1, 1, 1, 0.09)
    readonly property color pressOverlay: Qt.rgba(1, 1, 1, 0.16)

    // ---- text ----
    readonly property color text: "#F2F5F8"
    readonly property color textMuted: "#93A1B0"
    readonly property color textFaint: "#5C6773"

    // ---- accent ----
    readonly property color accent: "#2DD4BF"
    readonly property color accentDim: "#14B8A6"
    readonly property color danger: "#F87171"
    readonly property color boost: "#FBBF24"

    // ---- spacing ----
    readonly property int xs: 4
    readonly property int sm: 8
    readonly property int md: 12
    readonly property int lg: 16
    readonly property int xl: 24
    readonly property int xxl: 32

    // ---- shape ----
    readonly property int radiusControl: 8
    readonly property int radiusPanel: 14
    readonly property int radiusPill: 999
    readonly property int hitSize: 34

    // ---- type ----
    readonly property string fontFamily: "Poppins"
    readonly property int titleSize: 20
    readonly property int bodySize: 14
    readonly property int labelSize: 13
    readonly property int captionSize: 11

    // ---- motion ----
    readonly property int durFast: 120
    readonly property int durBase: 200
    readonly property int durSlow: 320
    readonly property int durEmphasis: 260

    readonly property int easeOut: Easing.OutCubic
    readonly property int easeIn: Easing.InCubic
    readonly property int easeInOut: Easing.InOutCubic
}
