#ifndef SETVIEWCOLORWIDGET_H
#define SETVIEWCOLORWIDGET_H

//#include <QObject>
#include <QDialog>

#include "ui_setViewColor.h"
class SetViewColorWidget : public QDialog,public Ui::Dialog
{
    Q_OBJECT
public:
    explicit SetViewColorWidget(QWidget *parent = 0,int bandCount = 0);
    QVector<int> getResults();
signals:
public slots:
	void colorRadioClicked();
    void grayRadioClicked();
private:
	//QVector<int> m_vect;
};

#endif // SETVIEWCOLORWIDGET_H
