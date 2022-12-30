#include "widget.h"
#include "ui_widget.h"

#include <QQmlContext>

//#include <QtCharts/QChartView>
//#include <QtCharts/QLineSeries>

Widget::Widget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Widget())
{
    ui->setupUi(this);
    QQmlContext* rootContext = nullptr;

    this->pSoundDataBase = new SoundDataBase(this);
    qmlRegisterUncreatableType<SoundDataBase>("SoundDataBasePackage", 1, 0, "SoundDataBase", "Access to SoundDataBase class");
    rootContext = ui->quickWidget_soundManager->rootContext();
    rootContext->setContextProperty("_soundDataBase", pSoundDataBase);

    this->pFilterBoxDataBase = new FilterBoxDataBase(this);
    this->pFilterBoxDataBase->connectSoundDataBase(this->pSoundDataBase);
    qmlRegisterUncreatableType<FilterBoxDataBase>("FilterBoxDataBasePackage", 1, 0, "FilterBoxDataBase", "Access to FilterBoxDataBase class");
    rootContext = ui->quickWidget_filterListView->rootContext();
    rootContext->setContextProperty("_filterBoxDataBase", this->pFilterBoxDataBase);
    rootContext = ui->quickWidget_filterProfile->rootContext();
    rootContext->setContextProperty("_filterBoxDataBase", this->pFilterBoxDataBase);

    this->pLoadingDialog = new LoadingDialog(this);

    this->pSoundDeviceManager = new SoundDeviceManager(this);
    this->pSoundDeviceManager->connectSoundDataBase(this->pSoundDataBase);
    this->pSoundDeviceManager->connectFilterBoxDataBase(this->pFilterBoxDataBase);
    qmlRegisterUncreatableType<SoundDeviceManager>("SoundDeviceManagerPackage", 1, 0, "SoundDeviceManager", "Access to SoundDeviceManager class");
    rootContext = ui->quickWidget_soundManager->rootContext();
    rootContext->setContextProperty("_soundDeviceManager", pSoundDeviceManager);

    this->pDeviceControllers = new DeviceControllers(this);
    this->pDeviceControllers->connectSoundDataBase(this->pSoundDataBase);
    this->pDeviceControllers->connectSoundDeviceManager(this->pSoundDeviceManager);
    this->pDeviceControllers->connectLoadingDialog(this->pLoadingDialog);
    qmlRegisterUncreatableType<DeviceControllers>("DeviceControllersPackage", 1, 0, "DeviceControllers", "Access to DeviceControllers class");
    rootContext = ui->quickWidget_deviceControllers->rootContext();
    rootContext->setContextProperty("_deviceControllers", pDeviceControllers);

    this->pOpenGLWidget = ui->glwidget;
    this->pOpenGLWidget->connectSoundDataBase(this->pSoundDataBase);
    this->pOpenGLWidget->connectFilterBoxDataBase(this->pFilterBoxDataBase);
    this->pOpenGLWidget->connectSoundDeviceManager(this->pSoundDeviceManager);
    qmlRegisterUncreatableType<MyOpenGLWidget>("MyOpenGLWidgetPackage", 1, 0, "MyOpenGLWidget", "Access to MyOpenGLWidget class");
    rootContext = ui->quickWidget_deviceControllers->rootContext();
    rootContext->setContextProperty("_glWidget", this->pOpenGLWidget);
    rootContext = ui->quickWidget_vertical->rootContext();
    rootContext->setContextProperty("_glWidget", this->pOpenGLWidget);
    rootContext = ui->quickWidget_horizontal->rootContext();
    rootContext->setContextProperty("_glWidget", this->pOpenGLWidget);
    rootContext = ui->quickWidget_soundManager->rootContext();
    rootContext->setContextProperty("_glWidget", this->pOpenGLWidget);
    rootContext = ui->quickWidget_filterListView->rootContext();
    rootContext->setContextProperty("_glWidget", this->pOpenGLWidget);

    this->pOpenGLinputPCMGraph = ui->graph1_originalWave;
    this->pOpenGLinputFreqGraph = ui->graph2_originalFrequency;
    this->pOpenGLoutputPCMGraph = ui->graph3_filteredWave;
    this->pOpenGLoutputFreqGraph = ui->graph4_filteredFrequency;
    bool drawPCM = true;
    bool drawFreq = false;
    bool drawInput = true;
    bool drawOutput = false;
    this->pOpenGLinputPCMGraph->setDrawTarget(drawPCM, drawInput);
    this->pOpenGLinputFreqGraph->setDrawTarget(drawFreq, drawInput);
    this->pOpenGLoutputPCMGraph->setDrawTarget(drawPCM, drawOutput);
    this->pOpenGLoutputFreqGraph->setDrawTarget(drawFreq, drawOutput);
    this->pOpenGLinputPCMGraph->connectSoundDataBase(this->pSoundDataBase);
    this->pOpenGLinputFreqGraph->connectSoundDataBase(this->pSoundDataBase);
    this->pOpenGLoutputPCMGraph->connectSoundDataBase(this->pSoundDataBase);
    this->pOpenGLoutputFreqGraph->connectSoundDataBase(this->pSoundDataBase);
    this->pOpenGLinputPCMGraph->connectSoundDeviceManager(this->pSoundDeviceManager);
    this->pOpenGLinputFreqGraph->connectSoundDeviceManager(this->pSoundDeviceManager);
    this->pOpenGLoutputPCMGraph->connectSoundDeviceManager(this->pSoundDeviceManager);
    this->pOpenGLoutputFreqGraph->connectSoundDeviceManager(this->pSoundDeviceManager);

    connect(this->pSoundDataBase, SIGNAL(afterPCMPushed()), this->pOpenGLWidget, SLOT(updateTexture()));
    connect(this->pSoundDataBase, SIGNAL(afterResetOccured()), this->pOpenGLWidget, SLOT(resetTexture()));

    connect(this->pFilterBoxDataBase, SIGNAL(activeFilterIdxChanged()), this->pOpenGLWidget, SLOT(update()));
    connect(this->pFilterBoxDataBase, SIGNAL(filterVerticesChanged()), this->pOpenGLWidget, SLOT(update()));
    connect(this->pSoundDeviceManager, SIGNAL(currentSliderIdxChanged()), this->pOpenGLWidget, SLOT(update()));

    connect(this->pSoundDeviceManager, SIGNAL(currentSliderIdxChanged()), this->pOpenGLinputPCMGraph, SLOT(updateGraph()));
    connect(this->pSoundDeviceManager, SIGNAL(currentSliderIdxChanged()), this->pOpenGLinputFreqGraph, SLOT(updateGraph()));
    connect(this->pSoundDeviceManager, SIGNAL(currentSliderIdxChanged()), this->pOpenGLoutputPCMGraph, SLOT(updateGraph()));
    connect(this->pSoundDeviceManager, SIGNAL(currentSliderIdxChanged()), this->pOpenGLoutputFreqGraph, SLOT(updateGraph()));
    connect(this->pSoundDeviceManager, SIGNAL(currentSliderIdxChanged()), this->pOpenGLinputPCMGraph, SLOT(update()));
    connect(this->pSoundDeviceManager, SIGNAL(currentSliderIdxChanged()), this->pOpenGLinputFreqGraph, SLOT(update()));
    connect(this->pSoundDeviceManager, SIGNAL(currentSliderIdxChanged()), this->pOpenGLoutputPCMGraph, SLOT(update()));
    connect(this->pSoundDeviceManager, SIGNAL(currentSliderIdxChanged()), this->pOpenGLoutputFreqGraph, SLOT(update()));

    connect(this->pSoundDataBase, SIGNAL(batchReadCurrentCountChanged()), this->pDeviceControllers, SLOT(updateLoadingDialog()));


    ui->quickWidget_deviceControllers->setSource(QUrl(QStringLiteral("qrc:/deviceControllers.qml")));
    ui->quickWidget_horizontal->setClearColor(Qt::lightGray);
    ui->quickWidget_horizontal->setSource(QUrl(QStringLiteral("qrc:/horizontalSlider.qml")));
    ui->quickWidget_vertical->setClearColor(Qt::lightGray);
    ui->quickWidget_vertical->setSource(QUrl(QStringLiteral("qrc:/verticalSlider.qml")));
    ui->quickWidget_soundManager->setSource(QUrl(QStringLiteral("qrc:/soundManager.qml")));
    ui->quickWidget_filterListView->setSource(QUrl(QStringLiteral("qrc:/filterBoxListView.qml")));
    ui->quickWidget_filterProfile->setSource(QUrl(QStringLiteral("qrc:/filterProfile.qml")));

}

Widget::~Widget()
{
    if (ui != nullptr) delete ui;
    //if (deviceControllers != nullptr) delete deviceControllers;
}

// todo : 마우스로 필터 생성 좌표 얻게 하기
// todo : 리코딩 잘되는지 확인하기
// todo : cuda 함수들 성능 테스트하기
// todo : 그래프 4개 그리기
