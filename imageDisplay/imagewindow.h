#ifndef IMAGEWINDOW_H
#define IMAGEWINDOW_H

#include <QMainWindow>

QT_BEGIN_NAMESPACE
class QAction;
class QListWidget;
class QMenu;
class QGraphicsView;
class QLabel;
class QGraphicsScene;
class GDALDataset;
class ViewWidget;
class QPixmap;
class QListWidgetItem;
struct PointList;
//class QTextEdit;
QT_END_NAMESPACE

class ImageWindow : public QMainWindow
{
    Q_OBJECT
public:
    ImageWindow();
    
signals:

	void setImageView(int bandCount);
    void startClick();
	void changeViewColor(QVector<int>);

	void sendToViewClickButtonOk(int x,int y);
	void sendToViewClickButtonFail();
	void writeToFile();
	void showItem();
	void delItem(int);
private slots:
    void inputImage();
    void inputXY();
    void outputResult();
    void selectBand();
    void about();
    void pan();
    void select();
    void normal();
	void setText(QString);
	void setFile(QListWidgetItem*);
	void setView(int bandCount);
	void setViewClick();
	void changeItemStatus(QString);
	void createPyramidfailed(QString);
	void clickPoint();
	void changeCur();
	//void labelChanged();
	//void aboutQt();
	void getClickPoint(int,int,int*);
	void writeToFileSlot();
	void showItemSlot();
	void oparetItem(QListWidgetItem*);
protected:
	//void resizeEvent(QResizeEvent *);
private:
    void createActions();
    void createMenu();
    void createToolBars();
    //void createStatusBar();
    void createDockWindows();
	void openFile();
	void ShowRaster();
	int buildOverViews();
	int getNowFileBandCount();
	//test
	QVector<double*> getRasterbandMaxMin();

private:
	//QGraphicsView* view;
	//QGraphicsScene* scene;

	//QGraphicsView* zoomView;
    //QListWidget*

	ViewWidget* imageView;
	QLabel* zoomView;
	QMenu* imageMenu;
	QMenu* xyMenu;
	QMenu* imageProcessMenu;
	QMenu* helpMenu;

	QToolBar* viewToolBar;
	//QToolBar* ClickPointBar;

	QAction* inputImageAct;
	QAction* inputXYAct;
	QAction* outputReAct;
	QAction* selectBandAct;
	QAction* aboutAct;
	QAction* aboutQtAct;
	QAction* quitAct;
	QAction* setViewAct;
	QAction* clickButtonAct;
	QAction* curChangeToNorAct;
	//QList
	QListWidget* imageListWidget;
	QListWidget* xyListWidget;
	QListWidget* pointListWidget;

	//
	double m_scale;
	int m_imageXsize;
	int m_imageYsize;
	int m_imageOriginXsize;
	int m_imageOriginYsize;
	GDALDataset* m_pDataset;



	/************************************************************************/
	/* 跟随图像变换的全局变量                                                                     */
	/************************************************************************/
	int m_bandCount;
	QVector<PointList> m_PointItem;
	int m_flag;
	QString m_ImagePath;

};
struct PointList
{
	PointList(uint iindex=0,double iX= 0,double iY =0,double iZ = 0,int ix = 0,int iy = 0,double ixoffset = 0,double iyoffset = 0):
		index(iindex),X(iX),Y(iY),Z(iZ),x(ix),y(iy),xoffset(ixoffset),yoffset(iyoffset)
	{
		
	}
	double X;
	double Y;
	double Z;

	int x;
	int y;
	
	double xoffset;
	double yoffset;

	uint index;
};

#endif // IMAGEWINDOW_H
