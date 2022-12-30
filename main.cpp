#include "widget.h"

#include <QApplication>
#include <QQuickWidget>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    QQuickWindow::setGraphicsApi(QSGRendererInterface::OpenGL);
    Widget mainWidget;
    mainWidget.show();
    return a.exec();
}
