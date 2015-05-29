#include <QtWidgets>
#include "InputPointWidget.h"
InputPointWidget::InputPointWidget(int flag,QWidget* parent):QDialog(parent)
{
	setupUi(this);
	//QRegExp regExp("^(-?\d+)(\.\d+)?$");
	this->lineEdit->setValidator(new QDoubleValidator(this));
	this->lineEdit_2->setValidator(new QDoubleValidator(this));
	this->lineEdit_3->setValidator(new QDoubleValidator(this));
	QObject::connect(buttonBox->buttons()[0], SIGNAL(clicked()), this, SLOT(okIsClicked()));
}
void InputPointWidget::okIsClicked()
{
	if (this->lineEdit->text() == ""||this->lineEdit_2->text()=="")
	{
		QMessageBox::warning(this,QStringLiteral("数据不能为空"),QStringLiteral("输入数据出错"));
	}
	else
	{
		accept();
	}
}
QVector<double> InputPointWidget::getInputData()
{
	double x = this->lineEdit->text().toDouble();
	double y = this->lineEdit_2->text().toDouble();
	double z= this->lineEdit_3->text().toDouble();

	QVector<double> data;
	data.append(x);
	data.append(y);
	data.append(z);
	return data;
}
