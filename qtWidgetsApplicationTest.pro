QT       += core gui quickwidgets openglwidgets

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    AlcInputDevice.cpp \
    AlcOutputDevice.cpp \
    devicecontrollers.cpp \
    filterboxdatabase.cpp \
    loadingdialog.cpp \
    main.cpp \
    myopenglfreqgraphwidget.cpp \
    myopenglwidget.cpp \
    widget.cpp

HEADERS += \
    AlcInputDevice.h \
    AlcOutputDevice.h \
    SoundDataBase.hpp \
    SoundDeviceManager.hpp \
    WaveHeader.hpp \
    devicecontrollers.h \
    filterboxdatabase.h \
    loadingdialog.h \
    myCudaForQt.h \
    myopenglfreqgraphwidget.h \
    myopenglwidget.h \
    widget.h

FORMS += \
    loadingdialog.ui \
    widget.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    res.qrc

#QTQUICK_COMPILER_SKIPPED_RESOURCES += res.qrc

INCLUDEPATH += \
    "C:\Coding\libs\openAL-Soft_1.22.2\openal-soft\include\AL"\
    "C:\Coding\VSCode\cuda_qt_SoundData_Test\include"

LIBS += \
    -L"C:\\Coding\\libs\\openAL-Soft_1.22.2\\openal-soft\\build\\Debug" -l"OpenAL32" \
    -l"opengl32" \
    -L"C:\\Coding\\QtCreator\\qtWidgetsApplicationTest" \
    -l"cudart" \
    -l"myCudaForQt"
