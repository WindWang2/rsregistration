#ifndef VIEWWIDGET_H
#define VIEWWIDGET_H

#include <QMainWindow>

#include "gdal_priv.h"
class GDALDataset;
class QListWidgetItem;
struct URect
{
	URect(int ix,int iy,int iwidth,int iheight)
	{
		x = ix;
		y=iy;
		width=iwidth;
		height = iheight;
	}
	URect()
	{
		x = 0;
		y = 0;
		width = 0;
		height = 0;
	}
	int x;
	int y;
	int width;
	int height;

};
struct URectF
{
	URectF(double ix,double iy,double iwidth,double iheight)
	{
		x = ix;
		y = iy;
		width = iwidth;
		height = iheight;
	}
	URectF()
	{
		x = 0;
		y = 0;
		width = 0;
		height = 0;

	}
	double x;
	double y;
	double width;
	double height;
};
class ViewWidget:public QWidget
{
	Q_OBJECT
public:
	ViewWidget(QWidget* parent);

public:
	void setFilePath(QString FilePath);
	//virtual void paint(QPinter* p);
	bool openFile();
	void clearnPaint();
	//void 
	void CreateShowImage();
	//void calculateDesSize();
	URect GetIntersectRect(URect);
	void getStartClick();
	void changeCurrseToNormal();



public slots:

	void readImage();
	//void Histogram_Equalization();
	void setViewColor(QVector<int>);
	void addClickedPoint(int,int);
	void delClickedPoint(int);
	void zoomToSelectedPoint(QListWidgetItem*);

signals:
	void sendClickPoint(int x,int y,int* data);
protected:
	void paintEvent(QPaintEvent *);
	void resizeEvent(QResizeEvent*);
	void mouseMoveEvent(QMouseEvent *);
	void mousePressEvent(QMouseEvent *);
	void mouseReleaseEvent(QMouseEvent *);
	//void mousePressEvent(QMouseEvent *);
	void wheelEvent(QWheelEvent *);
	void draw(QPainter*,QPointF);
	QString m_soureFileName;

	GDALDataset* m_pDataset;
	
	QSize m_oriImageSize;

	URect m_readImageRect;

	URect m_desImageRect;

	double m_scale;
	int m_dx;
	int m_dy;
	QPointF m_oldPoint;
	QPointF m_nowPoint;
	QPoint m_showImagePoint;

	QImage m_image;
	//QImage m_showImage;
	
	bool m_isOnMidButton;
	bool m_isHistogram_Equalization;
	QImage m_tempImage;
	QPoint m_tempShowImagePoint;
	URect m_nowImageRect;
	//QPixmap m_px;
	//URect m_nowImageRect;
	GDALDataType m_type;
	QVector<QRgb>  colorTable;
	int nRst[256];
	double maxMin[2];
	bool m_isOnBand;
	double m_count;
	bool m_isOnClick;
	bool m_hasImage;
	unsigned char* pafScan2;

	QVector<int> nowShowStyle;
	QVector<QPoint> m_clickPoint;
};



#endif



