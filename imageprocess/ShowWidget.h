#ifndef SHOWWIDGET_HH
#define SHOWWIDGET_HH
#include <QMainWindow>
class QPointF;
#include <QGraphicsView>

class ShowWidget:public QGraphicsView
{
	Q_OBJECT
public:
	ShowWidget(QWidget* parent);
protected:
	void mouseMoveEvent(QMouseEvent*);
signals:
	void sendPX(QPointF pt);
};

#endif