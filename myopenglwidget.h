#ifndef MYOPENGLWIDGET_H
#define MYOPENGLWIDGET_H

#include "SoundDataBase.hpp"
#include "filterboxdatabase.h"
#include "SoundDeviceManager.hpp"

#include <QOpenGLWidget>
#include <QOpenGLFunctions>

class MyOpenGLWidget : public QOpenGLWidget, public QOpenGLFunctions
{
    Q_OBJECT
    Q_PROPERTY(float first_v READ getFirst_v WRITE setFirst_v NOTIFY first_vChanged)
    Q_PROPERTY(float second_v READ getSecond_v WRITE setSecond_v NOTIFY second_vChanged)
    Q_PROPERTY(float to_v READ getTo_v WRITE setTo_v NOTIFY to_vChanged)
    Q_PROPERTY(float first_h READ getFirst_h WRITE setFirst_h NOTIFY first_hChanged)
    Q_PROPERTY(float second_h READ getSecond_h WRITE setSecond_h NOTIFY second_hChanged)
    Q_PROPERTY(float to_h READ getTo_h WRITE setTo_h NOTIFY to_hChanged)

    GLuint textureId;
    std::vector<GLfloat> texture;
public:
    MyOpenGLWidget(QWidget* parent = nullptr);

    void connectSoundDataBase(SoundDataBase* pSoundDataBase);
    void connectFilterBoxDataBase(FilterBoxDataBase* pFilterBoxDataBase);
    void connectSoundDeviceManager(SoundDeviceManager* pSoundDeviceManager);

    void setFirst_v(float f);
    float getFirst_v() const;
    void setSecond_v(float s);
    float getSecond_v() const;
    void setTo_v(float to);
    float getTo_v() const;

    void setFirst_h(float f);
    float getFirst_h() const;
    void setSecond_h(float s);
    float getSecond_h() const;
    void setTo_h(float to);
    float getTo_h() const;

protected:
    virtual void initializeGL() override;
    virtual void resizeGL(int w, int h) override;
    virtual void paintGL() override;
    virtual void mousePressEvent(QMouseEvent *event) override;
    virtual void mouseReleaseEvent(QMouseEvent *event) override;
    virtual void mouseMoveEvent(QMouseEvent *event) override;

private:
    float first_v = 0;
    float second_v = 16;
    float to_v = 16;
    float first_h = 0;
    float second_h = 1024;
    float to_h = 1024;

    float timeIdxPress = 0;
    float freqIdxPress = 0;
    float timeIdxRelease = 0;
    float freqIdxRelease = 0;

    uint64_t timeIdxSt = 0;
    uint64_t timeIdxEn = 0;
    uint64_t freqIdxSt = 0;
    uint64_t freqIdxEn = 0;

    SoundDataBase* pSoundDataBase = nullptr;
    FilterBoxDataBase* pFilterBoxDataBase = nullptr;
    SoundDeviceManager* pSoundDeviceManager = nullptr;

    size_t batchCount_inputFrequency;

    size_t batchSampleCount_singleChannel;
    size_t sampleByteSize_singleChannel;
    size_t channelCount;
signals:
    void first_vChanged();
    void second_vChanged();
    void first_hChanged();
    void second_hChanged();
    void to_vChanged();
    void to_hChanged();

public slots:
    void updateTexture();
    void resetTexture();
};

#endif // MYOPENGLWIDGET_H
