#ifndef INPUTPOINTWIDGET_H
#define INPUTPOINTWIDGET_H

#include <QDialog>
#include "ui_InputPointInf.h"
class InputPointWidget:public QDialog,public Ui::InputPointDialog
{
	Q_OBJECT
public:
	explicit InputPointWidget(int flag,QWidget *parent = 0);
	QVector<double> getInputData();
public slots:
	void okIsClicked();
	
};


#endif