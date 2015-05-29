#include "ImageItem.h"

ImageItem::ImageItem(QWidget* parent = 0):QLabel(parent)
{
	
}
void ImageItem::mouseMoveEvent(QMouseEvent *event)
{
	emit sendPX(event->localPos());
}