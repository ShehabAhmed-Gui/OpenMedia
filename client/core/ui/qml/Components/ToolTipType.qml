import QtQuick
import QtQuick.Controls
import QtQuick.Controls.Fusion

ToolTip {
    id: root

    visible: parent.hovered? true : false

    property string toolTipText: ""
    property string toolTipBckColor: "#000000"

    background: Rectangle {
        id: backgroud
        anchors.fill: parent

        radius: 5

        color: toolTipBckColor
    }

    contentItem: Text {
        anchors.centerIn: backgroud

        color: "#ffffff"
        text: qsTr(toolTipText)

        font.pixelSize: 11
        font.kerning: true
        font.weight: Font.Medium
    }

}
