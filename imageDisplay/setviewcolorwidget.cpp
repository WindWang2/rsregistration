#include <QtWidgets>
#include <QVector>
#include "setviewcolorwidget.h"

SetViewColorWidget::SetViewColorWidget(QWidget *parent,int bandCount) :
    QDialog(parent)
{
    setupUi(this);
	QStringList xx = QStringList();
	for (int i = 1;i<=bandCount;++i)
	{
		xx.append(QString::number(i));
	}
	this->rcomboBox->addItems(xx);
	this->gcomboBox->addItems(xx);
	this->bcomboBox->addItems(xx);
	this->pcomboBox->addItems(xx);
	connect(colorRadio,SIGNAL(clicked()),this,SLOT(colorRadioClicked()));
	connect(grayRadio,SIGNAL(clicked()),this,SLOT(grayRadioClicked()));
	//connect(colorRadio,SIGNAL())
	if (grayRadio->isChecked())
	{
		groupBox->setEnabled(false);
	}else
	{
		pcomboBox->setEnabled(false);
	}
}
void SetViewColorWidget::colorRadioClicked()
{
	if (colorRadio->isChecked())
	{
		groupBox->setEnabled(true);
		pcomboBox->setEnabled(false);

	}
}
void SetViewColorWidget::grayRadioClicked()
{
	if (grayRadio->isCheckable())
	{
		pcomboBox->setEnabled(true);

		//grayRadio->setEnabled(false);
		groupBox->setEnabled(false);
	}
}
QVector<int> SetViewColorWidget::getResults()
{
	QVector<int> vect;
	if (colorRadio->isChecked())
	{
		vect.append(0);
		//int xx = rcomboBox->currentIndex();
		vect.append(rcomboBox->currentIndex()+1);
		vect.append(gcomboBox->currentIndex()+1);
		vect.append(bcomboBox->currentIndex()+1);	
	}
	else
	{
		vect.append(1);
		vect.append(pcomboBox->currentIndex()+1);
	}

	return vect;
}