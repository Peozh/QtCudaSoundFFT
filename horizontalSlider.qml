import QtQuick
import QtQuick.Layouts
import QtQuick.Controls

Item {
    id: item1

    RowLayout {
        id: rowLayout
        anchors.fill: parent

        RangeSlider {
            id: rangeSlider
            Layout.fillWidth: true;
            Layout.fillHeight: true
            orientation: "Horizontal"

            from: 0
            to: _glWidget.to_h

            first.value: _glWidget.first_h
            second.value: _glWidget.second_h
            snapMode: RangeSlider.SnapAlways
            stepSize: 1

//            enabled: false;

            first.onMoved: {
                console.log("first.value changed! " + first.value)
                _glWidget.first_h = first.value
                _glWidget.update();
            }
            second.onMoved: {
                console.log("second.value changed! " + second.value)
                _glWidget.second_h = second.value
                _glWidget.update();
            }

            onToChanged: {
                if (second.value === to) second.value = to;
            }
        }
    }

}
