#include <QtWidgets>
#include "imagewindow.h"
#include "gdal_priv.h"
#include "ViewWidget.h"
#include "setviewcolorwidget.h"
#include "createraseterpyramids.h"
#include "InputPointWidget.h"
#include "OperatDialog.h"
//#include "resource.h"
ImageWindow::ImageWindow()
{
    GDALAllRegister();
	CPLSetConfigOption("USE_RRD","YES");
	//CPLSetConfigOption("TIF_USE_OVR","TRUE");
	//view = new QGraphicsView;
	//scene = new QGraphicsScene;
	//view->setScene(scene);
	imageView = new ViewWidget(this); 
	zoomView = new QLabel;
	setCentralWidget(imageView);
	
	//setWindowState(Qt::WindowMaximized);
	setGeometry(150,150,1000,800);

	imageView->setStyleSheet(QStringLiteral("background-color: rgb(157, 157, 157);"));


	//imageView->setMinimumSize(600,600);
	createActions();
	createMenu();
	createToolBars();
	createDockWindows();
	QString windowTitle = QStringLiteral("优化刺点程序");
	setWindowTitle(windowTitle);



	connect(this,SIGNAL(setImageView(int)),this,SLOT(setView(int)));
	connect(imageView,SIGNAL(sendClickPoint(int,int,int*)),this,SLOT(getClickPoint(int,int,int*)));
	connect(this,SIGNAL(writeToFile()),this,SLOT(writeToFileSlot()));
	connect(this,SIGNAL(showItem()),this,SLOT(showItemSlot()));
	connect(this,SIGNAL(sendToViewClickButtonOk(int,int)),imageView,SLOT(addClickedPoint(int,int)));
	connect(this->pointListWidget,SIGNAL(itemDoubleClicked(QListWidgetItem*)),this,SLOT(oparetItem(QListWidgetItem*)));
	connect(this,SIGNAL(delItem(int)),imageView,SLOT(delClickedPoint(int)));
	/************************************************************************/
	/* 测试                                                                     */
	/************************************************************************/
	//m_bandCount->
	//imageView->setFilePath(m_ImagePath);
	//buildOverViews();	
	//openFile();
	//ShowRaster();
	/************************************************************************/
	//m_ImagePath = QStringLiteral("D:\\test\\test2.tif");


}
/************************************************************************/
/*程序构造                                                                     */
/************************************************************************/
void ImageWindow::createActions()
{

	aboutAct = new QAction(QStringLiteral("&About"),this);
	aboutAct->setStatusTip("Show the application's About box");
	connect(aboutAct, SIGNAL(triggered()), this, SLOT(about()));

	aboutQtAct = new QAction(QStringLiteral("About &Qt"),this);
	aboutQtAct->setStatusTip((QStringLiteral("SHow the Qt Library's About box")));
	connect(aboutQtAct,SIGNAL(triggered()),qApp,SLOT(aboutQt()));

    inputImageAct = new QAction(QStringLiteral("加载影像"),this);
	inputImageAct->setStatusTip((QStringLiteral("加载影像")));
    connect(inputImageAct,SIGNAL(triggered()),this,SLOT(inputImage()));

	setViewAct = new QAction(QStringLiteral("设置显示颜色"),this);
	connect(setViewAct,SIGNAL(triggered()),this,SLOT(setViewClick()));

	clickButtonAct = new QAction(QIcon(":/click.ico"),QStringLiteral("刺点"),this);
	connect(clickButtonAct,SIGNAL(triggered()),this,SLOT(clickPoint()));

	curChangeToNorAct = new QAction(QIcon(":/cross.png"),QStringLiteral("浏览"),this);
	connect(curChangeToNorAct,SIGNAL(triggered()),this,SLOT(changeCur()));
}
void ImageWindow::createMenu()
{
	imageMenu = menuBar()->addMenu(QStringLiteral("&Image"));
    imageMenu->addAction(inputImageAct);
	imageMenu->addAction(setViewAct);

	xyMenu = menuBar()->addMenu(QStringLiteral("&XY"));

    //xyMenu->addAction(selectBandAct);
	imageProcessMenu = menuBar()->addMenu(QStringLiteral("Image &Process"));

	helpMenu = menuBar()->addMenu((QStringLiteral("Help")));
	helpMenu->addAction(aboutAct);
	helpMenu->addAction(aboutQtAct);


}
void ImageWindow::createToolBars()
{
	viewToolBar = addToolBar(QStringLiteral("刺点"));
	viewToolBar->addAction(clickButtonAct);
	viewToolBar->addAction(curChangeToNorAct);
}
void ImageWindow::setFile(QListWidgetItem* item)
{
	m_ImagePath = item->text();
	imageView->setFilePath(item->text());
	m_flag = 0;
	m_PointItem.clear();
}
void ImageWindow::setText(QString xx)
{
	//QString xx = this->imageListWidget->
	QMessageBox::about(this,tr(""),xx);
}
void ImageWindow::createDockWindows()
{
	QDockWidget* dock = new QDockWidget(QStringLiteral("图像列表"),this);
	dock->setAllowedAreas(Qt::LeftDockWidgetArea|Qt::RightDockWidgetArea);
	imageListWidget = new QListWidget;
	dock->setWidget(imageListWidget);
	addDockWidget(Qt::RightDockWidgetArea,dock);
	//connect(imageListWidget,SIGNAL(currentTextChanged(QString)),this,SLOT(setText(QString)));
	connect(imageListWidget, SIGNAL(itemDoubleClicked(QListWidgetItem*)), this, SLOT(setFile(QListWidgetItem*)));  
	
	dock = new QDockWidget(QStringLiteral("输入数据点号"),this);
	dock->setAllowedAreas(Qt::LeftDockWidgetArea|Qt::RightDockWidgetArea);
	xyListWidget = new QListWidget;
	dock->setWidget(xyListWidget);
	addDockWidget(Qt::RightDockWidgetArea,dock);

	dock = new QDockWidget(QStringLiteral("缩放窗口"),this);
	dock->setAllowedAreas(Qt::BottomDockWidgetArea);
	//zoomView = new QGraphicsView;
	dock->setWidget(zoomView);
	addDockWidget(Qt::BottomDockWidgetArea,dock);

	dock = new QDockWidget(QStringLiteral("刺点结果"),this);
	dock->setAllowedAreas(Qt::BottomDockWidgetArea);
	pointListWidget = new QListWidget;
	dock->setWidget(pointListWidget);
	addDockWidget((Qt::BottomDockWidgetArea),dock);



	connect(this,SIGNAL(changeViewColor(QVector<int>)),imageView,SLOT(setViewColor(QVector<int>)));
	
}

/************************************************************************/
/* slot                                                                     */
/************************************************************************/
void ImageWindow::inputImage()
{
	QStringList files = QFileDialog::getOpenFileNames(this, QStringLiteral("选择影像数据"),
	"D:\\","IMAGE(*.tif)");
	if (files.isEmpty())
	{
		return;
	}
	for (int i = 0;i<files.length();++i)
	{
        QListWidgetItem* xx =new QListWidgetItem(QIcon("://no.png"),files[i]);
		imageListWidget->addItem(xx);
	}
	CreateRaseterPyramids* xx = new CreateRaseterPyramids(files,this);
	xx->start();

	connect(xx,SIGNAL(CreatePyramidsFinished(QString)),this,SLOT(changeItemStatus(QString)));
	connect(xx,SIGNAL(CreatePyramidsFailed(QString)),this,SLOT(createPyramidfailed(QString)));
}
void ImageWindow::inputXY()
{}
void ImageWindow::outputResult()
{}
void ImageWindow::selectBand()
{}
void ImageWindow::about()
{
	QMessageBox::about(this, tr("About Dock Widgets"),
		tr("<b>ClickPoint<\b> v1.0.0"));
	imageView->setFilePath(tr("D:\\test\\test2.tif"));
}
void ImageWindow::pan()
{}
void ImageWindow::select()
{}
void ImageWindow::normal()
{}
void ImageWindow::setViewClick()
{
	emit setImageView(getNowFileBandCount());
}
void ImageWindow::setView(int bandCount)
{
	SetViewColorWidget dialog(this,bandCount);
	QVector<int> xx;
	if (dialog.exec())
	{
		xx = dialog.getResults();
	}
	emit changeViewColor(xx);
}
void ImageWindow::changeItemStatus(QString xx)
{
	int imageCount = this->imageListWidget->count();
	for (int j = 0;j<imageCount;j++)
	{
		if (imageListWidget->item(j)->text() == xx)
		{
			imageListWidget->item(j)->setIcon(QIcon("ok.png"));
			break;
		}
	}
}
void ImageWindow::createPyramidfailed(QString xx)
{
	QMessageBox::warning(this,QStringLiteral("创建金字塔错误"),xx);
}
void ImageWindow::clickPoint()
{
	imageView->getStartClick();
}
void ImageWindow::changeCur()
{
	imageView->changeCurrseToNormal();
}
double calOffset(int a,int b,int mid)
{
	double re = 0;
	if (mid	 == a)
	{
		re = -0.5;
	}
	else if (mid == b)
	{
		re = 0.5;
	}
	else
	{
		re = ((double)b-(double)mid)/((double)(a+b-2*mid));
		if (re>0.5)
		{
			re = re-0.5;
		}
		else if(re<=0.5)
		{
			re = -(a-0.5);
		}
	}
	return re;
}
void ImageWindow::getClickPoint(int x,int y,int* data)
{
	InputPointWidget xx2(0,this);
	int xx [3*3];
	for (int i = 0;i<9;i++)
	{
		xx[i] = data[i];
	}
	if(xx2.exec())
	{
		emit sendToViewClickButtonOk(x,y);
		QVector<double> xyz = xx2.getInputData();
		double xoffset = calOffset(data[3],data[5],data[4]);
		double yoffset = calOffset(data[1],data[7],data[4]);

		PointList po(m_flag,xyz[0],xyz[1],xyz[2],x,y,xoffset,yoffset);
		m_PointItem.append(po);
		m_flag++;
		emit writeToFile();
		emit showItem();
	}
	
}

void ImageWindow::writeToFileSlot()
{
	QFile file(m_ImagePath+".txt");
	if (file.exists())
	{
		file.resize(0);
	}
	if (file.open(QIODevice::WriteOnly))
	{
		QTextStream out(&file);
		for(int i = 0;i<m_PointItem.count();++i)
		{
			out<<i<<"	";
			out<<m_PointItem[i].x<<"	";
			out<<m_PointItem[i].y<<"	";
			out<<(m_PointItem[i].X+m_PointItem[i].xoffset)<<"	";
			out<<(m_PointItem[i].Y+m_PointItem[i].yoffset)<<"	";
			out<<m_PointItem[i].Z<<"\r\n";
		}
		file.close();
	}
	else
	{
		QMessageBox::warning(this,QStringLiteral("错误"),QStringLiteral("创建或打开文件错误"));
		return;
	}
}
void ImageWindow::showItemSlot()
{
	pointListWidget->clear();
	for(int i = 0;i<m_PointItem.count();++i)
	{
		QListWidgetItem* xx = new QListWidgetItem(QString::number(i)+" "+
			QString::number(m_PointItem[i].x)+" "+
			QString::number(m_PointItem[i].y)+" "+
			QString::number(m_PointItem[i].X)+" "+
			QString::number(m_PointItem[i].Y)+" "+
			QString::number(m_PointItem[i].Z)+" "+
			QString::number(m_PointItem[i].xoffset)+" "+
			QString::number(m_PointItem[i].yoffset));
		pointListWidget->addItem(xx);
	}
}
void ImageWindow::oparetItem(QListWidgetItem* item)
{
	int index = item->listWidget()->currentRow();
	OperatDialog xx(this);
	int re = 1;
	if (xx.exec())
	{
		re = xx.getCheckedOption();
	}
	switch(re)
	{
	case 1:
		{
			InputPointWidget xx2(0,this);
			QVector<double> a;
			if (xx2.exec())
			{
				a = xx2.getInputData();
				m_PointItem[index].X = a[0];
				m_PointItem[index].Y = a[1];
				m_PointItem[index].Z = a[2];
			}
		}
		break;
	case 2:
		{
			m_PointItem.remove(index);
			emit delItem(index);
		}
		break;
	case 3:
		{
			imageView->zoomToSelectedPoint(item);
		}
		break;
	default:
		break;
	}
	emit writeToFile();
	emit showItem();
}
//double a
//void ImageWindow::aboutQt()
//{
//
//}

/************************************************************************/
/* 实现函数部分                                                                     */
/************************************************************************/
void ImageWindow::openFile()
{
	//const char* filePath2 = ;
	m_pDataset = (GDALDataset*)GDALOpen(m_ImagePath.toStdString().data(),GA_Update);
	if(m_pDataset==NULL)
	{
		QMessageBox::warning(this,QStringLiteral("警告"),QStringLiteral("打开文件失败")
			,QMessageBox::Ok|QMessageBox::Cancel);
		return;
	}	

}
void ImageWindow::ShowRaster()
{
	
}
int ImageWindow::buildOverViews()
{
	GDALDataset* pDataset = (GDALDataset*)GDALOpen(m_ImagePath.toStdString().data(),GA_ReadOnly);
	GDALDriver* Driver = pDataset->GetDriver();
	if(pDataset == NULL)
	{
		return -1;
	}
	int iWidth = pDataset->GetRasterXSize();
	int iHeight = pDataset->GetRasterYSize();


	int iPixelNum = iWidth*iHeight;
	//顶层金字塔大小，64*64
	int iTopNum = 4096;
	//存储每一次个像元总数。当前为第一次
	int iCurNum = iPixelNum/4;

	int anLevels[1024]={0};
	int nLevelCount = 0;

	do 
	{
		anLevels[nLevelCount] = static_cast<int>(pow(2.0,nLevelCount+2));
		nLevelCount++;
		iCurNum /= 4;
	} while (iCurNum > iTopNum);
	const char* pszResampling="nearest";
	//GDALProgressFunc pfnProgress = GDALProgress;
	
	 CPLErr xx = pDataset->BuildOverviews(pszResampling,
		nLevelCount,anLevels,0,NULL,NULL,NULL);
	GDALClose(pDataset);
	return 1;
}
int ImageWindow::getNowFileBandCount()
{
	GDALDataset* pDataset = (GDALDataset*)GDALOpen(m_ImagePath.toStdString().data(),GA_ReadOnly);
	int x = pDataset->GetRasterCount();
	if (x == 0)
	{
		QMessageBox::warning(this,QStringLiteral("错误"),QStringLiteral("获取文件波段数失败"));
		return 0;
	}
	GDALClose(pDataset);
	return x;
}
QVector<double*> ImageWindow::getRasterbandMaxMin()
{
	QVector<double*> MaxMinList;
	double xx[2]={0.0,255.0};
	GDALDataset* pDataset = (GDALDataset*)GDALOpen(m_ImagePath.toStdString().data(),GA_ReadOnly);
	int rasterBand=pDataset->GetRasterCount();
	for (int i = 0;i<rasterBand;++i)
	{
		CPLErr error = pDataset->GetRasterBand(i+1)->ComputeRasterMinMax(TRUE,xx);
		MaxMinList.append(xx);		
	}
	return MaxMinList;
}
