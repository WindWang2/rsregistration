#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "setviewcolorwidget.h"
#include <QDebug>
#include <cmath>

#include "gdal_priv.h"

typedef unsigned char BYTE;
typedef int* Int;
MainWindow::MainWindow(QWidget *parent) :
QMainWindow(parent),
	ui(new Ui::MainWindow)
{
	ui->setupUi(this);
	//注册GDAL驱动
	GDALAllRegister();
	//m_showWidget = new ShowWidget(this);

	xyLabel = new QLabel(this);
	
	imageWidget = new ImageItem(this);
	imageWidget->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);
	imageWidget->setScaledContents(true);

	scrollArea = new QScrollArea(this);
	scrollArea->setBackgroundRole(QPalette::Dark);
	scrollArea->setWidget(imageWidget);

	ui->ViewWidget->layout()->
		addWidget(scrollArea);

	ui->ViewWidget->layout()->addWidget(xyLabel);
	//m_scene = new QGraphicsScene();
	//m_showWidget->setScene(m_scene);
	
	createConnect();
	//初始化不然会出错
	buffer = 0;
	iBuffer = 0;
	dBuffer = 0;
	m_poDataset = 0;
	out = 0;
	showImage(false);
	//hasImage =false;
	//测试
	isFFt = false;
	xx = new QLabel();
	QGridLayout* layout = new QGridLayout();
	layout->addWidget(xx);
	dia.setLayout(layout);
	dia.setGeometry(QRect(100,100,500,500));

	//Create Dir
	QString xx = QDir::currentPath();
	QDir d;
	bool exist = d.exists(xx+"/temp");
	if (exist)
	{
		QMessageBox::warning(this,QStringLiteral("创建文件夹"),
			QStringLiteral("文件夹已经存在"));
	}
	else
	{
		d.mkdir(xx+"/temp");
	}
	dirPath = xx+"/temp";
}
bool removeDir(const QString & dirName)
{
	bool result;
	QDir dir(dirName);

	if (dir.exists(dirName)) {
		Q_FOREACH(QFileInfo info, dir.entryInfoList(QDir::NoDotAndDotDot | QDir::System | QDir::Hidden  | QDir::AllDirs | QDir::Files, QDir::DirsFirst)) {
			if (info.isDir()) {
				result = removeDir(info.absoluteFilePath());
			}
			else {
				result = QFile::remove(info.absoluteFilePath());
			}

			if (!result) {
				return result;
			}
		}
		result = dir.rmdir(dirName);
	}
	return result;
}
MainWindow::~MainWindow()
{

	if(buffer)
	{
		qDebug()<<"Error";
		delete[] buffer;
		buffer = NULL;
	}
	if (dBuffer)
	{
		qDebug()<<"Error";
		delete[] dBuffer;
		buffer = NULL;
	}
	if (iBuffer)
	{
		qDebug()<<"Error";
		delete[] iBuffer;
		buffer = NULL;
	}
	//delete[] iBuffer;
	//delete[] dBuffer;
	if (out)
	{
		delete[] out;
		out = NULL;
	}
	QDir d;
	bool exist = d.exists(dirPath);
	if (exist)
	{
		if (!removeDir(dirPath))
		{
			showWrongMessageSignal("删除临时文件夹失败");
		}
	}
//	delete m_showWidget;
	delete xyLabel;
	delete ui;
}
bool MainWindow::createImage(QString source,QString dis)
{
	return true;
}
void MainWindow::openImageSlot()
{
	if (m_poDataset != NULL)
	{
		GDALClose(m_poDataset);
		m_poDataset = NULL;
	}
	m_fileName = QFileDialog::getOpenFileName(this,QStringLiteral("选择打开的图像文件"),"/",
		"Image Files(*.jpg *.png *.tif)");
	if(m_fileName == "")
	{
		qDebug()<<"the File Name is Null";
		emit showWrongMessageSignal(QStringLiteral("没有选择文件"));
	}
	else
	{
		m_poDataset = (GDALDataset*)GDALOpen(m_fileName.toStdString().c_str(),GA_ReadOnly);
		if(m_poDataset == NULL)
		{
			emit showWrongMessageSignal(QStringLiteral("文件打开失败"));
			return;
		}

		GDALDataType xx = m_poDataset->GetRasterBand(1)->GetRasterDataType();

		//现阶段只支持Byte型数据

		if (xx == GDT_Byte)
		{
			t_type = t_uchar;
		}
		else if(xx == GDT_Float32||xx == GDT_Float64)
		{
			t_type = t_double;
		}
		else if(xx == GDT_Int16||xx == GDT_Int32||xx == GDT_Int16||xx == GDT_UInt32)
		{
			t_type = t_int;
		}
		else
		{
			emit showWrongMessageSignal(QStringLiteral("不支持数据类型"));
			return;
		}
		m_xsize = m_poDataset->GetRasterXSize();
		m_ysize = m_poDataset->GetRasterYSize();
		m_bandCount = m_poDataset->GetRasterCount();
		m_poDataset->GetGeoTransform(m_refAffine);
		const char* tempProj = m_poDataset->GetProjectionRef();
		m_projInfo = QString(m_poDataset->GetProjectionRef());

		//默认黑白
		int bandMap[3];
		bool isColor = false;
		if(m_bandCount>=3)
		{
			bandMap[0] = 1;
			bandMap[1] = 2;
			bandMap[2] = 3;
			isColor = true;
		}
		else
		{
			bandMap[0] = 1;
		}
		emit readImageSignal(isColor,bandMap);
	}
}
//创建系统信号槽链接
void MainWindow::createConnect()
{
	connect(this,SIGNAL(readImageSignal(bool ,Int)),this,SLOT(readImageSlot(bool ,Int)));
	connect((ui->action_Open),SIGNAL(triggered()),this,SLOT(openImageSlot()));
	connect(this,SIGNAL(showWrongMessageSignal(QString)),this,SLOT(showWrongMessageSlot(QString)));
	connect(ui->actionChannels,SIGNAL(triggered()),this,SLOT(setColorSlot()));
	connect(ui->actionFFT_2,SIGNAL(triggered()),this,SLOT(fftSlot()));
	connect(ui->actionNFFT_2,SIGNAL(triggered()),this,SLOT(nfftSlot()));
	connect(this,SIGNAL(showImageSignal(bool)),this,SLOT(showImageSLot(bool)));
	connect(ui->actionLow_Pass,SIGNAL(triggered()),this,SLOT(lowPass()));
	connect(ui->actionHigh_Pass,SIGNAL(triggered()),this,SLOT(highPass()));
	connect(imageWidget,SIGNAL(sendPX(QPointF)),this,SLOT(showPx(QPointF)));
	connect(ui->action_Save_As,SIGNAL(triggered()),this,SLOT(saveAS()));
	//	connect(ui->graphicsView,SIGNAL())
	//测试
	connect(ui->actionTest,SIGNAL(triggered()),this,SLOT(testSlot()));
	connect(ui->actionHistogram_equalization,SIGNAL(triggered()),this,SLOT(HESlot()));

}
void MainWindow::showPx(QPointF px)
{
	if (isShow)
	{
		QString pXY = "X:"+QString::number(px.x(),'f',2)+" , Y:"+QString::number(px.y(),'f',2);
		QString rXY = "MapX:"+QString::number(px.x()*m_refAffine[1]+m_refAffine[0]+m_refAffine[2]*px.y(),'f',3)+
			"m, MapY:"+QString::number(px.y()*m_refAffine[5]+px.x()*m_refAffine[4]+m_refAffine[3],'f',3)+"m";
		xyLabel->setText(pXY+"........"+rXY);
	}
}
void MainWindow::showWrongMessageSlot(QString message)
{
	QMessageBox::critical(this,QStringLiteral("错误"),message);
}
void MainWindow::readImageSlot(bool isColor,Int bandMap)
{
	if(buffer)
	{
		qDebug()<<"Error";
		delete[] buffer;
		buffer = NULL;
	}
	if (dBuffer)
	{
		qDebug()<<"Error";
		delete[] dBuffer;
		dBuffer = NULL;

	}
	if (iBuffer)
	{
		qDebug()<<"Error";
		delete[] iBuffer;
		iBuffer = NULL;

	}
	if(isColor)
	{
		buffer = new BYTE[m_xsize*m_ysize*3];
		switch (t_type)
		{
		case t_uchar:
			{
				CPLErr x1 = m_poDataset->RasterIO(GF_Read,0,0,m_xsize,m_ysize,buffer,
					m_xsize,m_ysize,GDT_Byte,3,bandMap,3,3*m_xsize,1);
				if(x1!=CE_None)
				{
					emit showWrongMessageSignal(QStringLiteral("Color   RasterIO错误"));
					return;
				}
			}
			break;
		case t_int:
			{
				iBuffer = new int[m_xsize*m_ysize*3];
				int ts = sizeof(int);
				CPLErr x2 = m_poDataset->RasterIO(GF_Read,0,0,m_xsize,m_ysize,iBuffer,
					m_xsize,m_ysize,GDT_Int32,3,bandMap,3*ts,3*m_xsize*ts,ts);
				if(x2!=CE_None)
				{
					emit showWrongMessageSignal(QStringLiteral("Int Color RasterIO错误"));
					return;
				}
				//统计
				int threshold = static_cast<int>(m_xsize*m_ysize*0.02);
				int line[6];
				for (int i = 0;i<3;++i)
				{
					double MinMax[2];
					GDALRasterBand* xx = m_poDataset->GetRasterBand(bandMap[i]);
					xx->ComputeRasterMinMax(0,MinMax);
					int tempHist[200];
					xx->GetHistogram(MinMax[0]-0.5,MinMax[1]+0.5,200,tempHist,false,false,NULL,NULL);
					int tempTotal1=0;
					int tempTotal2=0;
					bool begin = true;
					bool end = true;
					for (int j = 0;j<200;j++)
					{
						tempTotal1+=tempHist[j];
						if (tempTotal1 > threshold&&begin)
						{
							line[2*i] = static_cast<int>(MinMax[0]+j*(MinMax[1]-MinMax[0])/200);
							begin = false;
						}
						tempTotal2+=tempHist[199-j];
						if (tempTotal2>threshold&&end)
						{
							line[2*i+1] = static_cast<int>(MinMax[0]+(199-j)*(MinMax[1]-MinMax[0])/200);
							end = false;
						}
						if (!begin && !end)
						{
							break;
						}
					}
				}
				//线性拉伸
				for (int i = 0;i<3;++i)
				{
					for (int y = 0;y<m_ysize;++y)
					{
						for (int x = 0;x<m_xsize;++x)
						{
							int id = i*m_xsize*m_ysize+y*m_xsize+x;
							if (iBuffer[id]<=line[2*i])
							{
								buffer[id] = static_cast<unsigned char>(0);
							}else if (iBuffer[id]>=line[2*i+1])
							{
								buffer[id] = static_cast<unsigned char>(255);
							}
							else
							{
								unsigned char xx = static_cast<unsigned char>(
									(iBuffer[id]-line[2*i])*255/(line[2*i+1]-line[2*i]));
								buffer[id] = xx;
							}
						}
					}
				}
			}
			delete[] iBuffer;
			iBuffer = NULL;
			break;
		case t_double:
			{
				dBuffer = new double[m_xsize*m_ysize*3];
				int ts = sizeof(double);
				CPLErr x2 = m_poDataset->RasterIO(GF_Read,0,0,m_xsize,m_ysize,dBuffer,
					m_xsize,m_ysize,GDT_Float64,3,bandMap,3*ts,3*m_xsize*ts,ts);
				if(x2!=CE_None)
				{
					emit showWrongMessageSignal(QStringLiteral("Int Color RasterIO错误"));
					return;
				}
				//统计
				int threshold = static_cast<int>(m_xsize*m_ysize*0.02);
				double line[6];
				for (int i = 0;i<3;++i)
				{
					double MinMax[2];
					GDALRasterBand* xx = m_poDataset->GetRasterBand(bandMap[i]);
					xx->ComputeRasterMinMax(0,MinMax);
					int tempHist[200];
					xx->GetHistogram(MinMax[0]-0.000001,MinMax[1]+0.000001,200,tempHist,false,false,NULL,NULL);
					int tempTotal1=0;
					int tempTotal2=0;
					bool begin = true;
					bool end = true;
					for (int j = 0;j<200;j++)
					{
						tempTotal1+=tempHist[j];
						if (tempTotal1 > threshold&&begin)
						{
							line[2*i] = (MinMax[0]+j*(MinMax[1]-MinMax[0])/200.0);
							begin = false;
						}
						tempTotal2+=tempHist[199-j];
						if (tempTotal2>threshold&&end)
						{
							line[2*i+1] = (MinMax[0]+(199-j)*(MinMax[1]-MinMax[0])/200.0);
							end = false;
						}
						if (!begin && !end)
						{
							break;
						}
					}
				}
				//线性拉伸
				for (int i = 0;i<3;++i)
				{
					for (int y = 0;y<m_ysize;++y)
					{
						for (int x = 0;x<m_xsize;++x)
						{
							int id = i*m_xsize*m_ysize+y*m_xsize+x;
							if (dBuffer[id]<=line[2*i])
							{
								buffer[id] = static_cast<unsigned char>(0);
							}else if (dBuffer[id]>=line[2*i+1])
							{
								buffer[id] = static_cast<unsigned char>(255);
							}
							else
							{
								unsigned char xx = static_cast<unsigned char>(
									(dBuffer[id]-line[2*i])*255/(line[2*i+1]-line[2*i]));
								buffer[id] = xx;
							}
						}
					}
				}
			}
			delete[] dBuffer;
			dBuffer = NULL;
			break;
		default:
			emit showWrongMessageSignal(QStringLiteral("t_type为空"));
			return;
		}

		m_showImg = QImage(buffer,m_xsize,m_ysize,m_xsize*3,QImage::Format_RGB888);
		m_showImg.save(QStringLiteral("D:\\test2.jpg"));
		//dia.exec();
		//m_scene->clear();
		//m_scene->items().clear();
		imageWidget->setPixmap(QPixmap::fromImage(m_showImg,Qt::ColorOnly));
		//m_scene->addPixmap(QPixmap::fromImage(m_showImg,Qt::ColorOnly));
		//m_scene->update(QRectF(0,0,m_xsize,m_ysize));
		imageWidget->adjustSize();

		showImage(true);
		m_showBands = 3;
	}
	else
	{
		buffer = new BYTE[m_xsize*m_ysize];
		switch (t_type)
		{
			case t_uchar:
			{
				CPLErr xx = m_poDataset->GetRasterBand(bandMap[0])->RasterIO(GF_Read,0,0,
					m_xsize,m_ysize,buffer,m_xsize,
					m_ysize,GDT_Byte,0,0);
				if(xx!=CE_None)
				{
					emit showWrongMessageSignal(QStringLiteral("Gray   RasterIO错误"));
					return;
				}
			}
			break;
			case t_int:
			{
				iBuffer = new int[m_xsize*m_ysize];
				int ts = sizeof(int);
				CPLErr xx = m_poDataset->GetRasterBand(bandMap[0])->RasterIO(GF_Read,0,0,
					m_xsize,m_ysize,iBuffer,m_xsize,
					m_ysize,GDT_Int32,0,0);
				if(xx!=CE_None)
				{
					emit showWrongMessageSignal(QStringLiteral("Int Color RasterIO错误"));
					return;
				}
				//统计
				int threshold = static_cast<int>(m_xsize*m_ysize*0.02);
				int line[2];

				double MinMax[2];
				GDALRasterBand* xxx = m_poDataset->GetRasterBand(bandMap[0]);
				xxx->ComputeRasterMinMax(0,MinMax);
				int tempHist[200];
				xxx->GetHistogram(MinMax[0]-0.5,MinMax[1]+0.5,200,tempHist,false,false,NULL,NULL);
				int tempTotal1=0;
				int tempTotal2=0;
				bool begin = true;
				bool end = true;
				for (int j = 0;j<200;j++)
				{
					tempTotal1+=tempHist[j];
					if (tempTotal1 > threshold&&begin)
					{
						line[0] = static_cast<int>(MinMax[0]+j*(MinMax[1]-MinMax[0])/200);
						begin = false;
					}
					tempTotal2+=tempHist[199-j];
					if (tempTotal2>threshold&&end)
					{
						line[1] = static_cast<int>(MinMax[0]+(199-j)*(MinMax[1]-MinMax[0])/200);
						end = false;
					}
					if (!begin && !end)
					{
						break;
					}
				}
				//线性拉伸

				for (int y = 0;y<m_ysize;++y)
				{
					for (int x = 0;x<m_xsize;++x)
					{
						int id = y*m_xsize+x;
						if (iBuffer[id]<=line[0])
						{
							buffer[id] = static_cast<unsigned char>(0);
						}else if (iBuffer[id]>=line[1])
						{
							buffer[id] = static_cast<unsigned char>(255);
						}
						else
						{
							unsigned char xx = static_cast<unsigned char>(
								(iBuffer[id]-line[0])*255/(line[1]-line[0]));
							buffer[id] = xx;
						}
					}
				}
			}
			delete[] iBuffer;
			iBuffer = NULL;
			//delete[] iBuffer;
			break;
			case t_double:
				{
					dBuffer = new double[m_xsize*m_ysize];
					//int ts = sizeof(int);
					CPLErr xx = m_poDataset->GetRasterBand(bandMap[0])->RasterIO(GF_Read,0,0,
						m_xsize,m_ysize,dBuffer,m_xsize,
						m_ysize,GDT_Float64,0,0);
					if(xx!=CE_None)
					{
						emit showWrongMessageSignal(QStringLiteral("double Color RasterIO错误"));
						return;
					}
					//统计
					int threshold = static_cast<int>(m_xsize*m_ysize*0.02);
					double line[2];

					double MinMax[2];
					GDALRasterBand* xxx = m_poDataset->GetRasterBand(bandMap[0]);
					xxx->ComputeRasterMinMax(0,MinMax);
					int tempHist[200];
					xxx->GetHistogram(MinMax[0]-0.5,MinMax[1]+0.5,200,tempHist,false,false,NULL,NULL);
					int tempTotal1=0;
					int tempTotal2=0;
					bool begin = true;
					bool end = true;
					for (int j = 0;j<200;j++)
					{
						tempTotal1+=tempHist[j];
						if (tempTotal1 > threshold&&begin)
						{
							line[0] = (MinMax[0]+j*(MinMax[1]-MinMax[0])/200);
							begin = false;
						}
						tempTotal2+=tempHist[199-j];
						if (tempTotal2>threshold&&end)
						{
							line[1] = (MinMax[0]+(199-j)*(MinMax[1]-MinMax[0])/200);
							end = false;
						}
						if (!begin && !end)
						{
							break;
						}
					}
					//线性拉伸

					for (int y = 0;y<m_ysize;++y)
					{
						for (int x = 0;x<m_xsize;++x)
						{
							int id = y*m_xsize+x;
							if (dBuffer[id]<=line[0])
							{
								buffer[id] = static_cast<unsigned char>(0);
							}else if (dBuffer[id]>=line[1])
							{
								buffer[id] = static_cast<unsigned char>(255);
							}
							else
							{
								unsigned char xx = static_cast<unsigned char>(
									(dBuffer[id]-line[0])*255/(line[1]-line[0]));
								buffer[id] = xx;
							}
						}
					}
				}
			delete[] dBuffer;
			dBuffer = NULL;
			break;
			default:
				emit showWrongMessageSignal("t_type is NULL single");
		}
		

		m_showImg = QImage(buffer,m_xsize,m_ysize,m_xsize,QImage::Format_Indexed8);
		//m_showImg = QImage(buffer,m_xsize,m_ysize,m_xsize*3,QImage::Format_RGB888);
		//m_scene->clear();
		//m_scene->items().clear();
		imageWidget->setPixmap(QPixmap::fromImage(m_showImg,Qt::AutoColor));
		imageWidget->adjustSize();
		showImage(true);
		m_showBands=1;
	}
}
//void MainWindow::show
QImage MainWindow::MatToQImage(Mat &mat)
{
	if(mat.type()==CV_8UC3)
	{
		cvtColor(mat, mat, CV_BGR2RGB);
		QImage xxx = QImage((unsigned char*)(mat.data), mat.cols, mat.rows,mat.step, QImage::Format_RGB888);
		return xxx;
	}
	else if(mat.type()==CV_8U)
	{
		//对于8位整型单通道
		return QImage((const unsigned char*)(mat.data),mat.cols,mat.rows,mat.step,QImage::Format_Indexed8);
	}
	else
	{
		emit showWrongMessageSignal(QStringLiteral("MatToQImage 错误"));
		return QImage();
	}
}
Mat MainWindow::QImageToMat(QImage& image)
{
	if(image.depth()==24)
	{
		cv::Mat mat = cv::Mat(image.height(), image.width(), CV_8UC3, (uchar*)image.bits(), image.bytesPerLine());
		cvtColor(mat, mat, CV_RGB2BGR);
		return mat;
	}
	else if(image.depth()==8)
	{
		cv::Mat mat = cv::Mat(image.height(), image.width(), CV_8U, (uchar*)image.bits(), image.bytesPerLine());
		return mat;
	}
	else
	{
		emit showWrongMessageSignal(QStringLiteral("QImage To Mat 错误"));
		return Mat();
	}
}
void MainWindow::testSlot()
{

	QString disxx = dirPath+"/test.tif";
	if (createImage(m_testFile,disxx))
	{
		m_poDataset = (GDALDataset*)GDALOpen(disxx.toStdString().c_str(),GA_ReadOnly);
		if(m_poDataset == NULL)
		{
			emit showWrongMessageSignal(QStringLiteral("文件打开失败"));
			return;
		}

		GDALDataType xx = m_poDataset->GetRasterBand(1)->GetRasterDataType();

		//现阶段只支持Byte型数据

		if (xx == GDT_Byte)
		{
			t_type = t_uchar;
		}
		else if(xx == GDT_Float32||xx == GDT_Float64)
		{
			t_type = t_double;
		}
		else if(xx == GDT_Int16||xx == GDT_Int32||xx == GDT_Int16||xx == GDT_UInt32)
		{
			t_type = t_int;
		}
		else
		{
			emit showWrongMessageSignal(QStringLiteral("不支持数据类型"));
			return;
		}
		m_xsize = m_poDataset->GetRasterXSize();
		m_ysize = m_poDataset->GetRasterYSize();
		m_bandCount = m_poDataset->GetRasterCount();
		m_poDataset->GetGeoTransform(m_refAffine);
		const char* tempProj = m_poDataset->GetProjectionRef();
		m_projInfo = QString(m_poDataset->GetProjectionRef());

		//默认黑白
		int bandMap[3];
		bool isColor = false;
		if(m_bandCount>=3)
		{
			bandMap[0] = 1;
			bandMap[1] = 2;
			bandMap[2] = 3;
			isColor = true;
		}
		else
		{
			bandMap[0] = 1;
		}
		emit readImageSignal(isColor,bandMap);
	}
	else
	{
		showWrongMessageSignal("创建失败");
	}
}
void MainWindow::setColorSlot()
{
	if(isShow)
	{
		SetViewColorWidget* xx = new SetViewColorWidget(this,m_bandCount);

		if(xx->Accepted == xx->exec())
		{
			QVector<int> re = xx->getResults();
			bool isColor;
			int bandMap[3];
			if(re[0]==0)
			{
				isColor = true;
				bandMap[0] = re[1];
				bandMap[1] = re[2];
				bandMap[2] = re[3];
			}
			else
			{
				isColor = false;
				bandMap[0] = re[1];
			}
			emit readImageSignal(isColor,bandMap);
		}

	}
}
void MainWindow::fftSlot()
{
	if (m_showBands !=1)
	{
		emit showWrongMessageSignal(QStringLiteral("目前暂时只支持单波段Byte"));
		return;
	}
	fftwf_complex* in;
	int n0 = m_ysize;
	int n1 = m_xsize;
	in = new fftwf_complex[n0*n1];
	out = new fftwf_complex[n0*n1];
	double tempr = (-1);
	double tempc = -1;
	for(int i = 0;i<n0*n1;++i)
	{
		if (i%n1 == 0)
		{
			tempr*=(-1);
			tempc = tempr*(-1);
		}
		tempc*=-1;
		in[i][0] = tempc*static_cast<double>(buffer[i]);
		in[i][1] = 0;
	}
	fftwf_plan p = fftwf_plan_dft_2d(n0,n1,in,out,FFTW_FORWARD,FFTW_ESTIMATE);
	fftwf_execute(p);

	double* tempBuffer = new double[n0*n1];
	for(int i = 0;i<n0*n1;++i)
	{

		tempBuffer[i] = std::log10(std::sqrt(out[i][0]*out[i][0]+out[i][1]*out[i][1]));
	}
	//Mat xx2 = Mat(n0,n1,CV_8U,buffer);
	Mat xx = Mat(n0,n1,CV_64F,tempBuffer);
	normalize(xx, xx, 0, 255, CV_MINMAX);
	Mat xxx = Mat(n0,n1,CV_8U);
	xx.convertTo(xxx,CV_8U);

	m_showImg = MatToQImage(xxx);
	//m_scene->items().clear();
	imageWidget->setPixmap(QPixmap::fromImage(m_showImg,Qt::AutoColor));
	imageWidget->adjustSize();
	showImage(true);
	isFFt = true;
	delete[] in;
	//delete[] out;
	delete[] tempBuffer;
}
void MainWindow::nfftSlot()
{
	if (m_showBands !=1)
	{
		emit showWrongMessageSignal(QStringLiteral("目前暂时只支持单波段Byte"));
		return;
	}
	fftwf_complex* in;
	int n0 = m_ysize;
	int n1 = m_xsize;
	in = new fftwf_complex[n0*n1];
	//out = new fftwf_complex[n0*n1];

	fftwf_plan q = fftwf_plan_dft_2d(n0,n1,out,in,FFTW_BACKWARD,FFTW_ESTIMATE);
	fftwf_execute(q);
	BYTE* tempBuffer =new unsigned char[n0*n1];
	//tpd = -1;
	double tempr = (-1);
	double tempc = -1;
	for(int i = 0;i<n0*n1;++i)
	{
		if (i%n1 == 0)
		{
			tempr*=(-1);
			tempc = tempr*(-1);
		}
		tempc*=-1;
		tempBuffer[i] = static_cast<unsigned char>(tempc*in[i][0]/(n0*n1));
	}
	Mat xx3 = Mat(n0,n1,CV_8U,tempBuffer);
	m_showImg = MatToQImage(xx3);
	//m_scene->items().clear();
	//m_scene->addPixmap(QPixmap::fromImage(m_showImg,Qt::AutoColor));

	imageWidget->setPixmap(QPixmap::fromImage(m_showImg,Qt::AutoColor));
	imageWidget->adjustSize();
	showImage(true);
	isFFt =false;
	delete[] in;
	delete[] tempBuffer;
}
void MainWindow::showImage(bool isshow)
{
	emit showImageSignal(isshow);
}
void MainWindow::showImageSLot(bool isshow)
{
	isShow = isshow;
	ui->menuFFT->setEnabled(isShow);
	ui->actionHistogram_equalization->setEnabled(isShow);
	ui->actionHistogram_Match->setEnabled(isShow);
	ui->actionChannels->setEnabled(isShow);
}
void MainWindow::highPass()
{
	if (!isFFt)
	{
		emit showWrongMessageSignal(QStringLiteral("没有正变换"));
		return;
	}
	for (int i = 0;i<m_xsize*m_ysize;++i)
	{
		int r = i/m_xsize;
		int c = i%m_xsize;

		double dis = (r-m_ysize/2)*(r-m_ysize/2)+(c-m_xsize/2)*(c-m_ysize/2);
		double limitDis = m_xsize/50*m_xsize/50;
		if (dis < limitDis)
		{
			out[i][0] = 0;
			out[i][1] = 0;
		}
	}
}
void MainWindow::lowPass()
{
	if (!isFFt)
	{
		emit showWrongMessageSignal(QStringLiteral("没有正变换"));
		return;
	}
	for (int i = 0;i<m_xsize*m_ysize;++i)
	{
		int r = i/m_xsize;
		int c = i%m_xsize;

		double dis = (r-m_ysize/2)*(r-m_ysize/2)+(c-m_xsize/2)*(c-m_ysize/2);
		double limitDis = m_xsize/20*m_xsize/20;
		if (dis > limitDis)
		{
			out[i][0] = 0;
			out[i][1] = 0;
		}
	}
}
void MainWindow::HESlot()
{
	//int* histogram = new int[256];
	if (m_showBands==1)
	{
		vector<int> histogram(256,0);
		vector<unsigned char> lookforTable(256,0);
		for (int i=0;i<m_ysize;++i)
		{
			for (int j = 0;j<m_xsize;++j)
			{
				histogram[buffer[i*m_xsize+j]]++;
			}
		}
		int pointCount = m_xsize*m_ysize;

		for (int i = 1;i < 256;++i)
		{
			if (i!=0)
			{
				histogram[i]+=histogram[i-1];
			}
			lookforTable[i] = static_cast<unsigned char>((double)histogram[i]/pointCount*255.0);
		}
		for (int i=0;i<m_ysize;++i)
		{
			for (int j = 0;j<m_xsize;++j)
			{
				buffer[i*m_xsize+j]=lookforTable[buffer[i*m_xsize+j]];
			}
		}
		m_showImg = QImage(buffer,m_xsize,m_ysize,m_xsize,QImage::Format_Indexed8);
		//m_scene->items().clear();
		//m_scene->addPixmap(QPixmap::fromImage(m_showImg,Qt::AutoColor));
		imageWidget->setPixmap(QPixmap::fromImage(m_showImg,Qt::AutoColor));
		imageWidget->adjustSize();
		showImage(true);
	}
	else
	{
		for(int chanel=0;chanel<3;chanel++)
		{
			vector<int> histogram(256,0);
			vector<unsigned char> lookforTable(256,0);
			for (int i=0;i<m_ysize;++i)
			{
				for (int j = 0;j<m_xsize;++j)
				{
					histogram[buffer[i*m_xsize*3+j*3+chanel]]++;
				}
			}
			int pointCount = m_xsize*m_ysize;

			for (int i = 1;i < 256;++i)
			{
				if (i!=0)
				{
					histogram[i]+=histogram[i-1];
				}
				lookforTable[i] = static_cast<unsigned char>((double)histogram[i]/pointCount*255.0);
			}
			for (int i=0;i<m_ysize;++i)
			{
				for (int j = 0;j<m_xsize;++j)
				{
					buffer[i*m_xsize*3+j*3+chanel]=lookforTable[buffer[i*m_xsize*3+j*3+chanel]];
				}
			}
			m_showImg = QImage(buffer,m_xsize,m_ysize,m_xsize*3,QImage::Format_RGB888);
			//m_scene->items().clear();
			//m_scene->addPixmap(QPixmap::fromImage(m_showImg,Qt::AutoColor));
			imageWidget->setPixmap(QPixmap::fromImage(m_showImg,Qt::AutoColor));
			imageWidget->adjustSize();
			showImage(true);
		}
	}
	//Mat xx3 = Mat(m_xsize,m_ysize,CV_8U,buffer);
}
void MainWindow::saveAS()
{
	if (!isShow)
	{
		showWrongMessageSignal("没有图像显示");
		return;
	}
	QString path   = QFileDialog::getSaveFileName(this,QStringLiteral("保存显示文件"),"/",
		"Image Files(*.jpg)");
	if(path == "")
	{
		qDebug()<<"the File Name is Null";
		emit showWrongMessageSignal(QStringLiteral("没有选择文件"));
		return;
	}

	m_showImg.save(path);
}