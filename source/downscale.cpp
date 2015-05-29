#include <gdal_priv.h>
#include "param.h"
#include "downscale.h"
/////////////////////////////
//测试
#include <iostream>
using namespace std;
/////////////////////////////
const int EXE_FILEWRONG = 5;
const int EXE_OK = 1;
const int EXE_Failure = -5;

DownScale::DownScale(const char* pazSrcFile)
{
	/****************************************
	* Missing code about determining whether the file is exists！
	*****************************************/
	m_pszSrcFile = pazSrcFile;

	m_iBandCount = 1;
	//m_pColor = NULL;
	m_projection = "";
	m_pGeoAffine = NULL;
	m_pColorTable = NULL;
	m_isInitial = false;
	//m_pBandMean = NULL;
	//m_pBandStd = NULL;
	m_pSrcDS = NULL;
}
DownScale::~DownScale()
{
	if (m_pGeoAffine != NULL)
	{
		delete[] m_pGeoAffine;
	}
	if (m_pColorTable != NULL)
	{
		//GDALClose(m_pColorTable);
	}
	if (m_pSrcDS != NULL)
	{
		GDALClose(m_pSrcDS);
		//delete m_pSrcDS;
	}
}
int DownScale::PreProcessData()
{
	if (!m_isInitial)
	{
		//Register GDAL all file formats driver
		GDALAllRegister();
		m_pSrcDS = (GDALDataset*)GDALOpen(m_pszSrcFile,GA_ReadOnly);
		if (m_pSrcDS == NULL)
		{
			return EXE_FILEWRONG;
		}

		m_iBandCount = m_pSrcDS->GetRasterCount();

		m_iXsize = m_pSrcDS->GetRasterXSize();
		m_iYsize = m_pSrcDS->GetRasterYSize();

		m_projection = m_pSrcDS->GetProjectionRef();
		m_pGeoAffine = new double[6];
		m_pSrcDS->GetGeoTransform(m_pGeoAffine);
		m_pColorTable = m_pSrcDS->GetRasterBand(1)->GetColorTable();

		if (m_pColorTable != NULL && m_pGeoAffine != NULL)
		{
			m_isInitial = true;
			return EXE_OK;
		}
		else
		{
			return EXE_Failure;
		}
	}

}
Byte* DownScale::ReadData()
{
	if (!m_isInitial)
	{
		return NULL;
	}
	Byte* data = new Byte[m_iXsize*m_iYsize];
	CPLErr re = m_pSrcDS->GetRasterBand(1)->
		RasterIO(GF_Read,0,0,m_iXsize,m_iYsize,data,m_iXsize,m_iYsize,GDT_Byte,0,0);
	if (re == CE_None)
	{
		return data;
	}
	else
	{
		return NULL;
	}

}
int DownScale::ExecuteAggregation(const char* pazOutFile,int msize)
{
	PreProcessData();
	Byte* data = ReadData();
	if (data == NULL)
	{
		return EXE_Failure;
	}
	double resolution = m_pGeoAffine[1];
	int newFileXsize = (m_iXsize+msize-1)/msize;
	int newFileYsize = (m_iYsize+msize-1)/msize;

	Byte* newData = new Byte[newFileXsize*newFileYsize];

	for (int i = 0; i < newFileYsize; i++)
	{
		for(int j = 0;j< newFileXsize;++j)
		{
			Byte test = Marjority_Aggregation(i,j,data,msize);
			newData[i*newFileXsize+j] = test;
			//delete[] tempData;
			//tempData = NULL;
		}
	}
	const char *pszFormat = "GTiff";
	GDALDriver *poDriver =(GDALDriver*)GDALGetDriverByName(pszFormat);
	GDALDataset* poDst = poDriver->Create(pazOutFile,newFileXsize,newFileYsize,1,GDT_Byte,NULL);
	double* geoInfo = new double[6];
	geoInfo[0] = m_pGeoAffine[0];
	geoInfo[1] = m_pGeoAffine[1]*msize;
	geoInfo[2] = m_pGeoAffine[2];
	geoInfo[3] = m_pGeoAffine[3];
	geoInfo[4] = m_pGeoAffine[4];
	geoInfo[5] = m_pGeoAffine[5]*msize;
	poDst->SetGeoTransform(geoInfo);
	poDst->SetProjection(m_projection);
	poDst->GetRasterBand(1)->SetColorTable(m_pColorTable);
	poDst->GetRasterBand(1)->RasterIO(GF_Write,0,0,newFileXsize,newFileYsize,newData,newFileXsize,newFileYsize,GDT_Byte,0,0);
	GDALClose(poDst);
	delete[] data;
	return EXE_OK;
}
int DownScale::ExecuteAggregationEx(const char* pszOutFile,int msize)
{
	PreProcessData();
	Byte* data = ReadData();
	if (data == NULL)
	{
		return EXE_Failure;
	}
	int newFileXsize = (m_iXsize*3+msize-1)/msize;
	int newFileYsize = (m_iYsize*3+msize-1)/msize;

	Byte* newData = new Byte[newFileXsize*newFileYsize];

	for (int i = 0; i < newFileYsize; i++)
	{
		for(int j = 0;j< newFileXsize;++j)
		{
			Byte test = Marjority_AggregationEx(i,j,data,msize);
			newData[i*newFileXsize+j] = test;
		}
	}
	const char *pszFormat = "GTiff";
	GDALDriver *poDriver =(GDALDriver*)GDALGetDriverByName(pszFormat);
	GDALDataset* poDst = poDriver->Create(pszOutFile,newFileXsize,newFileYsize,1,GDT_Byte,NULL);
	double* geoInfo = new double[6];
	geoInfo[0] = m_pGeoAffine[0];
	geoInfo[1] = m_pGeoAffine[1]*25/3;
	geoInfo[2] = m_pGeoAffine[2];
	geoInfo[3] = m_pGeoAffine[3];
	geoInfo[4] = m_pGeoAffine[4];
	geoInfo[5] = m_pGeoAffine[5]*25/3;
	poDst->SetGeoTransform(geoInfo);
	poDst->SetProjection(m_projection);
	poDst->GetRasterBand(1)->SetColorTable(m_pColorTable);
	poDst->GetRasterBand(1)->RasterIO(GF_Write,0,0,newFileXsize,newFileYsize,newData,newFileXsize,newFileYsize,GDT_Byte,0,0);
	GDALClose(poDst);
	delete[] data;
	return EXE_OK;
}
int DownScale::ExecuteSmooth(const char* pszOutFile,int msize,int smoothwindowSize)
{
	PreProcessData();
	Byte** pD = new Byte*[msize];
	//临时替换指针
	Byte* temp = NULL;
	for (int i = 0; i < msize; i++)
	{
		pD[i] = new Byte[m_iXsize];
		m_pSrcDS->GetRasterBand(1)->RasterIO(GF_Read,0,i,m_iXsize,1,pD[i],m_iXsize,1,GDT_Byte,0,0);
	}
	//for (int i = 0; i < 100; i++)
	//{
	//	cout << (int)pD[0][i] <<" "<<(int)pD[1][i]<<" "<<(int)pD[2][i]<<endl;
	//}

	const char *pszFormat = "GTiff";
	GDALDriver *poDriver =(GDALDriver*)GDALGetDriverByName(pszFormat);
	GDALDataset* poDst = poDriver->Create(pszOutFile,m_iXsize,m_iYsize,1,GDT_Byte,NULL);
	//double* geoInfo = new double[6];
	//geoInfo[0] = m_pGeoAffine[0];
	//geoInfo[1] = m_pGeoAffine[1];
	//geoInfo[2] = m_pGeoAffine[2];
	//geoInfo[3] = m_pGeoAffine[3];
	//geoInfo[4] = m_pGeoAffine[4];
	//geoInfo[5] = m_pGeoAffine[5];
	poDst->SetProjection(m_projection);

	poDst->SetGeoTransform(m_pGeoAffine);
	poDst->GetRasterBand(1)->SetColorTable(m_pColorTable);


	int r = msize/2;
	Byte** cell = new Byte*[msize*msize];

	for (int i = 0; i < m_iYsize; i++)
	{
		Byte* tempdata = new Byte[m_iXsize];
		if (i>r && i<(m_iYsize-r))
		{
			temp = pD[0];
			for (int pos = 1; pos < msize; pos++)
			{
				pD[pos-1] = pD[pos];
			}
			pD[msize-1] = temp;
			m_pSrcDS->GetRasterBand(1)->RasterIO(GF_Read,0,i+r,m_iXsize,1,pD[msize-1],m_iXsize,1,GDT_Byte,0,0);
		}
		for (int j = 0; j < m_iXsize; j++)
		{
			//cout << i << ":";
			//cout << j << endl;

			//前面m_size-r行
			if (i<=r)
			{
				for (int wi = -r; wi <= r; wi++)
				{
					for (int wj = -r; wj <= r; wj++)
					{
						if (i+wi < 0 || j+wj < 0 || j+wj>=m_iXsize)
						{
							cell[(wi+r)*msize + (wj+r)] = NULL;
						}
						else
						{
							cell[(wi+r)*msize + (wj+r)] = pD[i+wi]+j+wj;
						}
					}
				}
				tempdata[j] = Marjority_Smoothing(cell,msize,1);
			}
			else if (i>r && i<(m_iYsize-r))
			{
				//cout << i+r <<" ";

				for (int wi = -r; wi <= r; wi++)
				{
					for (int wj = -r; wj <= r; wj++)
					{
						if (j+wj < 0 || j+wj>=m_iXsize)
						{
							cell[(wi+r)*msize + (wj+r)] = NULL;
						}
						else
						{
							cell[(wi+r)*msize + (wj+r)] = pD[wi+r]+j+wj;
						}
					}
				}

				tempdata[j] = Marjority_Smoothing(cell,msize,1);

			}
			else if (i >= m_iYsize-r)
			{
				for (int wi = -r; wi <= r; wi++)
				{
					for (int wj = -r; wj <= r; wj++)
					{
						if (j+wj < 0 || j+wj>=m_iXsize || i+wi >= m_iYsize)
						{
							cell[(wi+r)*msize + (wj+r)] = NULL;
						}
						else
						{
							cell[(wi+r)*msize + (wj+r)] = pD[wi+r]+j+wj;
						}
					}
				}
				tempdata[j] = Marjority_Smoothing(cell,msize,1);

			}
		}
		poDst->GetRasterBand(1)->RasterIO(GF_Write,0,i,m_iXsize,1,tempdata,m_iXsize,1,GDT_Byte,0,0);
		delete[] tempdata;
	}
	GDALClose(poDst);

	//cout << "xxx";
	for (int i = 0; i < msize; i++)
	{
		delete[] pD[i];
	}
	delete[] pD;
	return 0;
}
Byte DownScale::Marjority_Aggregation(int oi,int oj, Byte* data,int msize)
{
	int* sta = new int[numClass];
	for (int i = 0; i < numClass; i++)
	{
		sta[i] = 0;
		//id[i] = i;
	}
	for (int i = 0; i < msize; i++)
	{
		for (int j = 0; j < msize; j++)
		{

			if ((oi*msize+i) >= m_iYsize || (oj*msize+j) >= m_iXsize)
			{
				continue;
			}
			int ind = (oi*msize+i)*m_iXsize + (oj*msize+j);
			if (data[ind] == 255)
			{
				sta[0]++;
			}
			else
			{
				sta[data[ind]/10]++;
			}
		}
	}
	//排序
	//Sort(sta,numClass,id,numClass);
	int reid = 0;
	int temp = sta[reid];
	for (int i = 1; i < numClass; i++)
	{
		if (sta[i] > temp)
		{
			temp = sta[i];
			reid = i;
		}
	}
	Byte re =  (Byte)(reid*10);

	delete[] sta;
	//delete[] id;
	return re;
}
Byte DownScale::Marjority_AggregationEx(int oi,int oj,Byte* data, int msize)
{
	int* sta = new int[numClass];
	for (int i = 0; i < numClass; i++)
	{
		sta[i] = 0;
		//id[i] = i;
	}
	//cout << oi << ":"<<oj<<endl;

	for (int i = 0; i < msize; i++)
	{
		for (int j = 0; j < msize; j++)
		{
			if ((oi*msize+i)/3 >= m_iYsize || (oj*msize+j)/3 >= m_iXsize)
			{
				continue;
			}

			int ind = ((oi*msize+i)/3)*m_iXsize + ((oj*msize+j)/3);
			//if (ind >= 1802*1801)
			//{
			//	cout << ind <<endl;
			//}
			if (data[ind] == 255)
			{
				sta[0]++;
			}
			else
			{
				sta[data[ind]/10]++;
			}
		}
	}
	int reid = 0;

	int temp = sta[reid];

	for (int i = 1; i < numClass; i++)
	{
		if (sta[i] > temp)
		{
			temp = sta[i];
			reid = i;
		}
	}
	Byte re =  (Byte)(reid*10);

	delete[] sta;
	//delete[] id;
	return re;
}

void DownScale::Sort(int* a, int n, int* id, int m)
{
	if ( m > 1)
	{
		int i = 0; 
		int j = m-1;
		int tmp = id[i];
		while(i<j)
		{
			while(j > i && a[id[j]] > a[tmp]) 
				--j;
			if (j > i)
				id[i++] = id[j];  //只改变索引顺序
			while(j > i && a[id[i]] < a[tmp])
				++i;
			if (j > i)
				id[j--] = id[i];  //只改变索引顺序
		}
		id[i] = tmp;
		Sort(a, n, id, i);
		Sort(a, n, id + i + 1, m - i - 1);
	}
}
Byte DownScale::Marjority_Smoothing(Byte** data,int size,int centerPriority = 1)
{
	int totalInvalidData = 0;
	//if (data == NULL)
	//{
	//	cout << "xxxxxxxxxxxxxxxx"<< endl;
	//}

	int* sta = new int[numClass];
	for (int i = 0; i < numClass; i++)
	{
		sta[i] = 0;
		//id[i] = i;
	}
	for (int i = 0; i < size; i++)
	{
		for (int j = 0; j < size; j++)
		{
			//当前数据
			if ((data[i*size+j]) == NULL)
			{
				totalInvalidData++;
				continue;
			}
			Byte d = *(data[i*size+j]);

			//cout << (int)(*data[i*size+j]);
			if((i == size/2) && (j == size/2))
			{
				if (d == 255)
				{
					sta[0]+=centerPriority;
				}
				else
				{
					sta[d/10]+=centerPriority;
				}
			}
			else
			{
				if (d == 255)
				{
					sta[0]++;
				}
				else
				{
					sta[d/10]++;
				}
			}

		}
	}
	int reid = 0;
	int temp = sta[reid];
	for (int i = 1; i < numClass; i++)
	{
		if (sta[i] > temp)
		{
			temp = sta[i];
			reid = i;
		}
	}
	Byte re;
	if (sta[reid] > ((size*size-totalInvalidData)/2))
	{
		re =  (Byte)(reid*10);
	}
	else
	{
		if (sta[reid]-sta[*(data[(size/2)*size+size/2])/10] > size*size/4)
		{
			re =  (Byte)(reid*10);
		}
		else
		{
			re = *(data[(size/2)*size+size/2]);
		}
	}
	delete[] sta;
	return re;
}
int DownScale::ExecuteAggregationWithPriority(const char* pazOutFile,int msize,int option = 1)
{
	PreProcessData();
	Byte* data = ReadData();
	if (data == NULL)
	{
		return EXE_Failure;
	}

	//聚合后文件大小
	int newFileXsize = (m_iXsize+msize-1)/msize;
	int newFileYsize = (m_iYsize+msize-1)/msize;

	Byte* newData = new Byte[newFileXsize*newFileYsize];

	for (int i = 0; i < newFileYsize; i++)
	{
		for(int j = 0;j< newFileXsize;++j)
		{
			Byte test = Marjority_AggregationwithPriority2(i,j,data,msize);
			newData[i*newFileXsize+j] = test;
		}
	}
	const char *pszFormat = "GTiff";
	GDALDriver *poDriver =(GDALDriver*)GDALGetDriverByName(pszFormat);
	GDALDataset* poDst = poDriver->Create(pazOutFile,newFileXsize,newFileYsize,1,GDT_Byte,NULL);
	double* geoInfo = new double[6];
	geoInfo[0] = m_pGeoAffine[0];
	geoInfo[1] = m_pGeoAffine[1]*msize;
	geoInfo[2] = m_pGeoAffine[2];
	geoInfo[3] = m_pGeoAffine[3];
	geoInfo[4] = m_pGeoAffine[4];
	geoInfo[5] = m_pGeoAffine[5]*msize;
	poDst->SetGeoTransform(geoInfo);
	poDst->SetProjection(m_projection);
	poDst->GetRasterBand(1)->SetColorTable(m_pColorTable);
	poDst->GetRasterBand(1)->RasterIO(GF_Write,0,0,newFileXsize,newFileYsize,newData,newFileXsize,newFileYsize,GDT_Byte,0,0);
	GDALClose(poDst);
	delete[] data;
	return 0;
}
Byte DownScale::Marjority_AggregationwithPriority1(int oi,int oj,Byte* data,int msize)
{
	int* sta = new int[numClass];
	//int* id = new int[numClass];
	for (int i = 0; i < numClass; i++)
	{
		sta[i] = 0;
		//id[i] = i;
	}
	for (int i = 0; i < msize; i++)
	{
		for (int j = 0; j < msize; j++)
		{

			if ((oi*msize+i) >= m_iYsize || (oj*msize+j) >= m_iXsize)
			{
				continue;
			}
			int ind = (oi*msize+i)*m_iXsize + (oj*msize+j);
			if (data[ind] == 255)
			{
				sta[0]++;
			}
			else
			{
				sta[data[ind]/10]++;
			}
		}
	}
	//排序
	//Sort(sta,numClass,id,numClass);

	int candidateid = 0;
	int temp = sta[candidateid];
	for (int i = 1; i < numClass; i++)
	{
		if (sta[i] > temp)
		{
			temp = sta[i];
			candidateid = i;
		}

	}
	if (sta[candidateid] <= msize*msize/2)
	{
		for (int i = candidateid+1; i < numClass; i++)
		{


			if (sta[i] == temp)
			{
				if (classProity[i] > classProity[candidateid])
				{
					//优先级判断
					candidateid = i;
				}
				else if (classProity[i] == classProity[candidateid])
				{
					//语义邻近度判断
					double classI = 0;
					double classCand = 0;

					for (int j = 0; j < numClass; j++)
					{
						classI+=confuseMatrix[i][j]*sta[j];
						classI+=confuseMatrix[j][i]*sta[j];

						classCand+=confuseMatrix[candidateid][j]*sta[j];
						classCand+=confuseMatrix[j][candidateid]*sta[j];
					}
					if (classI > classCand)
					{
						candidateid = i;
					}
				}
			}

		}
	}


	Byte re =  (Byte)(candidateid*10);

	delete[] sta;
	//delete[] id;
	//delete[] id;
	return re;
}
Byte DownScale::Marjority_AggregationwithPriority1Ex(int oi,int oj, Byte* data,int msize)
{
	int* sta = new int[numClass];
	//int* id = new int[numClass];
	for (int i = 0; i < numClass; i++)
	{
		sta[i] = 0;
		//id[i] = i;
	}
	for (int i = 0; i < msize; i++)
	{
		for (int j = 0; j < msize; j++)
		{

			if ((oi*msize+i)/3 >= m_iYsize || (oj*msize+j)/3 >= m_iXsize)
			{
				continue;
			}
			int ind = ((oi*msize+i)/3)*m_iXsize + ((oj*msize+j)/3);
			if (data[ind] == 255)
			{
				sta[0]++;
			}
			else
			{
				sta[data[ind]/10]++;
			}
		}
	}
	//排序
	//Sort(sta,numClass,id,numClass);

	int candidateid = 0;
	int temp = sta[candidateid];
	for (int i = 1; i < numClass; i++)
	{
		if (sta[i] > temp)
		{
			temp = sta[i];
			candidateid = i;
		}

	}
	if (sta[candidateid] <= msize*msize/2)
	{
		for (int i = candidateid+1; i < numClass; i++)
		{


			if (sta[i] == temp)
			{
				if (classProity[i] > classProity[candidateid])
				{
					//优先级判断
					candidateid = i;
				}
				else if (classProity[i] == classProity[candidateid])
				{
					//语义邻近度判断
					double classI = 0;
					double classCand = 0;

					for (int j = 0; j < numClass; j++)
					{
						classI+=confuseMatrix[i][j]*sta[j];
						classI+=confuseMatrix[j][i]*sta[j];

						classCand+=confuseMatrix[candidateid][j]*sta[j];
						classCand+=confuseMatrix[j][candidateid]*sta[j];
					}
					if (classI > classCand)
					{
						candidateid = i;
					}
				}
			}

		}
	}


	Byte re =  (Byte)(candidateid*10);

	delete[] sta;
	return re;
}
Byte DownScale::Marjority_AggregationwithPriority2(int oi,int oj,Byte* data,int msize)
{
	double* sta = new double[numClass];
	//int* id = new int[numClass];
	for (int i = 0; i < numClass; i++)
	{
		sta[i] = 0.0;
		//id[i] = i;
	}
	for (int i = 0; i < msize; i++)
	{
		for (int j = 0; j < msize; j++)
		{

			if ((oi*msize+i) >= m_iYsize || (oj*msize+j) >= m_iXsize)
			{
				continue;
			}
			int ind = (oi*msize+i)*m_iXsize + (oj*msize+j);
			if (data[ind] == 255)
			{
				sta[0]+= 1;
			}
			else
			{
				sta[data[ind]/10]+=classProity[data[ind]/10];
			}
		}
	}
	double* newsta = new double[numClass];

	for (int i = 0; i < numClass; i++)
	{
		newsta[i] = sta[i];
		for (int j = 0; j < numClass; j++)
		{
			if(i != j)
			{
				newsta[i] += sta[j]*confuseMatrix[j][i];
				newsta[i] += sta[j]*confuseMatrix[i][j];
			}
		}
	}
	delete[] sta;
	//排序
	//Sort(sta,numClass,id,numClass);

	int candidateid = 0;
	double temp = newsta[candidateid];
	for (int i = 1; i < numClass; i++)
	{
		if (newsta[i] > temp)
		{
			temp = newsta[i];
			candidateid = i;
		}
	}

	Byte re =  (Byte)(candidateid*10);

	//delete[] id;
	delete[] newsta;
	return re;
}
Byte DownScale::Marjority_AggregationwithPriority2Ex(int oi,int oj,Byte* data,int msize)
{
	double* sta = new double[numClass];
	//int* id = new int[numClass];
	for (int i = 0; i < numClass; i++)
	{
		sta[i] = 0.0;
		//id[i] = i;
	}
	for (int i = 0; i < msize; i++)
	{
		for (int j = 0; j < msize; j++)
		{

			if ((oi*msize+i)/3 >= m_iYsize || (oj*msize+j)/3 >= m_iXsize)
			{
				continue;
			}
			int ind = ((oi*msize+i)/3)*m_iXsize + ((oj*msize+j)/3);
			if (data[ind] == 255)
			{
				sta[0]+= 1/9;
			}
			else
			{
				sta[data[ind]/10]+=classProity[data[ind]/10]/9;
			}
		}
	}
	double* newsta = new double[numClass];

	for (int i = 0; i < numClass; i++)
	{
		newsta[i] = sta[i];
		for (int j = 0; j < numClass; j++)
		{
			if(i != j)
			{
				newsta[i] += sta[j]*confuseMatrix[j][i];
				newsta[i] += sta[j]*confuseMatrix[i][j];
			}
		}
	}
	delete[] sta;
	//排序
	//Sort(sta,numClass,id,numClass);

	int candidateid = 0;
	double temp = newsta[candidateid];
	for (int i = 1; i < numClass; i++)
	{
		if (newsta[i] > temp)
		{
			temp = newsta[i];
			candidateid = i;
		}
	}

	Byte re =  (Byte)(candidateid*10);

	//delete[] id;
	delete[] newsta;
	return re;
}