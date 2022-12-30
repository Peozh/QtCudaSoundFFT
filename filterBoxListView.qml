import QtQuick
import QtQuick.Layouts
import QtQuick.Controls
import FilterBoxDataBasePackage 1.0
import MyOpenGLWidgetPackage 1.0

Item {
    ColumnLayout {
        id: columnLayout
        anchors.fill: parent

//        ListModel{
//            id: groupModel
////            ListElement{ name: "group1" }
////            ListElement{ name: "group2" }
////            ListElement{ name: "group3" }
////            ListElement{ name: "group4" }
////            ListElement{ name: "group5" }
//        }

        Button {
            id: buttonAddNewFilter
            Layout.fillWidth: true
            Layout.preferredHeight: 28
            text: "Add New Filter"
            onClicked: {
                _filterBoxDataBase.addFilter("new filter");
            }
        }

        ListView {
            id: listViewBoxes
            Layout.fillWidth: true
            Layout.fillHeight: true
            focus: true
            clip: true
            currentIndex: _filterBoxDataBase.activeFilterIdx

            model: _filterBoxDataBase.filterNamesModel

            onModelChanged: {
                console.log("listViewBoxes.currentIndex = ", listViewBoxes.currentIndex);
            }

            delegate: ItemDelegate {
                id: filterBoxDelegate
                Layout.fillWidth: true
                width: 200
                height: 45
                highlighted: ListView.isCurrentItem

                background: Rectangle {
                    anchors.fill: filterBoxDelegate
//                    color: filterBoxDelegate.highlighted ? "lightsteelblue" : "transparent"
                    color: "transparent"
                }

                RowLayout {
                    Layout.fillWidth: true
                    anchors.fill: parent
                    Layout.alignment: Qt.AlignVCenter

                    ColumnLayout {
                        Layout.fillWidth: true
                        Layout.fillHeight: true
                        Layout.leftMargin: 10
                        Layout.alignment: Qt.AlignVCenter
                        spacing: 0
                        Text {
                            Layout.fillWidth: true
                            clip: true
                            text: '<b>' + index + " " + modelData + '</b>'
                            color: filterBoxDelegate.highlighted ? "white" : "black"
                        }
                        Text {
                            Layout.fillWidth: true
                            clip: true
                            text: '   ' + _filterBoxDataBase.getFilterWeight(index, _filterBoxDataBase.activeFilterIdx)
                            color: filterBoxDelegate.highlighted ? "white" : "black"
                        }
                    }

                    Button {
                        id: buttonDeleteFilterBox
                        Layout.preferredWidth: 30
                        Layout.preferredHeight: 30
                        Layout.rightMargin: 10
                        Layout.alignment: Qt.AlignRight
                        Text {
                            anchors.centerIn: parent
                            text: "X"
                            color: filterBoxDelegate.highlighted ? "white" : "black"
                        }
                        flat: true
                        onClicked: {
                            _filterBoxDataBase.deleteFilter(index);
                        }
                    }
                }
                onClicked: {
                    _filterBoxDataBase.activeFilterIdx = index;
                }
            }
            highlight: Rectangle {
                Layout.fillWidth: true
                width: 200
                height: 45
                color: 'dodgerblue'
            }
            ScrollIndicator.vertical: ScrollIndicator {}
            highlightFollowsCurrentItem: true
            highlightMoveDuration: 0
        }
    }

}
