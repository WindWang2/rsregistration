#include "OperatDialog.h"

OperatDialog::OperatDialog(QWidget*parent /* = 0 */):QDialog(parent)
{
	setupUi(this);
}
int OperatDialog::getCheckedOption()
{
	if (this->changeButton->isChecked())
	{
		return 1;
	}else if (this->delRadio->isChecked())
	{
		return 2;
	}else if (this->radioButton_3->isChecked())
	{
		return 3;
	}
}
