import QtQuick
import QtQuick.Controls
import QtQuick.Controls.Fusion

Slider {
    id: customSlider
    live: true

    property bool enableGradiant: true
    property bool enableHandler: false
    property double gradiantFirstPos: 0.40448
    property double gradiantLastStop: 1

    property string gradiantFirstColor: "#6E8BB3"
    property string gradiantSecondColor: "#4C5D8B"
    property string handleColor: "#A6A9C8"
    property string subPageColor: "#333333"

    property int sliderWidth: 150
    property int sliderHeight: 7

    focus: false
    stepSize: 1

    background: Rectangle {
        id: backgroundRect
        x: customSlider.leftPadding
        y: customSlider.topPadding + customSlider.availableHeight / 2 - height / 2
        implicitWidth: sliderWidth
        implicitHeight: sliderHeight

        width: customSlider.availableWidth
        height: implicitHeight
        radius: 2

        gradient: Gradient {
            orientation: Gradient.Horizontal
            GradientStop { position: 0.40448; color: subPageColor }
        }

        Rectangle {
            width: customSlider.visualPosition * parent.width
            height: parent.height

            gradient: Gradient {
                orientation: Gradient.Horizontal
                GradientStop { position: gradiantFirstPos; color: gradiantFirstColor  }
                GradientStop { position: gradiantLastStop; color: gradiantSecondColor }
            }
            radius: 2
        }

        MouseArea {
            id: sliderMA
            anchors.fill: parent
            hoverEnabled: true

            onEntered: {
                sliderMA.cursorShape = Qt.ClosedHandCursor
                enableHandler = true
            }

            onExited: {
                sliderMA.cursorShape = Qt.ArrowCursor
                enableHandler = false
            }
        }
    }

    handle: Rectangle {
        visible: enableHandler? true : false
        x: customSlider.leftPadding + customSlider.visualPosition * (customSlider.availableWidth - width)
        y: backgroundRect.y + backgroundRect.height / 2 - height / 2
        implicitWidth: 12
        implicitHeight: 12
        radius: 15
        color: handleColor
    }
}
