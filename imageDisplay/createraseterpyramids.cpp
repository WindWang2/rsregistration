#include "createraseterpyramids.h"
#include "gdal_priv.h"
CreateRaseterPyramids::CreateRaseterPyramids(QStringList xxxxx,QObject *parent):
    QThread(parent)
{
	m_fileNameList = xxxxx;
}
void CreateRaseterPyramids::run()
{
	for (int i = 0;i<m_fileNameList.count();++i)
	{
		int results = buildOverViews(m_fileNameList[i]);
		if (results == 1)
		{
			emit CreatePyramidsFinished(m_fileNameList[i]);
		}
		else
		{
			emit CreatePyramidsFailed(m_fileNameList[i]);
		}
	}
}
int CreateRaseterPyramids::buildOverViews(QString m_ImagePath)
{
	//GDALClose(m_pDataset);
	//GDALDatasetH hDataset;
	//hDataset = GDALOpen(m_ImagePath.toStdString().data(),GA_ReadOnly);
	//CPLSetConfigOption("TIF_USE_OVR","TRUE");
	CPLSetConfigOption("USE_RRD","YES");

	GDALDataset* pDataset = (GDALDataset*)GDALOpen(m_ImagePath.toStdString().data(),GA_ReadOnly);
	GDALDriver* Driver = pDataset->GetDriver();
	const char* pszDriver = GDALGetDriverShortName(Driver);

	//int cc = pDataset->GetRasterBand(1)->GetOverviewCount();
	if (pDataset->GetRasterBand(1)->GetOverviewCount())
	{
		return 1;
	}
	//GDACleanOverviews();
	if(pDataset == NULL)
	{
		return -1;
	}
	int iWidth = pDataset->GetRasterXSize();
	int iHeight = pDataset->GetRasterYSize();


	int iPixelNum = iWidth*iHeight;
	//顶层金字塔大小，64*64
	int iTopNum = 4096;
	//存储每一次个像元总数。当前为第一次
	int iCurNum = iPixelNum/4;

	int anLevels[1024]={0};
	int nLevelCount = 0;

	do 
	{
		anLevels[nLevelCount] = static_cast<int>(pow(2.0,nLevelCount+2));
		nLevelCount++;
		iCurNum /= 4;
	} while (iCurNum > iTopNum);
	const char* pszResampling="nearest";
	//GDALProgressFunc pfnProgress = GDALProgress;

	CPLErr xx = pDataset->BuildOverviews(pszResampling,
		nLevelCount,anLevels,0,NULL,NULL,NULL);
	//GDALBuildOverviews()
	//GDALClose(hDataset);
	//m_pDataset->
	//GDALDestroyDriverManager();
	//m_pDataset = (GDALDataset*)GDALOpen(m_ImagePath.toStdString().data(),GA_ReadOnly);
	GDALClose(pDataset);
	return 1;
}