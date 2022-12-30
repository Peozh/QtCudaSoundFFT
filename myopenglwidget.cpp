#include "myopenglwidget.h"
#include "qevent.h"


MyOpenGLWidget::MyOpenGLWidget(QWidget *parent)
    : QOpenGLWidget { parent }
{
    texture.resize((1024)*(16));
    for (auto& entry : texture) {
        entry = log2f(abs(rand()))/16;
        if (entry < 0) entry = 0;
        if (entry > 1) entry = 1;
    }
}

void MyOpenGLWidget::connectSoundDataBase(SoundDataBase *pSoundDataBase)
{
    this->pSoundDataBase = pSoundDataBase;
}
void MyOpenGLWidget::connectFilterBoxDataBase(FilterBoxDataBase *pFilterBoxDataBase)
{
    this->pFilterBoxDataBase = pFilterBoxDataBase;
}
void MyOpenGLWidget::connectSoundDeviceManager(SoundDeviceManager *pSoundDeviceManager)
{
    this->pSoundDeviceManager = pSoundDeviceManager;
}

void MyOpenGLWidget::setFirst_v(float f)
{
    if (this->first_v == f) return;
    this->first_v = f;
    emit first_vChanged();
}
float MyOpenGLWidget::getFirst_v() const
{
    return first_v;
}
void MyOpenGLWidget::setSecond_v(float s)
{
    if (this->second_v == s) return;
    this->second_v = s;
    emit second_vChanged();
}
float MyOpenGLWidget::getSecond_v() const
{
    return second_v;
}
void MyOpenGLWidget::setTo_v(float to)
{
    if (this->to_v == to) return;
    this->to_v = to;
    emit to_vChanged();
}
float MyOpenGLWidget::getTo_v() const
{
    return this->to_v;
}

void MyOpenGLWidget::setFirst_h(float f)
{
    if (this->first_h == f) return;
    this->first_h = f;
    emit first_hChanged();
}
float MyOpenGLWidget::getFirst_h() const
{
    return first_h;
}
void MyOpenGLWidget::setSecond_h(float s)
{
    if (this->second_h == s) return;
    this->second_h = s;
    emit second_hChanged();
}
float MyOpenGLWidget::getSecond_h() const
{
    return second_h;
}
void MyOpenGLWidget::setTo_h(float to)
{
    if (this->to_h == to) return;
    this->to_h = to;
    emit to_hChanged();
}
float MyOpenGLWidget::getTo_h() const
{
    return this->to_h;
}

void MyOpenGLWidget::initializeGL()
{
    initializeOpenGLFunctions();
    glClearColor(0, 0, 0, 1);

//    glEnable(GL_TEXTURE_2D);
    glGenTextures(1, &textureId); // 텍스쳐 id 에 텍스쳐를 하나 할당합니다.
    glActiveTexture(GL_TEXTURE0); // 활성화할 텍스쳐 슬롯을 지정합니다.
    glBindTexture(GL_TEXTURE_2D, textureId); // 현재 활성화된 텍스쳐 슬롯에 실질 텍스쳐를 지정합니다.
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, 1024, 16, 0, GL_RED, GL_FLOAT, texture.data()); // 텍스쳐 이미지가 RED 단일 채널이며, float 입니다. border 는 0 만 유효합니다.
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

    glEnable(GL_BLEND); // 아래에서 설정할 블렌딩 효과 활성화
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); // 전면(SRC)과 뒷면(DST)에 각각 곱해줄 계수
    glBlendEquation(GL_FUNC_ADD); // 위에서 얻은 두 항 간의 연산 방법 (default)

    glEnable(GL_LIGHT0);
    glEnable(GL_LIGHTING);

    glColorMaterial(GL_FRONT, GL_AMBIENT_AND_DIFFUSE);
    glEnable(GL_COLOR_MATERIAL);

    glEnable(GL_CULL_FACE); // 최적화 : 후면 제거 활성화
    glFrontFace(GL_CCW); // 전면/후면 판단 기준 (default)
    glCullFace(GL_BACK); // 후면만 폐기 (default)

//    glEnableClientState(GL_COLOR_ARRAY);
//    glEnableClientState(GL_VERTEX_ARRAY);
//    glEnableClientState(GL_TEXTURE_COORD_ARRAY);
}

void MyOpenGLWidget::resizeGL(int w, int h)
{
    glViewport(0, 0, w, h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    double left_h = -1.0 + 2.0*this->first_h/this->to_h;
    double right_h = -1.0 + 2.0*this->second_h/this->to_h;
    double top_v = -1.0 + 2.0*this->first_v/this->to_v;
    double bottom_v = -1.0 + 2.0*this->second_v/this->to_v;
    glOrtho(left_h, right_h, top_v, bottom_v, -1, 1);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
//    glRotatef(30.0, 1.0, 1.0, 1.0);
}

void MyOpenGLWidget::paintGL()
{
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    double left_h = -1.0 + 2.0*this->first_h/this->to_h;
    double right_h = -1.0 + 2.0*this->second_h/this->to_h;
    double top_v = -1.0 + 2.0*this->first_v/this->to_v;
    double bottom_v = -1.0 + 2.0*this->second_v/this->to_v;
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

//    std::vector<GLfloat> filterVertices = {
//        -0.3, -0.3, 0,
//         0.3, -0.3, 0,
//         0.3,  0.3, 0,
//        -0.3,  0.3, 0
//    };
    std::vector<GLfloat> filterColors = {
        0.0, 1.0, 1.0, 0.15,
        0.0, 1.0, 1.0, 0.15,
        0.0, 1.0, 1.0, 0.15,
        0.0, 1.0, 1.0, 0.15
    };
    std::vector<GLfloat> activeFilterColors = {
        0.0, 1.0, 1.0, 0.2,
        0.0, 1.0, 1.0, 0.2,
        0.0, 1.0, 1.0, 0.2,
        0.0, 1.0, 1.0, 0.2
    };
    std::vector<GLfloat> filterOutlineVerticeDiffs = {
        -1, -1, 0,
         1, -1, 0,
         1,  1, 0,
        -1,  1, 0
    };
    std::vector<GLfloat> filterOutlineColors = {
        0.0, 1.0, 1.0, 1,
        0.0, 1.0, 1.0, 1,
        0.0, 1.0, 1.0, 1,
        0.0, 1.0, 1.0, 1
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
//    glBegin(GL_POLYGON);
//    glBindTexture(GL_TEXTURE_2D, textureId);
//    glTexCoord2fv(texture2DCoords[0]); glVertex3fv(backgroundVertices[0]);
//    glTexCoord2fv(texture2DCoords[1]); glVertex3fv(backgroundVertices[1]);
//    glTexCoord2fv(texture2DCoords[2]); glVertex3fv(backgroundVertices[2]);
//    glTexCoord2fv(texture2DCoords[3]); glVertex3fv(backgroundVertices[3]);
//    glEnd();
    glDrawElements(GL_POLYGON, 4, GL_UNSIGNED_BYTE, VertexOrder);
    glDisableClientState(GL_TEXTURE_COORD_ARRAY);
    glDisable(GL_TEXTURE_2D);

    glEnableClientState(GL_COLOR_ARRAY);
//    std::cout << "this->pFilterBoxDataBase == nullptr ? " << (this->pFilterBoxDataBase == nullptr) << std::endl;

    // draw filter
    if (this->pFilterBoxDataBase != nullptr)
    {
        // draw un-activated filters
        glStencilFunc(GL_ALWAYS, 1, 0xFF); // 모든 fragment들은 통과, stencil buffer를 1으로 수정해야합니다
        glStencilMask(0x00); // stencil buffer 작성 (1 & 0x00 = 0) 비활성화

//        std::cout << "this->pFilterBoxDataBase->filters.size() ? " << (this->pFilterBoxDataBase->filters.size()) << std::endl;
        for (size_t idx = 0; idx < this->pFilterBoxDataBase->filters.size(); ++idx)
        {
            if (idx == this->pFilterBoxDataBase->activeFilterIdx) continue;
            FilterVertices filterVertices = this->pFilterBoxDataBase->filters[idx].filterVertices;
            glVertexPointer(3, GL_FLOAT, 0, &filterVertices);
            glColorPointer(4, GL_FLOAT, 0, filterColors.data());
            glDrawElements(GL_POLYGON, 4, GL_UNSIGNED_BYTE, VertexOrder);
        }

        // draw active filter
//        std::cout << "this->pFilterBoxDataBase->activeFilterIdx ? " << (this->pFilterBoxDataBase->activeFilterIdx) << std::endl;
        if (this->pFilterBoxDataBase->activeFilterIdx < this->pFilterBoxDataBase->filters.size()) // valid active filter index
        {
            // draw active filter
            glStencilFunc(GL_ALWAYS, 1, 0xFF); // 모든 fragment들은 통과, stencil buffer를 1으로 수정해야합니다
            glStencilMask(0xFF); // stencil buffer 작성 (1 & 0xFF = 1) 활성화
            FilterVertices filterVertices = this->pFilterBoxDataBase->filters[this->pFilterBoxDataBase->activeFilterIdx].filterVertices; // copy filter
            glVertexPointer(3, GL_FLOAT, 0, &filterVertices);
            glColorPointer(4, GL_FLOAT, 0, activeFilterColors.data());
            glDrawElements(GL_POLYGON, 4, GL_UNSIGNED_BYTE, VertexOrder);

            // draw active filter outline
            glStencilFunc(GL_NOTEQUAL, 1, 0xFF); // stencil buffer 가 1 로 작성되지 않은 모든 fragment 는 통과, stencil buffer 를 1로 수정해야 합니다.
            glStencilMask(0x00); // stencil buffer 작성 (1 & 0x00 = 0) 비활성화
            //glDisable(GL_DEPTH_TEST);
            float mag_h = (this->second_h - this->first_h)/this->to_h; // 0.0 ~ 1.0
            float mag_v = (this->second_v - this->first_v)/this->to_v; // 0.0 ~ 1.0
            float dw = 1.0f/this->width() * mag_h;
            float dh = 1.0f/this->height() * mag_v;
            auto pFilterVertices = (float*)&filterVertices;
            for (size_t idx = 0; idx < 12; idx += 3) // draw slightly bigger rect, vertex range [-1, 1]
            {
                pFilterVertices[idx]   += filterOutlineVerticeDiffs[idx]  *dw*2; // x
                pFilterVertices[idx+1] += filterOutlineVerticeDiffs[idx+1]*dh*2; // y
            }
//            filterVertices.LD[0] += filterOutlineVerticeDiffs[0]  *dw*2; // x
//            filterVertices.LD[1] += filterOutlineVerticeDiffs[0+1]*dh*2; // y
//            filterVertices.RD[0] += filterOutlineVerticeDiffs[3]  *dw*2; // x
//            filterVertices.RD[1] += filterOutlineVerticeDiffs[3+1]*dh*2; // y
//            filterVertices.RU[0] += filterOutlineVerticeDiffs[6]  *dw*2; // x
//            filterVertices.RU[1] += filterOutlineVerticeDiffs[6+1]*dh*2; // y
//            filterVertices.LU[0] += filterOutlineVerticeDiffs[9]  *dw*2; // x
//            filterVertices.LU[1] += filterOutlineVerticeDiffs[9+1]*dh*2; // y

            glVertexPointer(3, GL_FLOAT, 0, &filterVertices);
            auto pfv = (float*)&filterVertices;
//            for (size_t idx = 0; idx < 12; idx += 3) std::cout << "x = " << pfv[idx] << ", y = " <<  pfv[idx+1] << ", z = " << pfv[idx+2] << std::endl;
            glColorPointer(4, GL_FLOAT, 0, filterOutlineColors.data());
            glDrawElements(GL_POLYGON, 4, GL_UNSIGNED_BYTE, VertexOrder);
        }
    }

    // draw horizontal time guide line (white)
    {
        glStencilFunc(GL_ALWAYS, 1, 0xFF); // 모든 fragment들은 통과, stencil buffer를 수정할거면 1으로 수정해야합니다
        glStencilMask(0x00); // stencil buffer 작성 (1 & 0x00 = 0) 비활성화
        size_t playingIdx = -1;
        if (this->pSoundDeviceManager != nullptr) playingIdx = this->pSoundDeviceManager->getCurrentSliderIdx();
        float up = 1 - (float)playingIdx/to_v*2;
        float down = 1 - (float)(playingIdx+1)/to_v*2;
        float left = -1;
        float right = 1;
        FilterVertices timeGuideVertices {
            { left, down, 0 },
            { right, down, 0 },
            { right, up, 0 },
            { left, up, 0 }
        };
        std::vector<GLfloat> timeGuideColors = {
            1.0, 1.0, 1.0, 0.1,
            1.0, 1.0, 1.0, 0.1,
            1.0, 1.0, 1.0, 0.1,
            1.0, 1.0, 1.0, 0.1
        };
        glVertexPointer(3, GL_FLOAT, 0, &timeGuideVertices);
        glColorPointer(4, GL_FLOAT, 0, timeGuideColors.data());
        glDrawElements(GL_POLYGON, 4, GL_UNSIGNED_BYTE, VertexOrder);
    }

    // clean up
    glFlush();
    glStencilMask(0xFF);
    //glEnable(GL_DEPTH_TEST);
    glDisableClientState(GL_VERTEX_ARRAY);
    glDisableClientState(GL_COLOR_ARRAY);
}

void MyOpenGLWidget::mousePressEvent(QMouseEvent *event)
{
    int x = event->pos().x();
    int y = event->pos().y();
    if (x < 0) x = 0;
    if (y < 0) y = 0;
    if (x >= width()) x = width()-1;
    if (y >= height()) y = height()-1;
//    std::cout << "!! mouse pressed : (" << x << ", " << y << ")" << std::endl;

    // calc idx
    float idx_offset_x = this->first_h;
    float idx_offset_y = this->to_v - this->second_v;
    float range_h = (this->second_h - this->first_h);
    float range_v = (this->second_v - this->first_v);
    if (range_h == 0) return;
    if (range_v == 0) return;
    float dh = (float)this->width() / range_h;
    float dv = (float)this->height() / range_v;
    size_t idx_freq = idx_offset_x + (float)x/dh;
    size_t idx_time = idx_offset_y + (float)y/dv;

    this->freqIdxPress = idx_freq;
    this->timeIdxPress = idx_time;
    std::cout << std::fixed;
    std::cout.precision(2);
    std::cout << "!! mouse pressed : (" << idx_freq << "->" << this->freqIdxPress << ", " << idx_time << "->" << this->timeIdxPress << ")" << std::endl;
}
void MyOpenGLWidget::mouseReleaseEvent(QMouseEvent *event)
{
    int x = event->pos().x();
    int y = event->pos().y();
    if (x < 0) x = 0;
    if (y < 0) y = 0;
    if (x >= width()) x = width()-1;
    if (y >= height()) y = height()-1;
//    std::cout << "!! mouse released : (" << x << ", " << y << ")" << std::endl;

    // calc idx
    float idx_offset_x = this->first_h;
    float idx_offset_y = this->to_v - this->second_v;
    float range_h = (this->second_h - this->first_h);
    float range_v = (this->second_v - this->first_v);
    if (range_h == 0) return;
    if (range_v == 0) return;
    float dh = (float)this->width() / range_h;
    float dv = (float)this->height() / range_v;
    size_t idx_freq = idx_offset_x + (float)x/dh;
    size_t idx_time = idx_offset_y + (float)y/dv;

    this->freqIdxRelease = idx_freq;
    this->timeIdxRelease = idx_time;
    std::cout << std::fixed;
    std::cout.precision(2);
    std::cout << "!! mouse released : (" << idx_freq << "->" << this->freqIdxRelease << ", " << idx_time << "->" << this->timeIdxRelease << ")" << std::endl;

    if (this->freqIdxPress == this->freqIdxRelease && this->timeIdxPress == this->timeIdxRelease) return;

    this->freqIdxSt = std::round(std::min(this->freqIdxPress, this->freqIdxRelease));
    this->freqIdxEn = std::round(std::max(this->freqIdxPress, this->freqIdxRelease)) + 1;
    this->timeIdxSt = std::round(std::min(this->timeIdxPress, this->timeIdxRelease));
    this->timeIdxEn = std::round(std::max(this->timeIdxPress, this->timeIdxRelease)) + 1;

    std::cout << "!!!! filter generated : time = (" << this->timeIdxSt << ", " << this->timeIdxEn << ")" << std::endl;
    std::cout << "!!!! filter generated : freq = (" << this->freqIdxSt << ", " << this->freqIdxEn << ")" << std::endl;

    this->pFilterBoxDataBase->addFilter("new filter by mouse", 1.0f, this->timeIdxSt, this->timeIdxEn, this->freqIdxSt, this->freqIdxEn);
}
void MyOpenGLWidget::mouseMoveEvent(QMouseEvent *event)
{
//    std::cout << "!! mouse pressed : " << event << std::endl;
}

void MyOpenGLWidget::updateTexture()
{
    this->sampleByteSize_singleChannel = this->pSoundDataBase->sampleByteSize_singleChannel;
    this->batchSampleCount_singleChannel = this->pSoundDataBase->batchSampleCount_singleChannel;
    this->batchCount_inputFrequency = this->pSoundDataBase->inputFreqBatches.size();
    size_t targetBatchCount = this->batchCount_inputFrequency;
    size_t currentBatchCount;
    if (this->batchSampleCount_singleChannel == 0) { currentBatchCount = 0; }
    else { currentBatchCount = this->texture.size()/this->batchSampleCount_singleChannel; }
    std::vector<GLfloat> batch(this->batchSampleCount_singleChannel);

    this->setTo_h(this->batchSampleCount_singleChannel);
    this->setSecond_h(this->batchSampleCount_singleChannel);
    for (size_t idx = currentBatchCount; idx < targetBatchCount; ++idx)
    {
        if (this->sampleByteSize_singleChannel == 1) myCUDA::logSqrtComplexBatch_8((cufftComplex*)this->pSoundDataBase->inputFreqBatches[idx], batch.data(), this->batchSampleCount_singleChannel);
        else if (this->sampleByteSize_singleChannel == 2) myCUDA::logSqrtComplexBatch_16((cufftComplex*)this->pSoundDataBase->inputFreqBatches[idx], batch.data(), this->batchSampleCount_singleChannel);
        this->texture.insert(texture.end(), batch.begin(), batch.end());
    }
    this->setTo_v(targetBatchCount);
    this->setSecond_v(targetBatchCount);

    this->pFilterBoxDataBase->calcFilterVerticesAll();

    glActiveTexture(GL_TEXTURE0); // 활성화할 텍스쳐 슬롯을 지정합니다.
    glBindTexture(GL_TEXTURE_2D, textureId); // 현재 활성화된 텍스쳐 슬롯에 실질 텍스쳐를 지정합니다.
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, this->batchSampleCount_singleChannel, targetBatchCount, 0, GL_RED, GL_FLOAT, texture.data()); // 텍스쳐 이미지가 RED 단일 채널이며, float 입니다. border 는 0 만 유효합니다.}
    this->update();
}
void MyOpenGLWidget::resetTexture()
{
    this->texture = std::vector<GLfloat>{};
    this->texture.reserve(1024*256);

    glActiveTexture(GL_TEXTURE0); // 활성화할 텍스쳐 슬롯을 지정합니다.
    glBindTexture(GL_TEXTURE_2D, textureId); // 현재 활성화된 텍스쳐 슬롯에 실질 텍스쳐를 지정합니다.
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, 0, 0, 0, GL_RED, GL_FLOAT, texture.data()); // 텍스쳐 이미지가 RED 단일 채널이며, float 입니다. border 는 0 만 유효합니다.
}
