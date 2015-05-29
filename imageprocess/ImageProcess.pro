#-------------------------------------------------
#
# Project created by QtCreator 2013-04-28T19:39:22
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = ImageProcess
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    fftthread.cpp \
    setviewcolorwidget.cpp

HEADERS  += mainwindow.h \
    fftthread.h \
    setviewcolorwidget.h

FORMS    += mainwindow.ui \
    setViewColor.ui
OPENCV_LIB_DIR = C:/opencv/build/x86/vc10/lib

INCLUDEPATH +=   C:/opencv/build/include \
  C:/opencv/build/include/opencv \
  C:/opencv/build/include/opencv2 \
  C:/fftw-3.3.3-dll32

LIBS +=   $${OPENCV_LIB_DIR}/opencv_core244d.lib \
        $${OPENCV_LIB_DIR}/opencv_highgui244d.lib \
        $${OPENCV_LIB_DIR}/opencv_imgproc244d.lib \
        $${OPENCV_LIB_DIR}/opencv_features2d244d.lib\
        $${OPENCV_LIB_DIR}/opencv_nonfree244d.lib \
        $${OPENCV_LIB_DIR}/opencv_photo244d.lib \
        $${OPENCV_LIB_DIR}/opencv_legacy244d.lib \
        $${OPENCV_LIB_DIR}/opencv_calib3d244d.lib \
        C:/fftw-3.3.3-dll32/libfftw3f-3.lib \
        C:/fftw-3.3.3-dll32/libfftw3-3.lib \
        C:/fftw-3.3.3-dll32/libfftw3l-3.lib

INCLUDEPATH += C:/gdal/include \
LIBS += C:/gdal/lib/gdal_i.lib

