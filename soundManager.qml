import QtQuick
import QtQuick.Layouts
import QtQuick.Controls
import SoundDataBasePackage 1.0
import SoundDeviceManagerPackage 1.0

Item {
    ColumnLayout {
        id: columnLayout
        height: 55
        anchors.fill: parent

        Slider {
            id: sliderTime
            height: 20
            from: 0
            to: _soundDataBase.maxSliderIdx;
            value: _soundDeviceManager.currentSliderIdx;
            Layout.fillWidth: true
            stepSize: 1
            snapMode: "SnapAlways"

            MouseArea {
                id: mouseArea
                anchors.fill: parent
                acceptedButtons: Qt.LeftButton
                onClicked: {
                    if (mouseArea.pressedButtons & Qt.LeftButton) _soundDeviceManager.setPlayIndex(mouseX/sliderTime.width*sliderTime.to)
                }
                onMouseXChanged: {
                    _soundDeviceManager.setPlayIndex(mouseX/sliderTime.width*sliderTime.to)
                }
            }
        }

        RowLayout {
            id: rowLayout
            height: 35
            Layout.fillWidth: true

            Button {
                id: buttonRecordStop
                text: _soundDeviceManager.isRecording ? qsTr("Stop Record") : qsTr("Record")
                Layout.preferredWidth: 80
                Layout.fillHeight: true
                enabled: true
                onClicked: {
                    _soundDeviceManager.recordStartStop();
                }
            }
            Button {
                id: buttonPlayStop
                text: _soundDeviceManager.isPlaying ? qsTr("Stop Play") : qsTr("Play")
                Layout.preferredWidth: 80
                Layout.fillHeight: true
                enabled: true
                onClicked: {
                    _soundDeviceManager.playStartStop();
                }
            }

            Label {
                id: labelCurrentTime
                text: _soundDeviceManager.getTimeRepresentation( _soundDeviceManager.currentSliderIdx)
                horizontalAlignment: Text.AlignRight
                Layout.preferredWidth: 60
            }

            Label {
                text: qsTr("/")
            }

            Label {
                id: labelMaxTime
                text: _soundDeviceManager.getTimeRepresentation(_soundDataBase.maxSliderIdx)
                horizontalAlignment: Text.AlignLeft
                Layout.preferredWidth: 60
            }

            RowLayout {
                Layout.fillWidth: true
            }

            Button {
                id: buttonReset
                text: qsTr("Reset")
                Layout.preferredWidth: 80
                Layout.fillHeight: true
                Layout.alignment: Qt.AlignRight | Qt.AlignVCenter

                onClicked: {
                    _soundDeviceManager.resetPlayer();
                }
            }



        }
    }

}
