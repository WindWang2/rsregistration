#ifndef OPERATDIALOG_HH
#define OPERATDIALOG_HH
#include <QDialog>
#include "ui_operatDialog.h"

class OperatDialog:public QDialog,public Ui::operatDialog
{
	Q_OBJECT
public:
	explicit OperatDialog(QWidget*parent = 0);
	int getCheckedOption();
};

#endif