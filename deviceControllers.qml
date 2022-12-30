import QtQuick
import QtQuick.Layouts
import QtQuick.Controls
import DeviceControllersPackage 1.0

Item {
    id: itemDevice
    width: 1280
    height: 65
    GridLayout {
        id: gridLayout
        anchors.fill: parent
        rows: 2
        columns: 12

        Button {
            id: buttonSearchDevices
            text: qsTr("Search\nDevices")
            Layout.preferredWidth: 80
            GridLayout.column: 0
            GridLayout.row: 0
            Layout.rowSpan: 2
            Layout.fillHeight: true
            onClicked: {
                _deviceControllers.searchDevices()
            }
        }

        Button {
            id: buttonSearchWaveFile
            text: qsTr("Search\nWave File")
            Layout.preferredWidth: 80
            GridLayout.column: 1
            GridLayout.row: 0
            Layout.rowSpan: 2
            Layout.fillHeight: true
            onClicked: {
                _deviceControllers.searchFile()
            }
        }

        Label {
            id: labelBatchSize
            text: qsTr("Batch Size : ")
            Layout.preferredWidth: -1
            horizontalAlignment: Text.AlignRight
            verticalAlignment: Text.AlignVCenter
            Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
            GridLayout.column: 2
            GridLayout.row: 0
            Layout.rowSpan: 2
        }

        MyLongComboBox {
            id: comboBoxBatchSize
            Layout.preferredWidth: 80
            Layout.preferredHeight: 20
            model: _deviceControllers.batchSizeListModel
            currentIndex: 0
            GridLayout.column: 3
            GridLayout.row: 0
            Layout.rowSpan: 2
            onCurrentTextChanged: _deviceControllers.selectBatchSize(currentIndex)
        }

        Label {
            id: labelInputDevice
            text: qsTr("Input Device : ")
            Layout.preferredWidth: -1
            horizontalAlignment: Text.AlignRight
            Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
            GridLayout.column: 4
            GridLayout.row: 0
        }

        Label {
            id: labelOutputDevice
            text: qsTr("Output Device : ")
            Layout.preferredWidth: -1
            horizontalAlignment: Text.AlignRight
            Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
            GridLayout.column: 4
            GridLayout.row: 1
        }

        MyLongComboBox {
            id: comboBoxInputDevice
            Layout.preferredWidth: 400
            Layout.preferredHeight: 20
            currentIndex: -1
            model: _deviceControllers.inputDeviceListModel
            GridLayout.column: 5
            GridLayout.row: 0
            onCurrentTextChanged: _deviceControllers.selectInputDevice(currentIndex)
        }

        MyLongComboBox {
            id: comboBoxOutputDevice
            Layout.preferredWidth: 400
            Layout.preferredHeight: 20
            currentIndex: -1
            model: _deviceControllers.outputDeviceListModel
            GridLayout.column: 5
            GridLayout.row: 1
            onCurrentTextChanged: _deviceControllers.selectOutputDevice(currentIndex)
        }

        Button {
            id: buttonLoadDevices
            text: qsTr("Load\nDevices")
            GridLayout.column: 6
            GridLayout.row: 0
            Layout.rowSpan: 2
            Layout.preferredWidth: 80
            Layout.fillHeight: true
            onClicked: {
                _deviceControllers.loadDevices()
                _glWidget.update()
            }
        }

        Button {
            id: buttonUnloadDevices
            text: qsTr("Unload\nDevices")
            GridLayout.column: 7
            GridLayout.row: 0
            Layout.rowSpan: 2
            Layout.preferredWidth: 80
            Layout.fillHeight: true
            onClicked: {
                _deviceControllers.unloadDevices()
                _glWidget.resetTexture()
                _glWidget.update()
            }
        }


        Label {
            id: labelInputSampleRate
            text: qsTr("Input Sample Rate[Hz] : ")
            horizontalAlignment: Text.AlignRight
            Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
            GridLayout.column: 8
            GridLayout.row: 0
        }

        Label {
            id: labelOutputSampleRate
            text: qsTr("Output Sample Rate[Hz] : ")
            horizontalAlignment: Text.AlignRight
            Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
            GridLayout.column: 8
            GridLayout.row: 1
        }

        Label {
            id: labelInputSampleRateValue
            text: _deviceControllers.inputSampleRate
            horizontalAlignment: Text.AlignLeft
            Layout.preferredWidth: 80
            GridLayout.column: 9
            GridLayout.row: 0
            clip: true
        }

        Label {
            id: labelOutputSampleRateValue
            text: _deviceControllers.outputSampleRate
            horizontalAlignment: Text.AlignLeft
            Layout.preferredWidth: 80
            GridLayout.column: 9
            GridLayout.row: 1
            clip: true
        }

        Label {
            id: labelSampleMonoStereo
            text: qsTr("Channel : ")
            horizontalAlignment: Text.AlignRight
            Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
            GridLayout.column: 10
            GridLayout.row: 0
        }

        Label {
            id: labelSampleBit
            text: qsTr("Sample Bit : ")
            horizontalAlignment: Text.AlignRight
            Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
            GridLayout.column: 10
            GridLayout.row: 1
        }

        Label {
            id: labelSampleMonoStereoValue
            text: _deviceControllers.sampleChannel
            horizontalAlignment: Text.AlignLeft
            Layout.preferredWidth: 80
            GridLayout.column: 11
            GridLayout.row: 0
            clip: true
        }

        Label {
            id: labelSampleBitValue
            text: _deviceControllers.sampleBits
            horizontalAlignment: Text.AlignLeft
            Layout.preferredWidth: 80
            GridLayout.column: 11
            GridLayout.row: 1
            clip: true
        }
    }
}
