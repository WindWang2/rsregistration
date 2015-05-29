#ifndef CREATERASETERPYRAMIDS_H
#define CREATERASETERPYRAMIDS_H

#include <QThread>
#include <QStringList>
class CreateRaseterPyramids : public QThread
{
    Q_OBJECT
public:
    explicit CreateRaseterPyramids(QStringList fileNameList,QObject *parent = 0);
    virtual void run();
	int buildOverViews(QString m_ImagePath);
    
signals:
	void CreatePyramidsFinished(QString);
	void CreatePyramidsFailed(QString);
    
public slots:
private:
	QStringList m_fileNameList;
};

#endif // CREATERASETERPYRAMIDS_H
