#include "ShowWidget.h"
#include <QPointF>
#include <QtEvents>
#include <QScrollBar>
ShowWidget::ShowWidget(QWidget* parent = 0):QGraphicsView(parent)
{
	
}
void ShowWidget::mouseMoveEvent(QMouseEvent *event)
{
	QPoint xx = this->verticalScrollBar()->pos();
	int x = this->verticalScrollBar()->x();
	int y = this->verticalScrollBar()->y();
	emit sendPX(event->localPos());
}