#include <QBitmap>
#include <QPushButton>
#include <QGroupBox>
#include <QPainter>
#include <QPainterPath>
#include <QPixmapCache>
#include <QtEvents>
#include <QTextDocument>
#include <QAbstractTextDocumentLayout>
#include <QFile>
#include <QTextBrowser>
#include <QBoxLayout>
#include <QMessageBox>
#include <QListWidget>
#include "gdal_priv.h"
#include "ViewWidget.h"

typedef unsigned char* BYTE;
ViewWidget::ViewWidget(QWidget* parent):QWidget(parent)
{
	/************************************************************************/
	/* 类成员初始定义                                                                     */
	/************************************************************************/
	GDALAllRegister();
	CPLSetConfigOption("USE_RRD","YES");
	CPLSetConfigOption("TIF_USE_OVR","TRUE");

	for(int k=0;k<256;++k) 
	{
		colorTable.push_back( qRgb(k,k,k) );

	}
	m_image.setColorTable(colorTable);

	m_hasImage = false;
}

void ViewWidget::setFilePath(QString FilePath)
{
	m_soureFileName = FilePath;
	m_isOnMidButton = false;
	m_isHistogram_Equalization = false;
	m_isOnBand = false;
	m_hasImage = true;
	m_count = 0;
	m_isOnClick = false;
	nowShowStyle.append(1);
	nowShowStyle.append(1);
	this->setCursor(Qt::ArrowCursor);
	pafScan2 = nullptr;
	//////////////////////////////////////////////////////////////////////////
	openFile();
	readImage();
	update();
	//double x = 0;
}
bool ViewWidget::openFile()
{
	m_pDataset = (GDALDataset*)GDALOpen(
		m_soureFileName.toStdString().data(),GA_ReadOnly);
	m_oriImageSize = QSize(m_pDataset->GetRasterXSize(),
		m_pDataset->GetRasterYSize());
	m_type = m_pDataset->GetRasterBand(1)->GetRasterDataType();
	double a_scale = static_cast<int>(((double)m_oriImageSize.width())/this->width());
	double b_scale = static_cast<int>(((double)m_oriImageSize.height())/this->height());
	m_scale = a_scale>b_scale?a_scale:b_scale;
	/************************************************************************/
	/* 测试                                                                     */
	/************************************************************************/
	m_showImagePoint = QPoint(0,0);

	m_nowImageRect.x = 0;
	m_nowImageRect.y = 0;
	m_nowImageRect.height = static_cast<int>(m_oriImageSize.height()/m_scale+0.5);
	m_nowImageRect.width  = static_cast<int>(m_oriImageSize.width()/m_scale+0.5);
	m_desImageRect = GetIntersectRect(m_nowImageRect);
	m_readImageRect.x = (static_cast<int>(m_desImageRect.x*m_scale+0.5));
	m_readImageRect.y = (static_cast<int>(m_desImageRect.y*m_scale+0.5));
	m_readImageRect.width = (static_cast<int>(m_desImageRect.width*m_scale+0.5));
	m_readImageRect.height = (static_cast<int>(m_desImageRect.height*m_scale+0.5));
	
	if (m_readImageRect.x+m_readImageRect.width>m_oriImageSize.width())
	{
		m_readImageRect.x = m_oriImageSize.width()-m_readImageRect.width;
		if (m_readImageRect.x<0)
		{
			m_readImageRect.x=0;
		}
		if (m_readImageRect.width>m_oriImageSize.width())
		{
			m_readImageRect.width=m_oriImageSize.width();
		}
	}
	if (m_readImageRect.y+m_readImageRect.height>m_oriImageSize.height())
	{
		m_readImageRect.y = m_oriImageSize.height()-m_readImageRect.height;
		if (m_readImageRect.y<0)
		{
			m_readImageRect.y=0;
		}
		if (m_readImageRect.height>m_oriImageSize.height())
		{
			m_readImageRect.height=m_oriImageSize.height();
		}
	}
	CPLErr error = m_pDataset->GetRasterBand(1)->ComputeRasterMinMax(TRUE,maxMin);

	//if (m_isOnBand)
	//{

	//	m_pDataset->GetRasterBand(1)->GetHistogram(maxMin[0],maxMin[1],256,nRst,false,true,NULL,NULL);

	//	for (int i = 0;i<256;++i)
	//	{
	//		m_count+=nRst[i];
	//		if (i>0)
	//		{
	//			nRst[i]+=nRst[i-1];
	//		}
	//	}
	//}
	return true;
}

void ViewWidget::readImage()
{
	if (pafScan2 != 0)
	{
		delete pafScan2;
	}
	
	if (nowShowStyle[0]==0)
	{
		//delete m_image.data_ptr();
		if (m_type == GDT_Byte)
		{
			int iSize = GDALGetDataTypeSize(GDT_Byte) / 8;
			int bandCount = 3;
			int bandmap[3] = {nowShowStyle[1],nowShowStyle[2],nowShowStyle[3]};
			pafScan2 = new unsigned char[m_desImageRect.width*m_desImageRect.height*bandCount];
			CPLErr er = m_pDataset->RasterIO(GF_Read,m_readImageRect.x,m_readImageRect.y,m_readImageRect.width,
				m_readImageRect.height,pafScan2,m_desImageRect.width,m_desImageRect.height,
				GDT_Byte,bandCount,bandmap,iSize*bandCount,iSize*m_desImageRect.width*bandCount,iSize);
			m_image = QImage(pafScan2,m_desImageRect.width,m_desImageRect.height,m_desImageRect.width*3,QImage::Format_RGB888);
			//delete []pafScan;
		}
		else
		{
			QMessageBox::warning(this,QStringLiteral("错误"),QStringLiteral("暂时不支持非byte数据"),QMessageBox::Ok);
			nowShowStyle[0]=1;
			nowShowStyle[0]=1;
		}
	}
	else
	{
		//delete m_image;
		//m_image.~QImage();


		int iSize = GDALGetDataTypeSize(m_type)/8;
		int bandCount = 1;
		int* pafScan = new int[m_desImageRect.width*m_desImageRect.height];
		CPLErr er = m_pDataset->GetRasterBand(nowShowStyle[1])->RasterIO(GF_Read,m_readImageRect.x,m_readImageRect.y,m_readImageRect.width,
			m_readImageRect.height,pafScan,m_desImageRect.width,m_desImageRect.height,
			GDT_Int32,0,0);
		pafScan2 = new unsigned char[m_desImageRect.width*m_desImageRect.height];
		int xxxxxxx= static_cast<int>(maxMin[1]-(maxMin[1]-maxMin[0])*0.1);

		for (int i = 0;i<m_desImageRect.width*m_desImageRect.height;++i)
		{
			if ((pafScan[i])<xxxxxxx)
			{

				pafScan2[i] = static_cast<unsigned char>(((double)pafScan[i]-maxMin[0])/(xxxxxxx-maxMin[0])*255.+0.5);

			}else
			{

				pafScan2[i]=static_cast<unsigned char>(254);

			
			}
		}
		delete []pafScan;
		m_image = QImage(pafScan2,m_desImageRect.width,m_desImageRect.height,m_desImageRect.width,QImage::Format_Indexed8);
		//double xx = 5;
		//delete[] pafScan2;
	}

	//
	//if (m_isHistogram_Equalization&&m_isOnBand)
	//{
	//	for (int i = 0;i<m_image.width()*m_image.height();++i)
	//	{
	//		int xx = m_image.bits()[i];
	//		m_image.bits()[i] = static_cast<unsigned char>(nRst[m_image.bits()[i]]/m_count*255.+0.5);
	//	}
	//}

}
void ViewWidget::paintEvent(QPaintEvent * event)
{
	
	QImage image = QImage(size(),QImage::Format_ARGB32_Premultiplied);
	QSize xx = size();
	QPainter imagePainter(&image);
	imagePainter.initFrom(this);
	imagePainter.setRenderHint(QPainter::Antialiasing,true);
	imagePainter.eraseRect(QRect());
	draw(&imagePainter,m_showImagePoint);
	imagePainter.end();
	
	QPainter widgetPainter(this);
	//
	widgetPainter.drawImage(0,0,image);
}
void ViewWidget::draw(QPainter* painter,QPointF pt)
{
	painter->drawImage(pt,m_image);
	for (int i = 0;i<m_clickPoint.count();++i)
	{
		if ((m_clickPoint[i].x()<=m_readImageRect.x+m_readImageRect.width)&&
			(m_clickPoint[i].x()>=m_readImageRect.x))
		{
			if ((m_clickPoint[i].y()<=m_readImageRect.y+m_readImageRect.height)&&(
				m_clickPoint[i].y()>=m_readImageRect.y))
			{
				double newHeightScale = ((double)m_readImageRect.height)/m_desImageRect.height;
				double newWidthScale = ((double)m_readImageRect.width)/m_desImageRect.width;

				double tempx = (m_clickPoint[i].x()-m_readImageRect.x)/newWidthScale;
				double tempy = (m_clickPoint[i].y()-m_readImageRect.y)/newHeightScale;

				float px = tempx+m_showImagePoint.x()+1/2./newWidthScale;
				float py = tempy+m_showImagePoint.y()+1/2./newHeightScale;

				painter->setPen(QPen(Qt::red,3));
				painter->drawEllipse(QPointF(px,py),10,10);
				painter->setPen(QPen(Qt::red,1));
				painter->drawLine(QPointF(px,py-10),QPointF(px,py+10));
				painter->drawLine(QPointF(px-10,py),QPointF(px+10,py));
				painter->drawText(QRectF(px-10,py+12,20,20),Qt::AlignHCenter|Qt::AlignVCenter,QString::number(i));

			}
		}
	}

}
void ViewWidget::CreateShowImage()
{
	
	m_dx += m_nowPoint.x()-m_oldPoint.x();
	m_dy += m_nowPoint.y()-m_oldPoint.y();

	if (m_tempShowImagePoint.x()+m_dx+m_desImageRect.width<0)
	{
		m_dx -= m_nowPoint.x()-m_oldPoint.x();
	}
	if (m_tempShowImagePoint.y()+m_dy+m_desImageRect.height<0)
	{
		m_dy -= m_nowPoint.y()-m_oldPoint.y();
	}
	if (m_tempShowImagePoint.x()+m_dx>this->width())
	{
		m_dx -= m_nowPoint.x()-m_oldPoint.x();
	}
	if (m_tempShowImagePoint.y()+m_dy>this->height())
	{
		m_dy -= m_nowPoint.y()-m_oldPoint.y();
	}
	double xcond1 = m_dx + m_tempShowImagePoint.x();
	double xcond2 = m_dx + m_tempShowImagePoint.x()+m_desImageRect.width;

	double ycond1 = m_dy + m_tempShowImagePoint.y();
	double ycond2 = m_dy + m_tempShowImagePoint.y()+m_desImageRect.height;

	if (ycond1 <= 0)
	{
		if (xcond1 <=0)
		{
			double tx = abs(m_dx)-m_tempShowImagePoint.x();
			double ty = abs(m_dy)-m_tempShowImagePoint.y();
			m_showImagePoint.setX(0);
			m_showImagePoint.setY(0);

			m_image = m_tempImage.copy(tx,ty,m_desImageRect.width-tx,m_desImageRect.height-ty);
		}
		else if(xcond2 >(-m_desImageRect.width) && xcond2 <= this->width())
		{
			//int tx = abs(m_dx)-m_tempShowImagePoint.x();
			double ty = abs(m_dy)-m_tempShowImagePoint.y();
			m_showImagePoint.setX(m_dx+m_tempShowImagePoint.x());
			m_showImagePoint.setY(0);

			m_image = m_tempImage.copy(0,ty,m_desImageRect.width,m_desImageRect.height-ty);
		}
		else
		{
			double ty = abs(m_dy)-m_tempShowImagePoint.y();

			m_showImagePoint.setX(m_dx+m_tempShowImagePoint.x());
			m_showImagePoint.setY(0);

			m_image = m_tempImage.copy(0,ty,this->width()-(m_dx+m_tempShowImagePoint.x()),m_desImageRect.height-ty);
		}
	}
	else if (ycond2 >(-m_desImageRect.height) && ycond2 <=this->height())
	{
		if (xcond1 <=0)
		{
			m_showImagePoint.setX(0);
			m_showImagePoint.setY(m_dy+m_tempShowImagePoint.y());

			m_image = m_tempImage.copy(abs(m_dx)-m_tempShowImagePoint.x(),0,
				m_desImageRect.width-(abs(m_dx)-m_tempShowImagePoint.x()),m_desImageRect.height);
		}
		else if(xcond2 >(-m_desImageRect.width) && xcond2 <= this->width())
		{
			m_showImagePoint.setX(m_dx+m_tempShowImagePoint.x());
			m_showImagePoint.setY(m_dy+m_tempShowImagePoint.y());
			m_image = m_tempImage;
		}
		else
		{
			//int dx = abs(m_dx)-m_tempShowImagePoint.x();
			m_showImagePoint.setX(m_dx+m_tempShowImagePoint.x());
			m_showImagePoint.setY(m_dy+m_tempShowImagePoint.y());

			m_image = m_tempImage.copy(0,0,this->width()-(m_dx+m_tempShowImagePoint.x()),m_desImageRect.height);
		}
	}
	else if (ycond2 > this->height())
	{
		if (xcond1 <=0)
		{
			int dx = abs(m_dx)-m_tempShowImagePoint.x();
			m_showImagePoint.setX(0);
			m_showImagePoint.setY(m_dy+m_tempShowImagePoint.y());
			m_image = m_tempImage.copy(dx,0,m_desImageRect.width-dx,this->height()-(m_dy+m_tempShowImagePoint.y()));
		}
		else if(xcond2 >(-m_desImageRect.width) && xcond2 <= this->width())
		{
			m_showImagePoint.setX(m_dx+m_tempShowImagePoint.x());
			m_showImagePoint.setY(m_dy+m_tempShowImagePoint.y());
			m_image = m_tempImage.copy(0,0,m_desImageRect.width,this->height()-(m_dy+m_tempShowImagePoint.y()));
		}
		else
		{
			m_showImagePoint.setX(m_dx+m_tempShowImagePoint.x());
			m_showImagePoint.setY(m_dy+m_tempShowImagePoint.y());
			m_image = m_tempImage.copy(0,0,this->width()-(m_dx+m_tempShowImagePoint.x()),
				this->height()-(m_dy+m_tempShowImagePoint.y()));
		}
	}
}
void ViewWidget::mousePressEvent(QMouseEvent *event)
{
	if(event->button() == Qt::LeftButton&&m_hasImage&&m_isOnClick)
	{
		double newHeightScale = ((double)m_readImageRect.height)/m_desImageRect.height;
		double newWidthScale = ((double)m_readImageRect.width)/m_desImageRect.width;

		double showImagex = event->localPos().x()-m_showImagePoint.x();
		double showImagey = event->localPos().y()-m_showImagePoint.y();

		int x = static_cast<int>(showImagex*newWidthScale+m_readImageRect.x);
		int y = static_cast<int>(showImagey*newHeightScale+m_readImageRect.y);

		if (x<0||y<0||x>m_oriImageSize.width()||y>m_oriImageSize.height())
		{
			QMessageBox::warning(this,QStringLiteral("刺点错误"),QStringLiteral("点在图像外"));
			return;
		}
		int w = 3;
		int h = 3;
		int startx = x-1;
		int starty = y-1;
		if (x==0&&y==0)
		{
			//4邻域;
			w = 2;
			h = 2;
			startx = x;
			starty = y;
		}
		else if (x==0&&y!=0&&y!=m_oriImageSize.width()-1)
		{
			w=2;
			startx = x;
		}
		else if (x==0&&y==m_oriImageSize.height()-1)
		{
			w = 2;
			h = 2;
			startx = x;
		}
		else if (x!=0&&x!=m_oriImageSize.width()-1&&y==m_oriImageSize.height()-1)
		{
			h=2;
			//starty = y-1;
		}
		else if (x==m_oriImageSize.width()-1&&y==m_oriImageSize.height()-1)
		{
			w = 2;
			h=2;
		}
		else if (x==m_oriImageSize.width()-1&&y!=m_oriImageSize.height()-1&&y!=0)
		{
			w = 2;
		}
		else if (x == m_oriImageSize.width()-1&&y==0)
		{
			starty =y;
			w = 2;
			h=2;
		}
		else if (x != 0&& x!= m_oriImageSize.width()-1&&y==0)
		{
			starty = y;
			h = 2;
		}
		int* tempPafScan = new int[w*h];
		int iSize = GDALGetDataTypeSize(m_type)/8;
		CPLErr xx = m_pDataset->GetRasterBand(1)->RasterIO(GF_Read,startx,starty,w,h,tempPafScan,w,h,GDT_Int32,0,0);
		
		int mostBlackDataX = startx;
		int mostBlackDataY = starty;
		int mostBackData = tempPafScan[0];
		for (int i = 0;i<h;++i)
		{
			for (int j = 0;j<w;++j)
			{
				if (tempPafScan[i*w+j]<mostBackData)
				{
					mostBackData = tempPafScan[i*w+j];
					mostBlackDataX=startx+j;
					mostBlackDataY=starty+i;
				}
			}
		}
		if (mostBlackDataY ==0 ||mostBlackDataX == 0||
			mostBlackDataX==m_oriImageSize.width()-1||mostBlackDataY == m_oriImageSize.height()-1)
		{
			QMessageBox::warning(this,QStringLiteral("刺点失误"),QStringLiteral("最黑点在边缘上"));
			delete[] tempPafScan;			
			return;
		}
		int tempPafScan2[3*3];
		CPLErr xx2 = m_pDataset->GetRasterBand(1)->RasterIO(GF_Read,startx,starty,w,h,tempPafScan,w,h,GDT_Int32,0,0);
		emit sendClickPoint(mostBlackDataX,mostBlackDataY,tempPafScan);
		delete[] tempPafScan;
		QMessageBox::about(this,QStringLiteral("点位"),QStringLiteral("x=")+
			QString::number(mostBlackDataX)+QStringLiteral(",y=")+QString::number(mostBlackDataY));
		

	}else if (event->button() == Qt::MidButton&&m_hasImage)
	{
		m_dx = 0;
		m_dy = 0;
		m_isOnMidButton = true;
		this->setCursor(Qt::OpenHandCursor);
		m_tempImage = m_image;
		m_tempShowImagePoint = m_showImagePoint;
		m_oldPoint = event->localPos();
		m_nowPoint = event->localPos();
	}
}
void ViewWidget::mouseMoveEvent(QMouseEvent * event)
{
	if (m_isOnMidButton)
	{
		m_nowPoint = event->localPos();
		CreateShowImage();
		update();
		m_oldPoint = m_nowPoint;
	}
}

void ViewWidget::wheelEvent(QWheelEvent * event)
{
	if (!m_hasImage)
	{
		return;
	}
	int numDegrees = event->delta();
	double oldscale = m_scale;
	if (numDegrees<0)
	{
		
		if (m_scale>50)
		{
			return;
			
		}
		m_scale*=1.2;
	}
	if (numDegrees>0)
	{
		if (m_scale<0.01)
		{
			return;
		}
		m_scale/=1.2;

	}
	QPointF xx = event->posF();
	m_nowImageRect.x = static_cast<int>(oldscale/m_scale*(m_nowImageRect.x - xx.x())+xx.x()+0.5);
	m_nowImageRect.y = static_cast<int>(oldscale/m_scale*(m_nowImageRect.y - xx.y())+xx.y()+0.5);
	m_nowImageRect.width = static_cast<int>(m_oriImageSize.width()/m_scale+0.5);
	m_nowImageRect.height = static_cast<int>(m_oriImageSize.height()/m_scale+0.5);
	URect tempNowShowRect = GetIntersectRect(m_nowImageRect);

	m_desImageRect = tempNowShowRect;
	m_showImagePoint.setX(tempNowShowRect.x);
	m_showImagePoint.setY(tempNowShowRect.y);

	tempNowShowRect.x = (-m_nowImageRect.x+tempNowShowRect.x);
	tempNowShowRect.y = (-m_nowImageRect.y+tempNowShowRect.y);
	m_readImageRect.x = (static_cast<int>(tempNowShowRect.x*m_scale+0.5));
	m_readImageRect.y = (static_cast<int>(tempNowShowRect.y*m_scale+0.5));
	m_readImageRect.width = (static_cast<int>(tempNowShowRect.width*m_scale+0.5));
	m_readImageRect.height = (static_cast<int>(tempNowShowRect.height*m_scale+0.5));

	if (m_readImageRect.x+m_readImageRect.width>m_oriImageSize.width())
	{
		m_readImageRect.x = m_oriImageSize.width()-m_readImageRect.width;
		if (m_readImageRect.x<0)
		{
			m_readImageRect.x=0;
		}
		if (m_readImageRect.width>m_oriImageSize.width())
		{
			m_readImageRect.width=m_oriImageSize.width();
		}
	}
	if (m_readImageRect.y+m_readImageRect.height>m_oriImageSize.height())
	{
		m_readImageRect.y = m_oriImageSize.height()-m_readImageRect.height;
		if (m_readImageRect.y<0)
		{
			m_readImageRect.y=0;
		}
		if (m_readImageRect.height>m_oriImageSize.height())
		{
			m_readImageRect.height=m_oriImageSize.height();
		}
	}

	readImage();
	update();

}
void ViewWidget::mouseReleaseEvent(QMouseEvent * event)
{
	if (event->button()==Qt::MidButton&&m_hasImage)
	{
		if(m_isOnClick)
		{
			this->setCursor(Qt::CrossCursor);
		}
		else
		{
			this->setCursor(Qt::ArrowCursor);
		}
		m_isOnMidButton = false;
		//m_desImageRect = m_image.rect();
		m_nowImageRect.x = m_dx+m_nowImageRect.x;
		m_nowImageRect.y = m_dy+m_nowImageRect.y;
		//int x = m_nowImageRect.width();
		URect tempNowShowRect = GetIntersectRect(m_nowImageRect);
		
		m_desImageRect = tempNowShowRect;
		
		m_showImagePoint.setX(tempNowShowRect.x);
		m_showImagePoint.setY(tempNowShowRect.y);
		
		tempNowShowRect.x = (-m_nowImageRect.x+tempNowShowRect.x);
		tempNowShowRect.y = (-m_nowImageRect.y+tempNowShowRect.y);

	    m_readImageRect.x = (static_cast<int>(tempNowShowRect.x*m_scale+0.5));
		m_readImageRect.y = (static_cast<int>(tempNowShowRect.y*m_scale+0.5));
		m_readImageRect.width = (static_cast<int>(tempNowShowRect.width*m_scale+0.5));
		m_readImageRect.height = (static_cast<int>(tempNowShowRect.height*m_scale+0.5));

		if (m_readImageRect.x+m_readImageRect.width>m_oriImageSize.width())
		{
			m_readImageRect.x = m_oriImageSize.width()-m_readImageRect.width;
			if (m_readImageRect.x<0)
			{
				m_readImageRect.x=0;
			}
			if (m_readImageRect.width>m_oriImageSize.width())
			{
				m_readImageRect.width=m_oriImageSize.width();
			}
		}
		if (m_readImageRect.y+m_readImageRect.height>m_oriImageSize.height())
		{
			m_readImageRect.y = m_oriImageSize.height()-m_readImageRect.height;
			if (m_readImageRect.y<0)
			{
				m_readImageRect.y=0;
			}
			if (m_readImageRect.height>m_oriImageSize.height())
			{
				m_readImageRect.height=m_oriImageSize.height();
			}
		}

		readImage();
		update();
	}
}
URect ViewWidget::GetIntersectRect(URect r1)
{
	URect rect(0,0,0,0);
	//Judge where the left-top Point
	if(r1.x <= 0)
	{
		if ((r1.x+r1.width)<=this->width()&&(r1.x+r1.width)>0)
		{
			rect.width = (r1.x+r1.width);
		}
		else if (r1.x+r1.width>this->width())
		{
			rect.width=(this->width());
		}
		if (r1.y<=0)
		{
			rect.x=(0);
			rect.y=(0);
			if (r1.y+r1.height>this->height())
			{
				rect.height=(this->height());
				return rect;
			} 
			else if(r1.y+r1.height>0&&r1.y+r1.height<=this->height())
			{
				rect.height=(r1.y+r1.height);
				return rect;
			}
		}
		else if (r1.y>0&&r1.y<=this->height())
		{
			rect.x=(0);
			rect.y=(r1.y);
			if (r1.y+r1.height>this->height())
			{
				rect.height=(this->height()-r1.y);
				return rect;

			} 
			else if(r1.y+r1.height>0&&r1.y+r1.height<=this->height())
			{
				rect.height=(r1.height);
				return rect;
			}
		}
	} 
	else if(r1.x > 0&&r1.x<=this->width())
	{
		if (r1.x+r1.width>this->width())
		{
			rect.width=(this->width()-r1.x);
		}
		else if (r1.x+r1.width<=this->width()&&r1.x+r1.width>0)
		{
			rect.width=(r1.width);
		}
		
		if (r1.y<=0)
		{
			rect.x=(r1.x);
			rect.y=(0);
			if (r1.y+r1.height<=this->height()&&r1.y+r1.height>0)
			{
				rect.height=(r1.y+r1.height);
				return rect;
			}
			else if (r1.y+r1.height>this->height())
			{
				rect.height=(this->height());
				return rect;
			}
		}
		else if (r1.y>0&&r1.y<=this->height())
		{
			rect.x=(r1.x);
			rect.y=(r1.y);
			if (r1.y+r1.height<=this->height()&&r1.y+r1.height>0)
			{
				rect.height=(r1.height);
				return rect;
			}
			else if (r1.y+r1.height>this->height())
			{
				rect.height=(this->height()-r1.y);
				return rect;
			}
		}

	}
	return URect(0,0,0,0);
}
void ViewWidget::resizeEvent(QResizeEvent* event)
{
		
}
void ViewWidget::getStartClick()
{
	this->setCursor(Qt::CrossCursor);
	m_isOnClick = true;
}
void ViewWidget::setViewColor(QVector<int> xx)
{
	nowShowStyle = xx;
	readImage();
	update();
}
void ViewWidget::changeCurrseToNormal()
{
	m_isOnClick = false;
	this->setCursor(Qt::ArrowCursor);
}
void ViewWidget::addClickedPoint(int x,int y)
{
	m_clickPoint.append(QPoint(x,y));
	this->update();
}
void ViewWidget::delClickedPoint(int i)
{
	m_clickPoint.remove(i);
	this->update();
}
void ViewWidget::zoomToSelectedPoint(QListWidgetItem* item)
{
	int i = (item->listWidget())->currentRow();

	int dx = static_cast<int>(m_clickPoint[i].x()/m_scale+m_nowImageRect.x-0.5*width()+0.5);
	int dy= static_cast<int>(m_clickPoint[i].y()/m_scale+m_nowImageRect.y-0.5*height()+0.5);
	m_nowImageRect.x = -dx+m_nowImageRect.x;
	m_nowImageRect.y = -dy+m_nowImageRect.y;
	//int x = m_nowImageRect.width();
	URect tempNowShowRect = GetIntersectRect(m_nowImageRect);

	m_desImageRect = tempNowShowRect;

	m_showImagePoint.setX(tempNowShowRect.x);
	m_showImagePoint.setY(tempNowShowRect.y);

	tempNowShowRect.x = (-m_nowImageRect.x+tempNowShowRect.x);
	tempNowShowRect.y = (-m_nowImageRect.y+tempNowShowRect.y);

	m_readImageRect.x = (static_cast<int>(tempNowShowRect.x*m_scale+0.5));
	m_readImageRect.y = (static_cast<int>(tempNowShowRect.y*m_scale+0.5));
	m_readImageRect.width = (static_cast<int>(tempNowShowRect.width*m_scale+0.5));
	m_readImageRect.height = (static_cast<int>(tempNowShowRect.height*m_scale+0.5));

	if (m_readImageRect.x+m_readImageRect.width>m_oriImageSize.width())
	{
		m_readImageRect.x = m_oriImageSize.width()-m_readImageRect.width;
		if (m_readImageRect.x<0)
		{
			m_readImageRect.x=0;
		}
		if (m_readImageRect.width>m_oriImageSize.width())
		{
			m_readImageRect.width=m_oriImageSize.width();
		}
	}
	if (m_readImageRect.y+m_readImageRect.height>m_oriImageSize.height())
	{
		m_readImageRect.y = m_oriImageSize.height()-m_readImageRect.height;
		if (m_readImageRect.y<0)
		{
			m_readImageRect.y=0;
		}
		if (m_readImageRect.height>m_oriImageSize.height())
		{
			m_readImageRect.height=m_oriImageSize.height();
		}
	}
	readImage();
	update();
}