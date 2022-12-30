import QtQuick
//import QtQuick.Layouts
import QtQuick.Controls

ComboBox {
    id: myCombobox
//    Layout.preferredWidth: 400
//    Layout.preferredHeight: 20
    currentIndex: -1
//    model: ["First", "Second", "Third"]
    implicitContentWidthPolicy: ComboBox.WidestText

    property int delegateHorizontalPadding: 5
    delegate: ItemDelegate {
        width: myCombobox.implicitContentWidth + 2*delegateHorizontalPadding
        height: 20
        padding: 0
        horizontalPadding: 5
        Rectangle {
            width: parent.width
            height: parent.height
            color: highlighted ? "#3399ff" : "white"
        }
        contentItem: Text {
            text: modelData
            color: highlighted ? "white" : "black"
            padding: 0
        }
        highlighted: myCombobox.highlightedIndex === index
    }

    popup: Popup {
        padding: 1
        width: myCombobox.implicitContentWidth + 2*padding + 2*delegateHorizontalPadding
        height: contentHeight + 2*padding
        contentItem: ListView {
            clip: true
            implicitHeight: contentHeight
            model: visible ? myCombobox.delegateModel : null
            currentIndex: myCombobox.highlightedIndex
            ScrollIndicator.vertical: ScrollIndicator {}
        }
    }
}
