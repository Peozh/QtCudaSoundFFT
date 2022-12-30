#ifndef WIDGET_H
#define WIDGET_H

#include "devicecontrollers.h"
#include "myopenglwidget.h"
#include "myopenglfreqgraphwidget.h"
#include "SoundDeviceManager.hpp"
#include "filterboxdatabase.h"
#include "loadingdialog.h"

#include <QWidget>
#include <QMainWindow>

QT_BEGIN_NAMESPACE
namespace Ui { class Widget; }
QT_END_NAMESPACE

class Widget : public QWidget
{
    Q_OBJECT

public:
    Widget(QWidget *parent = nullptr);
    ~Widget();

private:
    Ui::Widget *ui;
    DeviceControllers* pDeviceControllers;
    MyOpenGLWidget* pOpenGLWidget;
    MyOpenGLFreqGraphWidget* pOpenGLinputPCMGraph;
    MyOpenGLFreqGraphWidget* pOpenGLinputFreqGraph;
    MyOpenGLFreqGraphWidget* pOpenGLoutputPCMGraph;
    MyOpenGLFreqGraphWidget* pOpenGLoutputFreqGraph;
    SoundDataBase* pSoundDataBase;
    SoundDeviceManager* pSoundDeviceManager;
    FilterBoxDataBase* pFilterBoxDataBase;

    LoadingDialog* pLoadingDialog;
};
#endif // WIDGET_H
