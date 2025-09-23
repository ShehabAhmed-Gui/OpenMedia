import QtQuick
import QtQuick.Controls
import QtQuick.Controls.Fusion

ToolTip {
    id: root

    visible: parent.hovered? true : false

    property string toolTipText: ""
    property string toolTipBckColor: "black"

    background: Rectangle {
        id: backgroud
        anchors.fill: parent
        radius: 5
        color: toolTipBckColor
    }

    contentItem: Text {
        anchors.centerIn: backgroud

        color: "orange"
        text: qsTr(toolTipText)

        font.family: "Poppins"
        font.pixelSize: 12
        font.kerning: true
        font.weight: Font.Medium
    }
}
