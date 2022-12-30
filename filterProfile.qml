import QtQuick 2.15
import QtQuick.Layouts 6.3
import QtQuick.Controls 6.3
import FilterBoxDataBasePackage 1.0

Item {
    width: 250
    height: 300

    ColumnLayout {
        anchors.fill: parent

        Button {
            id: buttonApplyChange
            text: qsTr("Apply Change")
            Layout.fillWidth: true
            Layout.preferredHeight: 28
            Layout.alignment: Qt.AlignTop

            onClicked: {
                _filterBoxDataBase.applyChange(
                            _filterBoxDataBase.activeFilterIdx,
                            textInputFilterName.text,
                            textInputFilterWeight.text,
                            textInputTimeIdxSt.text,
                            textInputTimeIdxEn.text,
                            textInputFreqIdxSt.text,
                            textInputFreqIdxEn.text);
            }
        }

        ColumnLayout {
            Layout.fillWidth: true
            Layout.fillHeight: true
            Layout.rightMargin: 10
            Layout.leftMargin: 10

            RowLayout {
                width: 250
                height: 20
                Layout.fillWidth: true

                Label {
                    width: 80
                    height: parent.height
                    text: qsTr("Filter Name : ")
                    Layout.preferredWidth: 80
                }
                Item {
                    Layout.fillWidth: true
                    height: 20
                    Rectangle {
                        anchors.fill: parent
                        color: "gainsboro"
                    }
                    TextInput {
                        id: textInputFilterName
                        text: _filterBoxDataBase.getFilterName(_filterBoxDataBase.activeFilterIdx)
                        anchors.fill: parent
                        font.pixelSize: 12
                        horizontalAlignment: Text.AlignHCenter
                        activeFocusOnTab: true
                        selectByMouse: true
                        selectionColor: "dodgerblue"
                        clip: true
                    }
                }
            }

            RowLayout {
                width: 250
                height: 20
                Layout.fillWidth: true
                Label {
                    width: 80
                    height: parent.height
                    text: qsTr("Filter Weight : ")
                    Layout.preferredWidth: 80
                }

                Item {
                    height: 20
                    Layout.fillWidth: true
                    Rectangle {
                        color: "#dcdcdc"
                        anchors.fill: parent
                    }

                    TextInput {
                        id: textInputFilterWeight
                        text: _filterBoxDataBase.getFilterWeight(_filterBoxDataBase.activeFilterIdx)
                        anchors.fill: parent
                        font.pixelSize: 12
                        horizontalAlignment: Text.AlignHCenter
                        activeFocusOnTab: true
                        selectionColor: "#1e90ff"
                        selectByMouse: true
                        clip: true
                    }
                }
            }

            Label {
                text: qsTr("Time Indices :")
            }

            Row {
                width: 230
                height: 400
                leftPadding: 30
                Layout.fillWidth: true
                spacing: 5

                Item {
                    width: 90
                    height: 20
                    Rectangle {
                        anchors.fill: parent
                        color: "gainsboro"
                    }
                    TextInput {
                        id: textInputTimeIdxSt
                        anchors.fill: parent
                        text: _filterBoxDataBase.getFilterTimeIdxSt(_filterBoxDataBase.activeFilterIdx)
                        font.pixelSize: 12
                        horizontalAlignment: Text.AlignHCenter
                        activeFocusOnTab: true
                        selectByMouse: true
                        selectionColor: "dodgerblue"
                        validator: IntValidator { bottom: 0; top: 1024*1024 }
                        clip: true

                        onTextChanged: {
                            labelTimeIdxSt.text = _filterBoxDataBase.getTimeRepresentation(textInputTimeIdxSt.text)
                        }
                    }
                }

                Label {
                    width: 10
                    text: qsTr("~")
                }

                Item {
                    width: 90
                    height: 20
                    Rectangle {
                        anchors.fill: parent
                        color: "gainsboro"
                    }
                    TextInput {
                        id: textInputTimeIdxEn
                        anchors.fill: parent
                        text: _filterBoxDataBase.getFilterTimeIdxEn(_filterBoxDataBase.activeFilterIdx)
                        font.pixelSize: 12
                        horizontalAlignment: Text.AlignHCenter
                        activeFocusOnTab: true
                        selectByMouse: true
                        selectionColor: "dodgerblue"
                        validator: IntValidator { bottom: 0; top: 1024*1024 }
                        clip: true

                        onTextChanged: {
                            labelTimeIdxEn.text = _filterBoxDataBase.getTimeRepresentation(textInputTimeIdxEn.text);
                        }
                    }
                }

            }

            Row {
                width: 300
                height: 400
                leftPadding: 30
                Layout.fillWidth: true
                spacing: 5

                Label {
                    id: labelTimeIdxSt
                    width: 90
                    horizontalAlignment: Text.AlignHCenter
                    text: qsTr("Label")
                }

                Label {
                    width: 10
                    text: qsTr("~")
                }

                Label {
                    id: labelTimeIdxEn
                    width: 90
                    horizontalAlignment: Text.AlignHCenter
                    text: qsTr("Label")
                }
            }

            Label {
                text: qsTr("Frequency Indices :")
            }

            Row {
                width: 230
                height: 400
                leftPadding: 30
                spacing: 5
                Layout.fillWidth: true

                Item {
                    width: 90
                    height: 20
                    Rectangle {
                        anchors.fill: parent
                        color: "gainsboro"
                    }
                    TextInput {
                        id: textInputFreqIdxSt
                        anchors.fill: parent
                        text: _filterBoxDataBase.getFilterFreqIdxSt(_filterBoxDataBase.activeFilterIdx)
                        font.pixelSize: 12
                        horizontalAlignment: Text.AlignHCenter
                        activeFocusOnTab: true
                        selectByMouse: true
                        selectionColor: "dodgerblue"
                        validator: IntValidator { bottom: 0; top: 1024 }
                        clip: true

                        onTextChanged: {
                            labelFreqIdxSt.text = _filterBoxDataBase.getFreqRepresentation(textInputFreqIdxSt.text, true)
                        }
                    }
                }

                Label {
                    width: 10
                    text: qsTr("~")
                }

                Item {
                    width: 90
                    height: 20
                    Rectangle {
                        anchors.fill: parent
                        color: "gainsboro"
                    }
                    TextInput {
                        id: textInputFreqIdxEn
                        anchors.fill: parent
                        text: _filterBoxDataBase.getFilterFreqIdxEn(_filterBoxDataBase.activeFilterIdx)
                        font.pixelSize: 12
                        horizontalAlignment: Text.AlignHCenter
                        activeFocusOnTab: true
                        selectByMouse: true
                        selectionColor: "dodgerblue"
                        validator: IntValidator { bottom: 0; top: 1024 }
                        clip: true

                        onTextChanged: {
                            labelFreqIdxEn.text = _filterBoxDataBase.getFreqRepresentation(textInputFreqIdxEn.text, false)
                        }
                    }
                }
            }

            Row {
                width: 200
                height: 400
                leftPadding: 30
                spacing: 5
                Layout.fillWidth: true
                Label {
                    id: labelFreqIdxSt
                    width: 90
                    horizontalAlignment: Text.AlignHCenter
                    text: qsTr("Label")
                }

                Label {
                    width: 10
                    text: qsTr("~")
                }

                Label {
                    id: labelFreqIdxEn
                    width: 90
                    horizontalAlignment: Text.AlignHCenter
                    text: qsTr("Label")
                }
            }
        }
    }
}
