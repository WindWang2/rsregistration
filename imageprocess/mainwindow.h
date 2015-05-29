#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QFileDialog>
#include <QGraphicsScene>
#include <QLabel>
#include <QDialog>
#include <QAction>
#include <QMessageBox>
#include <QPointF>
#include <QScrollArea>

#include "ShowWidget.h"
#include "ImageItem.h"
//#inlcude <QVector>
#include "gdal_priv.h"
#include "fftw3.h"
#include "opencv.hpp"
#include "opencv2/core/core.hpp"
#include "opencv2/features2d/features2d.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/legacy/legacy.hpp"
#include "opencv2/nonfree//nonfree.hpp"
#include "opencv2/nonfree/features2d.hpp"

namespace Ui {
class MainWindow;
}
using namespace cv;
typedef int* Int;
enum t_DataType
{
	t_uchar,t_int,t_double
};
class MainWindow : public QMainWindow
{
    Q_OBJECT
    



public slots:
    void openImageSlot();
    void readImageSlot(bool,Int);
    void showWrongMessageSlot(QString);
    void setColorSlot();
    void showImageSLot(bool);
    void fftSlot();
    void nfftSlot();
	void highPass();
	void lowPass();
    //测试
    void testSlot();
	void HESlot();
	void showPx(QPointF);
	void saveAS();

signals:
    void showWrongMessageSignal(QString);
    void readImageSignal(bool ,Int);
	void showImageSignal(bool);

public:
    explicit MainWindow(QWidget *parent = 0);
    void createConnect();
    Mat QImageToMat(QImage &image);
    QImage MatToQImage(Mat &mat);
	void showImage(bool);
	//测试函数
	bool createImage(QString source,QString dis);
    ~MainWindow();
    
private:
    Ui::MainWindow *ui;
private:
    QImage m_showImg;
    /*影像信息*/
    int m_xsize;
    int m_ysize;
    int m_bandCount;
    GDALDataset* m_poDataset;
    QString m_fileName;
    QGraphicsScene* m_scene;
	ShowWidget* m_showWidget;

	t_DataType t_type;
    unsigned char* buffer;	
	double* dBuffer;
	int* iBuffer;
	QString dirPath;
	//控件
	QScrollArea *scrollArea;
	ImageItem* imageWidget;

	double m_refAffine[6];
	QString m_projInfo;
	fftwf_complex* out;
	bool isFFt;
	//表示显示的是单波段还是多波段
	int m_showBands;
    bool isShow;
	//bool hasImage;
    //测试
	QString m_testFile;
    QDialog dia;
    QLabel* xx;
	QLabel* xyLabel;
};

#endif // MAINWINDOW_H
