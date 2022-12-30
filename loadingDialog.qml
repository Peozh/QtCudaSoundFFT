import QtQuick
import QtQuick.Controls 6.3
import LoadingDialogPackage 1.0

Item {
    width: 100
    height: 100
    BusyIndicator {
        id: busyIndicator
        x: 30
        y: 30
    }

    Label {
        id: labelPercentage
        x: 36
        y: 76
        text: _loadingDialog.percentage
    }
}
