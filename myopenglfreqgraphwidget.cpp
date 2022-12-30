#include "myopenglfreqgraphwidget.h"

MyOpenGLFreqGraphWidget::MyOpenGLFreqGraphWidget(QWidget *parent)
    : QOpenGLWidget { parent }
{
    texture.resize((1)*(1));
}

void MyOpenGLFreqGraphWidget::connectSoundDataBase(SoundDataBase *pSoundDataBase)
{
    this->pSoundDataBase = pSoundDataBase;
}
void MyOpenGLFreqGraphWidget::connectSoundDeviceManager(SoundDeviceManager *pSoundDeviceManager)
{
    this->pSoundDeviceManager = pSoundDeviceManager;
}
void MyOpenGLFreqGraphWidget::setDrawTarget(bool isDrawPCM, bool isDrawInput)
{
    this->isDrawPCM = isDrawPCM;
    this->isDrawInput = isDrawInput;
}

void MyOpenGLFreqGraphWidget::initializeGL()
{
    initializeOpenGLFunctions();
    glClearColor(0, 0, 0, 1);

    glGenTextures(1, &textureId); // 텍스쳐 id 에 텍스쳐를 하나 할당합니다.
    glActiveTexture(GL_TEXTURE0); // 활성화할 텍스쳐 슬롯을 지정합니다.
    glBindTexture(GL_TEXTURE_2D, textureId); // 현재 활성화된 텍스쳐 슬롯에 실질 텍스쳐를 지정합니다.
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, 1, 1, 0, GL_RED, GL_FLOAT, texture.data()); // 텍스쳐 이미지가 RED 단일 채널이며, float 입니다. border 는 0 만 유효합니다.
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER); // s 축의 비어있는 텍스쳐 외부를 border 색상으로 채웁니다.
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER); // t 축의 비어있는 텍스쳐 외부를 border 색상으로 채웁니다.
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST); // 텍스쳐 확대 시 fragment 를 최근접 값으로 설정합니다.
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST); // 텍스쳐 축소 시 fragment 를 최근접 값으로 설정합니다.
    float colour[4] = {0.0f, 0.0f, 0.0f, 0.0f};
    glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, colour); // 텍스쳐 border 색상을 결정합니다.
    glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE); // (default)

    glEnable(GL_DEPTH_TEST); // 최적화 : Depth 테스트 실패 시 그려지지 않음
    glDepthFunc(GL_LEQUAL); // 겹치거나 가까우면 그리기
    // glEnable(GL_ALPHA_TEST); // 최적화 : 특정 Alpha 값 이하 제거(discard) threshold
    glEnable(GL_STENCIL_TEST);
    glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE); // 스텐실 테스트와 뎁스 테스트 모두 통과 시 stencil buffer 를 glStencilFunc 에서 지정한 ref 로 설정합니다. 나머지 경우 유지.

//    glEnable(GL_BLEND); // 아래에서 설정할 블렌딩 효과 활성화
//    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); // 전면(SRC)과 뒷면(DST)에 각각 곱해줄 계수
//    glBlendEquation(GL_FUNC_ADD); // 위에서 얻은 두 항 간의 연산 방법 (default)

    glEnable(GL_LIGHT0);
    glEnable(GL_LIGHTING);

    glColorMaterial(GL_FRONT, GL_AMBIENT_AND_DIFFUSE);
    glEnable(GL_COLOR_MATERIAL);

    glEnable(GL_CULL_FACE); // 최적화 : 후면 제거 활성화
    glFrontFace(GL_CCW); // 전면/후면 판단 기준 (default)
    glCullFace(GL_BACK); // 후면만 폐기 (default)
}
void MyOpenGLFreqGraphWidget::resizeGL(int w, int h)
{
    glViewport(0, 0, w, h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    double left_h = -1;
    double right_h = 1;
    double top_v = -1;
    double bottom_v = 1;
    glOrtho(left_h, right_h, top_v, bottom_v, -1, 1);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}
void MyOpenGLFreqGraphWidget::paintGL()
{
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    double left_h = -1;
    double right_h = 1;
    double top_v = -1;
    double bottom_v = 1;
    glOrtho(left_h, right_h, top_v, bottom_v, -1, 1);

    GLfloat backgroundVertices[4][3] = {
        {-1, -1, 0},
        { 1, -1, 0},
        { 1,  1, 0},
        {-1,  1, 0} };
    GLubyte VertexOrder[4] = { 0, 1, 2, 3 };
    GLfloat texture2DCoords[4][2] = {
        {0, 1},
        {1, 1},
        {1, 0},
        {0, 0}
    };

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

    // draw background
    glStencilMask(0x00);
    // 바닥을 그리는 동안에는 stencil buffer를 수정하지 않습니다
    // stencil buffer 작성 (1 & 0x00 = 0) 비활성화
    glEnable(GL_TEXTURE_2D);
    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, textureId);
    glVertexPointer(3, GL_FLOAT, 0, backgroundVertices);
    glTexCoordPointer(2, GL_FLOAT, 0, texture2DCoords);
    glDrawElements(GL_POLYGON, 4, GL_UNSIGNED_BYTE, VertexOrder);

    glDisableClientState(GL_TEXTURE_COORD_ARRAY);
    glDisableClientState(GL_VERTEX_ARRAY);
    glDisable(GL_TEXTURE_2D);

    // draw line graph
    drawLineGraph();

    // clean up
    glFlush();
    glStencilMask(0xFF);
}
void MyOpenGLFreqGraphWidget::drawLineGraph()
{
    glBegin(GL_LINE_STRIP);
    glColor4f(1.0, 0.0, 0.0, 1.0);
    for (size_t idx = 0; idx < normBatch.size(); ++idx)
    {
        float x = -1.0f + 2.0*idx/batchSampleCount_singleChannel;
        float y = normBatch[idx];
        glVertex3f(x, y, 0);
    }
    glEnd();
}

void MyOpenGLFreqGraphWidget::updateGraph()
{
    if (this->pSoundDataBase == nullptr) return;

    this->batchSampleCount_singleChannel = this->pSoundDataBase->batchSampleCount_singleChannel;
    if (this->batchSampleCount_singleChannel == 0) return;
    this->sampleByteSize_singleChannel = this->pSoundDataBase->sampleByteSize_singleChannel;
    if (this->sampleByteSize_singleChannel == 0) return;

    this->normBatch.resize(batchSampleCount_singleChannel); // [-1,1]

    size_t idx = this->pSoundDeviceManager->getCurrentSliderIdx();

    if (this->isDrawPCM == true)
    {
        if (this->isDrawInput == true)
        {
            if (idx >= this->pSoundDataBase->inputPCMBatches.size()) return;
            if (this->pSoundDataBase->inputPCMBatches[idx] == nullptr) return;
            if (sampleByteSize_singleChannel == 1) myCUDA:: normPCMBatch_8((uint8_t*)this->pSoundDataBase->inputPCMBatches[idx], normBatch.data(), batchSampleCount_singleChannel);
            else if (sampleByteSize_singleChannel == 2) myCUDA::normPCMBatch_16((int16_t*)this->pSoundDataBase->inputPCMBatches[idx], normBatch.data(), batchSampleCount_singleChannel);
        }
        else if (this->isDrawInput == false)
        {
            if (idx >= this->pSoundDataBase->outputPCMBatches.size()) return;
            if (this->pSoundDataBase->outputPCMBatches[idx] == nullptr) return;
            if (sampleByteSize_singleChannel == 1) myCUDA::normPCMBatch_8((uint8_t*)this->pSoundDataBase->outputPCMBatches[idx], normBatch.data(), batchSampleCount_singleChannel);
            else if (sampleByteSize_singleChannel == 2) myCUDA::normPCMBatch_16((int16_t*)this->pSoundDataBase->outputPCMBatches[idx], normBatch.data(), batchSampleCount_singleChannel);
        }
    }
    else if (this->isDrawPCM == false)
    {
        if (this->isDrawInput == true)
        {
            if (idx >= this->pSoundDataBase->inputFreqBatches.size()) return;
            if (this->pSoundDataBase->inputFreqBatches[idx] == nullptr) return;
            if (sampleByteSize_singleChannel == 1) myCUDA::normSqrtComplexBatch_8((cufftComplex*)this->pSoundDataBase->inputFreqBatches[idx], normBatch.data(), batchSampleCount_singleChannel);
            else if (sampleByteSize_singleChannel == 2) myCUDA::normSqrtComplexBatch_16((cufftComplex*)this->pSoundDataBase->inputFreqBatches[idx], normBatch.data(), batchSampleCount_singleChannel);
        }
        else if (this->isDrawInput == false)
        {
            if (idx >= this->pSoundDataBase->outputFreqBatches.size()) return;
            if (this->pSoundDataBase->outputFreqBatches[idx] == nullptr) return;
            if (sampleByteSize_singleChannel == 1) myCUDA::normSqrtComplexBatch_8((cufftComplex*)this->pSoundDataBase->outputFreqBatches[idx], normBatch.data(), batchSampleCount_singleChannel);
            else if (sampleByteSize_singleChannel == 2) myCUDA::normSqrtComplexBatch_16((cufftComplex*)this->pSoundDataBase->outputFreqBatches[idx], normBatch.data(), batchSampleCount_singleChannel);
        }
    }

}
