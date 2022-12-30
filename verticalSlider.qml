import QtQuick
import QtQuick.Layouts
import QtQuick.Controls

Item {
    ColumnLayout {
        id: columnLayout
        anchors.fill: parent

        RangeSlider {
            id: rangeSlider
            Layout.fillWidth: true
            Layout.fillHeight: true;
            orientation: "Vertical"

            from: 0.0
            to: _glWidget.to_v

            first.value: _glWidget.first_v
            second.value: _glWidget.second_v
            snapMode: RangeSlider.SnapAlways
            stepSize: 1.0

//            enabled: false;
            first.onMoved: {
                console.log("first.value changed! " + first.value)
                _glWidget.first_v = first.value
                _glWidget.update();
            }
            second.onMoved: {
                console.log("second.value changed! " + second.value)
                _glWidget.second_v = second.value
                _glWidget.update();
            }

            onToChanged: {
                if (second.value === to) second.value = to;
            }
        }
    }

}
