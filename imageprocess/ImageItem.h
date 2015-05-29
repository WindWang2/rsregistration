#ifndef IMAGEITEM_HH
#define IMAGEITEM_HH
#include <QMainWindow>
class QPointF;
#include <QLabel>
#include <QMouseEvent>
class ImageItem:public QLabel
{
	Q_OBJECT
public:
	ImageItem(QWidget* parent);
protected:
	void mouseMoveEvent(QMouseEvent*);
signals:
	void sendPX(QPointF pt);
};

#endif