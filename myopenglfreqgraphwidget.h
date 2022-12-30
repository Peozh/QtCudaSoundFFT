#ifndef MYOPENGLFREQGRAPHWIDGET_H
#define MYOPENGLFREQGRAPHWIDGET_H

#include "SoundDataBase.hpp"
#include "SoundDeviceManager.hpp"

#include <QObject>
#include <QOpenGLWidget>
#include <QOpenGLFunctions>

class MyOpenGLFreqGraphWidget : public QOpenGLWidget, public QOpenGLFunctions
{
    Q_OBJECT

    GLuint textureId;
    std::vector<GLfloat> texture;
public:
    MyOpenGLFreqGraphWidget(QWidget* parent = nullptr);

    void connectSoundDataBase(SoundDataBase* pSoundDataBase);
    void connectSoundDeviceManager(SoundDeviceManager* pSoundDeviceManager);
    void setDrawTarget(bool isDrawPCM, bool isDrawInput);

protected:
    virtual void initializeGL() override;
    virtual void resizeGL(int w, int h) override;
    virtual void paintGL() override;

private:
    SoundDataBase* pSoundDataBase = nullptr;
    SoundDeviceManager* pSoundDeviceManager = nullptr;

    size_t batchSampleCount_singleChannel = 0;
    size_t sampleByteSize_singleChannel = 0;
    bool isDrawPCM = true;
    bool isDrawInput = true;
    std::vector<GLfloat> normBatch; // [-1,1]

    void drawLineGraph();

signals:
    void to_vChanged();
    void to_hChanged();
public slots:
    void updateGraph();
};

#endif // MYOPENGLFREQGRAPHWIDGET_H
