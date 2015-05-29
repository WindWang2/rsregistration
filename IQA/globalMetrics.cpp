#include "GlobalMetrics.h"
//////////////////////////////////////////////////////////////////////////
#include <iostream>
#include <omp.h>

//#include <fstream>
//////////////////////////////////////////////////////////////////////////
using namespace Metrics;
//using namespace std;
const int MIN_ITERATOR_NUM = 4;
GlobalMetrics::GlobalMetrics(const char* fileName)
{
	GDALAllRegister();
	GDALDataset *p = (GDALDataset*)GDALOpen(fileName,GA_ReadOnly);
	if (p==NULL)
	{
		m_fileisOK = false;
		m_fileName = "";
	}
	else
	{
		m_fileisOK = true;
		m_fileName = fileName;
		m_dataset = p;
	}
	hasRefFile = false;
	m_rmean = NULL;
	m_rstd = NULL;
	V2_CC = NULL;
	hasV2_CC = false;
	V8_MSE = NULL;
	hasV8_MSE = false;
}
GlobalMetrics::~GlobalMetrics()
{
	if (m_dataset != NULL && m_fileisOK == true)
	{
		GDALClose(m_dataset);
	}
	delete[] m_pBandMean;
	delete[] m_pBandStd;
}
int GlobalMetrics::SetRefImage(const char* r_filename)
{
	m_prdataset = (GDALDataset*)GDALOpen(r_filename,GA_ReadOnly);
	//GDALDataset* pr_file = (GDALDataset*)GDALOpen(r_file.c_str(),GA_ReadOnly);
	if (m_prdataset != NULL)
	{
		int r_nb = m_prdataset->GetRasterCount();
		int r_xsize = m_prdataset->GetRasterXSize();
		int r_ysize = m_prdataset->GetRasterYSize();
		if (r_nb == m_iBandCount && r_xsize == m_iXsize 
			&& r_ysize == m_iYsize)
		{
			m_rmean = new double [m_iBandCount];
			m_rstd = new double[m_iBandCount];
			for (int i=1; i<=m_iBandCount; i++)  //获取每个波段的均值和标准差  
			{  
				double dMaxValue, dMinValue;  
				m_prdataset->GetRasterBand(i)->ComputeStatistics(FALSE, &dMinValue, &dMaxValue,   
					m_rmean+(i-1), m_rstd+(i-1), NULL, NULL);  
			}
			hasRefFile = true;
			return EXE_OK;
		}
		else
		{
			hasRefFile = false;
			delete[] m_rmean;
			delete[] m_rstd;
			GDALClose(m_prdataset);
			return EXE_WRONG;
		}
	}
	else	
	{
		hasRefFile = false;
		if (m_rmean != NULL)
		{
			delete[] m_rmean;
		}
		if (m_rstd != NULL)
		{
			delete[] m_rstd;
		}	
		if (m_prdataset != NULL)
		{
			GDALClose(m_prdataset);
		}
		return EXE_WRONG;
	}
}
double* GlobalMetrics::Gaussion_Function(double sigma,int L /* = 11 */) const
{
	if (L%2==0)
	{
		return NULL;
	}
	double* gaussionkernel = new double[L];
	double sum = 0.0;
	int half = L/2;
	for(int i = 0;i<L;++i)
	{
		gaussionkernel[i] = 1.0/(sqrt(2*M_PI)*sigma)*exp(-((double)i-half)*((double)i-half)/(2.0*sigma*sigma));
		sum+=gaussionkernel[i];
	}
	for (int i = 0;i<L;++i)
	{
		gaussionkernel[i]/=sum;
	}
	return gaussionkernel;
}
int GlobalMetrics::PreProcessData()
{  
	if (m_dataset == NULL || m_fileisOK == false)  
	{  
		//if (m_pProcess != NULL)  
		//	m_pProcess->SetMessage("输入文件不能打开！");  
		return 	EXE_WRONG;
	}
	m_iBandCount = m_dataset->GetRasterCount();  
	m_pBandMean = new double [m_iBandCount];  
	m_pBandStd = new double [m_iBandCount];
	m_dMax = new double[m_iBandCount];
	m_dMin = new double[m_iBandCount];
	m_iXsize = m_dataset->GetRasterXSize();
	m_iYsize = m_dataset->GetRasterYSize();
	for (int i=1; i<=m_iBandCount; i++)  //获取每个波段的均值和标准差  
	{  
		m_dataset->GetRasterBand(i)->ComputeStatistics(FALSE, m_dMin+i-1, m_dMax+i-1,   
			m_pBandMean+(i-1), m_pBandStd+(i-1), NULL, NULL);  
	}  
	return EXE_OK;  
}
double* GlobalMetrics::GetBandMean() const
{
	return m_pBandMean;
}
double* GlobalMetrics::GetBandStd() const
{
	return m_pBandStd;
}
int GlobalMetrics::GetBandCount() const
{
	return m_iBandCount;
}
double* GlobalMetrics::GetAG() const
{
	double* AG = new double[m_iBandCount];
	double** line1 = new double*[m_iBandCount];
	double** line2 = new double*[m_iBandCount];
	//#pragma omp parallel for
	//#pragma omp parallel for reduction(+:sum)
	for (int i = 0;i<m_iBandCount;++i)
	{	
		double AGForBand = 0;
		line1[i] = new double[m_iXsize];
		m_dataset->GetRasterBand(i+1)->
			RasterIO(GF_Read,0,0,m_iXsize,1,line1[i],m_iXsize,1,GDT_Float64,0,0);
		for (int j = 0;j<m_iYsize-1;++j)
		{
			line2[i] = new double[m_iXsize];
			double temp = 0;
			m_dataset->GetRasterBand(i+1)->
				RasterIO(GF_Read,0,j+1,m_iXsize,1,line2[i],m_iXsize,1,GDT_Float64,0,0);
			//cout<<line2[1000]<<endl;
			for (int k = 0;k<m_iXsize-1;++k)
			{
				temp += sqrt(((line1[i][k]-line2[i][k])*(line1[i][k]-line2[i][k])+
					(line1[i][k]-line1[i][k+1])*(line1[i][k]-line1[i][k+1]))/2);
			}
			for (int k =0;k<m_iXsize;++k)
			{
				line1[i][k] = line2[i][k];
			}
			delete[] line2[i];
			line2[i]=NULL;
			AGForBand += temp;
		}
		AG[i] = AGForBand/((m_iXsize-1)*(m_iYsize - 1));
		//cout<<AG<<endl;
		delete[] line1[i];
		line1[i] = NULL;
	}
	delete[] line1;
	delete[] line2;
	return AG;
}
double* Metrics::GlobalMetrics::GetCC() const
{
	if (hasRefFile)
	{
		double *CC = new double[m_iBandCount];
		for (int b = 0;b<m_iBandCount;++b)
		{
			
			GDALRasterBand* pfBandDataset = m_dataset->GetRasterBand(b+1);
			GDALRasterBand* prBandDataset = m_prdataset->GetRasterBand(b+1);
			//////////////////////////////////////////////////////////////////////////
			double tempup = 0;
			double tempdown = 0;
			//////////////////////////////////////////////////////////////////////////
			double temp1 = 0;
			double temp2 = 0;
			double* r_line;
			double* f_line;
			for (int i = 0;i<m_iYsize;++i)
			{
				f_line = new double[m_iXsize];
				r_line = new double[m_iXsize];
				
				pfBandDataset->RasterIO(GF_Read,0,i,m_iXsize,1,f_line,m_iXsize,1,GDT_Float64,0,0);
				prBandDataset->RasterIO(GF_Read,0,i,m_iXsize,1,r_line,m_iXsize,1,GDT_Float64,0,0);
				
				for (int j = 0;j<m_iXsize;++j)
				{
					//////////////////////////////////////////////////////////////////////////
					tempup+=(f_line[j]-m_pBandMean[b])*(r_line[j]-m_rmean[b]);
					tempdown += (f_line[j]-m_pBandMean[b])*(f_line[j]-m_pBandMean[b])*
						(r_line[j]-m_rmean[b])*(r_line[j]-m_rmean[b]);
					//////////////////////////////////////////////////////////////////////////
					temp1+=(f_line[j]-m_pBandMean[b])*(f_line[j]-m_pBandMean[b]);
					temp2 +=(r_line[j]-m_rmean[b])*(r_line[j]-m_rmean[b]);
				}
				delete[] r_line;
				r_line = NULL;
				delete[] f_line;
				f_line =NULL;
			}
			CC[b] = tempup/(sqrt(temp1*temp2));
		}
		//V2_CC = CC;
		//hasV2_CC = true;
		return CC;
		
	}
	else
	{
		return NULL;
	}
}
double* Metrics::GlobalMetrics::GetMSE()
{
	if (hasRefFile)
	{
		double *MSE = new double[m_iBandCount];
		for (int b = 0;b<m_iBandCount;++b)
		{

			GDALRasterBand* pfBandDataset = m_dataset->GetRasterBand(b+1);
			GDALRasterBand* prBandDataset = m_prdataset->GetRasterBand(b+1);
			double tempup = 0;
			double* r_line;
			double* f_line;
			for (int i = 0;i<m_iYsize;++i)
			{
				f_line = new double[m_iXsize];
				r_line = new double[m_iXsize];

				pfBandDataset->RasterIO(GF_Read,0,i,m_iXsize,1,f_line,m_iXsize,1,GDT_Float64,0,0);
				prBandDataset->RasterIO(GF_Read,0,i,m_iXsize,1,r_line,m_iXsize,1,GDT_Float64,0,0);

				for (int j = 0;j<m_iXsize;++j)
				{
					tempup+=(f_line[j]-r_line[j])*(f_line[j]-r_line[j]);
				}
				delete[] r_line;
				r_line = NULL;
				delete[] f_line;
				f_line =NULL;
			}
			MSE[b] = tempup/(m_iXsize*m_iYsize);
		}
		V8_MSE = MSE;
		hasV8_MSE = true;
		return MSE;
	}
	else
	{
		return NULL;
	}
}
double* GlobalMetrics::GetNLSE() const
{
	if (hasRefFile)
	{
		double *NLSE = new double[m_iBandCount];
		for (int b = 0;b<m_iBandCount;++b)
		{

			GDALRasterBand* pfBandDataset = m_dataset->GetRasterBand(b+1);
			GDALRasterBand* prBandDataset = m_prdataset->GetRasterBand(b+1);
			double tempup = 0;
			double tempdown = 0;
			double* r_line;
			double* f_line;
			for (int i = 0;i<m_iYsize;++i)
			{
				f_line = new double[m_iXsize];
				r_line = new double[m_iXsize];

				pfBandDataset->RasterIO(GF_Read,0,i,m_iXsize,1,f_line,m_iXsize,1,GDT_Float64,0,0);
				prBandDataset->RasterIO(GF_Read,0,i,m_iXsize,1,r_line,m_iXsize,1,GDT_Float64,0,0);

				for (int j = 0;j<m_iXsize;++j)
				{
					tempup+=(f_line[j]-r_line[j])*(f_line[j]-r_line[j]);
					tempdown+=(r_line[j]*r_line[j]);
				}
				delete[] r_line;
				r_line = NULL;
				delete[] f_line;
				f_line =NULL;
			}
			NLSE[b] = tempup/(tempdown);
		}
		return NLSE;
	}
	else
	{
		return NULL;
	}
}
double* Metrics::GlobalMetrics::GeneratePSNR()
{
	if (!hasV8_MSE)
	{
		double* xx = GetMSE();
		if (xx == NULL)
		{
			return NULL;
		}
	}
	double* PSNR = new double[m_iBandCount];
	for (int i = 0;i<m_iBandCount;++i)
	{
		//用最大值代替可能最大值！
		PSNR[i] = 10.00*log10(m_dMax[i]*m_dMax[i]/V8_MSE[i]);
	}
	return PSNR;
}
double GlobalMetrics::GenerateQ4perBlock(double* data_f,double*data_r,int L)
{
	double sum_z1z2=0;
	double z1abs = 0;
	double z1mean[4] = {0.0,0.0,0.0,0.0};
	double z2abs = 0;
	double z2mean[4] = {0.0,0.0,0.0,0.0};
	double z1z2abs = 0;
	double z1z2mean[4] = {0.0,0.0,0.0,0.0};
	for (int i =0;i<L;++i)
	{
		for (int j = 0;j<L;++j)
		{
			for (int b = 0;b<m_iBandCount;++b)
			{
				z1abs += data_f[i*L+b*L*L+j]*data_f[i*L+b*L*L+j];
				z1mean[b] += data_f[i*L+b*L*L+j];

				z2abs += data_r[i*L+b*L*L+j]*data_r[i*L+b*L*L+j];
				z2mean[b] += data_r[i*L+b*L*L+j];

				z1z2abs+=data_r[i*L+b*L*L+j]*data_f[i*L+b*L*L+j];		
			}
		}
	}
	double Z1 = (z1mean[0]*z1mean[0]+z1mean[1]*z1mean[1]+z1mean[2]*z1mean[2]+z1mean[3]*z1mean[3])/
		((double)L*L*L*(double)L);
	double Z2 = (z2mean[0]*z2mean[0]+z2mean[1]*z2mean[1]+z2mean[2]*z2mean[2]+z2mean[3]*z2mean[3])/
		((double)L*L*L*(double)L);
	double V1 = z1abs/((double)L*L)-Z1;
	double V2 = z2abs/((double)L*L)-Z2;

	double V1V2 = z1z2abs/((double)L*L)-
		((z1mean[0]*z2mean[0]+z1mean[1]*z2mean[1]+z1mean[2]*z2mean[2]+z1mean[3]*z2mean[3])/
		((double)L*L*L*(double)L));
	double Q4 = 4*abs(V1V2)*sqrt(Z1)*sqrt(Z2)/((V1+V2)*(Z1+Z2));
	return Q4;
}
double GlobalMetrics::GenerateQ4(int L)
{
	if (hasRefFile && m_iBandCount == 4)
	{
	
		int reX = (m_iXsize%L)/2;
		int reY = (m_iYsize%L)/2;
		int bandMap[4] = {1,2,3,4};

		int n_xBlock = m_iXsize / L;
		int n_yBlock = m_iYsize / L;
		double Q4  =0;
		for (int i = 0;i<n_xBlock;++i)
		{
			for (int j =0;j<n_yBlock;++j)
			{
				double* f= new double[L*L*m_iBandCount];
				double* r = new double[L*L*m_iBandCount];
				m_dataset->RasterIO(GF_Read,i*L,j*L,L,L,f,L,L,GDT_Float64,4,bandMap,0,0,0);
				m_prdataset->RasterIO(GF_Read,i*L,j*L,L,L,r,L,L,GDT_Float64,4,bandMap,0,0,0);

				Q4+=GenerateQ4perBlock(f,r,L);
				delete[] f;
				delete[] r;
			}
		}
		Q4 = Q4/(n_xBlock*n_yBlock);
		return Q4;
	}
	else
	{
		return -1.0;
	}
	//return NULL;
}
double* GlobalMetrics::GenerateUIQIperBlock(double* data_f,double* data_r,int L)
{
	double* re = new double[m_iBandCount];
	for(int b =0;b<m_iBandCount;++b)
	{
		double ur = 0;
		double uf = 0;
		for (int i = 0;i<L;++i)
		{
			for (int j =0;j<L;++j)
			{
				uf += data_f[b*L*L+i*L+j];
				ur += data_r[b*L*L+i*L+j];
			}
		}
		uf/=(L*L);
		ur/=(L*L);
		double vf = 0;
		double vr = 0;
		double cov = 0;
		for (int i = 0;i<L;++i)
		{
			for (int j =0;j<L;++j)
			{
				cov += (data_f[b*L*L+i*L+j]-uf)*(data_r[b*L*L+i*L+j]-ur);
				vf += (data_f[b*L*L+i*L+j]-uf)*(data_f[b*L*L+i*L+j]-uf);
				vr += (data_r[b*L*L+i*L+j]-ur)*(data_r[b*L*L+i*L+j]-ur);
			}
			cov /= (L*L);
			vf /= (L*L);
			vr /= (L*L);
		}
		if ((vf+vr) == 0||(uf*uf+ur*ur) == 0)
		{
			re[b] = 1;
		}
		else
		{
			re[b] = (4*cov*uf*ur)/((uf*uf+ur*ur)*(vf+vr));
		}
	}
	return re;
}
double* GlobalMetrics::GenerateUIQI(int L)
{
	if (hasRefFile)
	{
		double* UIQI = new double[m_iBandCount];
		int reX = (m_iXsize % L)/2;
		int reY = (m_iYsize % L)/2;
		int* bandMap = new int[m_iBandCount];
		for (int i =0;i<m_iBandCount;i++)
		{
			bandMap[i] = i+1;
			UIQI[i] = 0;
		}
		int n_xBlock = m_iXsize / L;
		int n_yBlock = m_iYsize / L;
		//double Q4  =0;
		for (int i = 0;i<n_xBlock;++i)
		{
			for (int j =0;j<n_yBlock;++j)
			{
				double* f= new double[L*L*m_iBandCount];
				double* r = new double[L*L*m_iBandCount];
				m_dataset->RasterIO(GF_Read,i*L,j*L,L,L,f,L,L,GDT_Float64,4,bandMap,0,0,0);
				m_prdataset->RasterIO(GF_Read,i*L,j*L,L,L,r,L,L,GDT_Float64,4,bandMap,0,0,0);

				double* re = GenerateUIQIperBlock(f,r,L);

				for (int b = 0;b<m_iBandCount;++b)
				{
					UIQI[b]+=re[b];
				}

				delete[] re;
				delete[] f;
				delete[] r;
			}
		}
		for (int b = 0;b<m_iBandCount;++b)
		{
			UIQI[b]/=(n_xBlock*n_yBlock);
		}
		delete[] bandMap;
		return UIQI;
	}
	else
	{
		return NULL;
	}
}
double GlobalMetrics::GenerateRASE()
{
	if (!hasV8_MSE)
	{
		double* xx = GetMSE();
		if (xx == NULL)
		{
			return -1.0;
		}
	}
	//double RASE = 0.0;
	if (hasRefFile)
	{
		//std::cout<<"RASE"<<this<<std::endl;
		

		double mean = 0;
		double MSE = 0;
		for (int i = 0;i<m_iBandCount;++i)
		{
			mean += m_rmean[i];
			MSE += V8_MSE[i];
		}
		mean /= m_iBandCount;
		double RASE = 1.0/mean*sqrt(1.0/m_iBandCount*MSE);
		//std::cout<<"RASE"<<this<<std::endl;	
		return RASE;
	}
	else
	{
		return -1.0;
	}

}
double* GlobalMetrics::GenerateRM()
{
	//if (hasRefFile)
	//{
	//	double *PM = new double[m_iBandCount];
	//	for (int b = 0;b<m_iBandCount;++b)
	//	{

	//		GDALRasterBand* pfBandDataset = m_dataset->GetRasterBand(b+1);
	//		GDALRasterBand* prBandDataset = m_prdataset->GetRasterBand(b+1);
	//		double temp = 0;
	//		double* r_line;
	//		double* f_line;
	//		for (int i = 0;i<m_iYsize;++i)
	//		{
	//			f_line = new double[m_iXsize];
	//			r_line = new double[m_iXsize];

	//			pfBandDataset->RasterIO(GF_Read,0,i,m_iXsize,1,f_line,m_iXsize,1,GDT_Float64,0,0);
	//			prBandDataset->RasterIO(GF_Read,0,i,m_iXsize,1,r_line,m_iXsize,1,GDT_Float64,0,0);

	//			for (int j = 0;j<m_iXsize;++j)
	//			{
	//				temp += r_line[j]/f_line[j]-1.0;
	//			}
	//			delete[] r_line;
	//			r_line = NULL;
	//			delete[] f_line;
	//			f_line =NULL;
	//		}
	//		PM[b] = temp/(m_iXsize*m_iYsize);
	//	}
	//	return PM;
	//}
	//else
	//{
	//	return NULL;
	//}
	if (hasRefFile)
	{
		double *RM = new double[m_iBandCount];
		for (int b = 0;b<m_iBandCount;++b)
		{

			GDALRasterBand* pfBandDataset = m_dataset->GetRasterBand(b+1);
			GDALRasterBand* prBandDataset = m_prdataset->GetRasterBand(b+1);
			double temp = 0;
			double* r_line;
			double* f_line;
			for (int i = 0;i<m_iYsize;++i)
			{
				f_line = new double[m_iXsize];
				r_line = new double[m_iXsize];

				pfBandDataset->RasterIO(GF_Read,0,i,m_iXsize,1,f_line,m_iXsize,1,GDT_Float64,0,0);
				prBandDataset->RasterIO(GF_Read,0,i,m_iXsize,1,r_line,m_iXsize,1,GDT_Float64,0,0);

				for (int j = 0;j<m_iXsize;++j)
				{
					if (f_line[i] != 0)
					{
						temp+=(r_line[i]-f_line[i])/f_line[i];
					}
					//tempdown+=(r_line[j]*r_line[j]);
				}
				delete[] r_line;
				r_line = NULL;
				delete[] f_line;
				f_line =NULL;
			}
			RM[b] = temp/((double)m_iXsize)/((double)m_iYsize);
		}
		return RM;
	}
	else
	{
		return NULL;
	}
}
double* GlobalMetrics::GenerateRMSE()
{
	//double* RMSE;
	if (!hasV8_MSE)
	{
		double* xx = GetMSE();
		if (xx == NULL)
		{
			return NULL;
		}
	}
	
	if (hasRefFile)
	{
		double* RMSE = new double[m_iBandCount];
		for (int i = 0;i<m_iBandCount;++i)
		{
			RMSE[i] = sqrt(V8_MSE[i]);
		}
		return RMSE;
	}
	else
	{
		return NULL;
	}
}
double* GlobalMetrics::GetSD()
{
	double* SD = new double[m_iBandCount];
	for (int i = 0;i<m_iBandCount;++i)
	{
		SD[i] = m_pBandStd[i];
	}
	//std::cout<<SD<<std::endl;
	return SD;
}
double* GlobalMetrics::GenerateSF() const
{
	double* SF = new double[m_iBandCount];
	double** line1 = new double*[m_iBandCount];
	double** line2 = new double*[m_iBandCount];
	//#pragma omp parallel for
	//#pragma omp parallel for reduction(+:sum)
	for (int i = 0;i<m_iBandCount;++i)
	{	
		line1[i] = new double[m_iXsize];
		line2[i] = new double[m_iXsize];

		double tempRF = 0;
		double tempCF = 0;
		m_dataset->GetRasterBand(i+1)->
			RasterIO(GF_Read,0,0,m_iXsize,1,line1[i],m_iXsize,1,GDT_Float64,0,0);
		for (int k = 0;k<m_iXsize-1;++k)
		{
			tempCF+=(line1[i][k+1]-line1[i][k])*(line1[i][k+1]-line1[i][k]);
		}
		for (int j = 0;j<m_iYsize-1;++j)
		{
			m_dataset->GetRasterBand(i+1)->
				RasterIO(GF_Read,0,j+1,m_iXsize,1,line2[i],m_iXsize,1,GDT_Float64,0,0);
			//cout<<line2[1000]<<endl;
			for (int k = 0;k<m_iXsize-1;++k)
			{
				tempCF+=(line2[i][k+1]-line2[i][k])*(line2[i][k+1]-line2[i][k]);
			}
			for (int k =0;k<m_iXsize;++k)
			{
				tempRF += (line2[i][k]-line1[i][k])*(line2[i][k]-line1[i][k]);
			}
			for (int k =0;k<m_iXsize;++k)
			{
				line1[i][k] = line2[i][k];
			}
		}

		SF[i] = sqrt((tempRF+tempCF)/((double)m_iXsize*m_iYsize));
		//cout<<AG<<endl;
		delete[] line2[i];
		line2[i]= NULL;
		delete[] line1[i];
		line1[i] = NULL;
	}
	delete[] line1;
	delete[] line2;
	return SF;
}
double* GlobalMetrics::GenerateSNR() const
{
	if (hasRefFile)
	{
		double *SNR = new double[m_iBandCount];
		for (int b = 0;b<m_iBandCount;++b)
		{

			GDALRasterBand* pfBandDataset = m_dataset->GetRasterBand(b+1);
			double tempup = 0;
			double* f_line;
			for (int i = 0;i<m_iYsize;++i)
			{
				f_line = new double[m_iXsize];

				pfBandDataset->RasterIO(GF_Read,0,i,m_iXsize,1,f_line,m_iXsize,1,GDT_Float64,0,0);

				for (int j = 0;j<m_iXsize;++j)
				{
					tempup+=(f_line[j])*(f_line[j]);
				}
				delete[] f_line;
				f_line = NULL;
			}
			SNR[b] = 10.0*log10(tempup/(V8_MSE[b]*(double)m_iXsize*m_iYsize));
		}
		return SNR;
	}
	else
	{
		return NULL;
	}
}

double GlobalMetrics::GenerateSAM()
{
	if (hasRefFile)
	{
		double SAM = 0;
		double temp = 0;
		int xx = 0;
		double* r_line;
		double* f_line;
		int* bandMap = new int[m_iBandCount];
		for (int i = 0;i<m_iBandCount;++i)
		{
			bandMap[i] = i+1;
		}
		for (int i = 0;i<m_iYsize;++i)
		{
			f_line = new double[m_iXsize*m_iBandCount];
			r_line = new double[m_iXsize*m_iBandCount];
			
			m_dataset->RasterIO(GF_Read,0,i,m_iXsize,1,f_line,m_iXsize,1,
				GDT_Float64,m_iBandCount,bandMap,0,0,0);
			m_prdataset->RasterIO(GF_Read,0,i,m_iXsize,1,r_line,m_iXsize,1,
				GDT_Float64,m_iBandCount,bandMap,0,0,0);

			for (int j = 0;j<m_iXsize;++j)
			{
				double tempup = 0;
				double tempdownL = 0;
				double tempdownR= 0;
				for (int b =0;b<m_iBandCount;++b)
				{
					tempup += f_line[j+b*m_iXsize]*r_line[j+b*m_iXsize];
					tempdownL += f_line[j+b*m_iXsize]*f_line[j+b*m_iXsize];
					tempdownR += r_line[j+b*m_iXsize]*r_line[j+b*m_iXsize];
				}
				if (tempdownL == 0 || tempdownR == 0)
				{
					temp += 0;
					xx++;
				}
				else
				{
					temp += acos(tempup/(sqrt(tempdownL*tempdownR)));
				}
			}
			delete[] r_line;
			r_line = NULL;
			delete[] f_line;
			f_line =NULL;
		}
		
		SAM = temp/((double)m_iXsize*(double)m_iYsize-xx);	
		return SAM;
	}
	else
	{
		return -1;
	}
}
double* GlobalMetrics::GenerateSSIMAndQILV(double K1 /* = 0.01 */,double K2 /* = 0.03 */,
	double sigma/* =1.5 */,int L /* = 11 */,string Out_SSIMImage/*=""*/) 
{
	int outer = L/2;
	double* Gaussion1D= Gaussion_Function(sigma,L);
	int* bandMap = new int[m_iBandCount];
	for (int i = 0;i<m_iBandCount;++i)
	{
		bandMap[i] = i+1;
	}
	double* f_tempData;
	double* r_tempData;

	double* C1 = new double[m_iBandCount];
	double* C2 = new double[m_iBandCount];

	double* SSIM = new double[m_iBandCount*2];

	for(int i = 0;i<m_iBandCount;++i)
	{
		//C1[i] = K1*m_dMax[i]*K1*m_dMax[i];
		C1[i] = K1*K1*255*255;
		//C2[i] = K2*m_dMax[i]*K2*m_dMax[i];
		C2[i] = K2*K2*255*255;
		
		SSIM[i] = 0;
	}
	f_tempData = new double[L*m_iXsize*m_iBandCount];
	r_tempData = new double[L*m_iXsize*m_iBandCount];
	//f_tempData_sq = new double[L*m_iXsize*m_iBandCount];
	//r_tempData_sq = new double[L*m_iXsize*m_iBandCount];
	//fr_tempData = new double[L*m_iXsize*m_iBandCount];

	m_dataset->RasterIO(GF_Read,0,0,m_iXsize,L,f_tempData,m_iXsize,L,
		GDT_Float64,m_iBandCount,bandMap,0,0,0);
	m_prdataset->RasterIO(GF_Read,0,0,m_iXsize,L,r_tempData,m_iXsize,L,
		GDT_Float64,m_iBandCount,bandMap,0,0,0);
	double t1 = omp_get_wtime();
	
	double* r_Var = new double[m_iBandCount];
	double* r_Var_sq = new double[m_iBandCount];

	double* f_Var = new double[m_iBandCount];
	double* f_Var_sq = new double[m_iBandCount];

	double* cov = new double[m_iBandCount];

	for (int i = 0;i<m_iBandCount;++i)
	{
		r_Var[i] = 0;
		r_Var_sq[i] = 0;
		f_Var[i] = 0;
		f_Var_sq[i] = 0;
		cov[i] = 0;
	}
	//Get the Cores of the CPU
	int ncore = omp_get_num_procs();
	int max_tn = (m_iYsize-2*outer)/MIN_ITERATOR_NUM;
	int tn = max_tn>2*ncore?2*ncore:max_tn;

	for (int i = 0+outer;i<m_iYsize-outer;++i)
	{
		#pragma omp parallel for if(tn>1) num_threads(tn)
		for (int j = 0+outer;j<m_iXsize-outer;++j)
		{
			//Allocate memory
			//默认BSQ
			double* f = new double[L*L*m_iBandCount];
			double* r = new double[L*L*m_iBandCount];

			double* f_sq = new double[L*L*m_iBandCount];
			double* r_sq = new double[L*L*m_iBandCount];
			double* fr = new double[L*L*m_iBandCount];

			for (int x = 0;x<L;++x)
			{
				for (int y = 0;y<L;++y)
				{
					for (int b = 0;b<m_iBandCount;++b)
					{
						f[L*L*b+y*L+x] = f_tempData[y*m_iXsize+(j-outer+x)+b*L*m_iXsize];
						r[L*L*b+y*L+x] = r_tempData[y*m_iXsize+(j-outer+x)+b*L*m_iXsize];
					}
				}
			}
			for (int b = 0; b<m_iBandCount;++b)
			{
				for(int m = 0;m<L*L;++m)
				{
					f_sq[b*L*L+m] = f[b*L*L+m]*f[b*L*L+m];
					r_sq[b*L*L+m] = r[b*L*L+m]*r[b*L*L+m];
					fr[b*L*L+m] = r[b*L*L+m]*f[b*L*L+m];
				}
				double ux = Convolve(Gaussion1D,f+b*L*L,L);
				double uy = Convolve(Gaussion1D,r+b*L*L,L);
				double sx = Convolve(Gaussion1D,f_sq+b*L*L,L);
				double sy = Convolve(Gaussion1D,r_sq+b*L*L,L);
				double xy = Convolve(Gaussion1D,fr+b*L*L,L);

				double cxy = xy - ux*uy;
				double dx = sx-ux*ux;
				double dy = sy-uy*uy;
				//if ((ux*uy+uy*uy+C1[b]) == 0 || (dx+dy+C2[b]) == 0)
				//{
				//	std::cout<<"Error"<<std::endl;
				//
				SSIM[b]+=(2*ux*uy+C1[b])*(2*cxy+C2[b])/((ux*ux+uy*uy+C1[b])*(dx+dy+C2[b]));

				f_Var[b]+=dx;
				r_Var[b]+=dy;

				f_Var_sq[b]+=dx*dx;
				r_Var_sq[b]+=dy*dy;

				cov[b] += dx*dy;

			}
			delete[] f;
			delete[] r;
			delete[] f_sq;
			delete[] r_sq;
			delete[] fr;
			f = NULL;
			r = NULL;
			f_sq = NULL;
			r_sq = NULL;
			fr = NULL;
		}

		double* f_line = new double[m_iXsize*m_iBandCount];
		double* r_line = new double[m_iXsize*m_iBandCount];
		m_dataset->RasterIO(GF_Read,0,i+outer,m_iXsize,1,f_line,m_iXsize,1,
			GDT_Float64,m_iBandCount,bandMap,0,0,0);
		m_prdataset->RasterIO(GF_Read,0,i+outer,m_iXsize,1,r_line,m_iXsize,1,
			GDT_Float64,m_iBandCount,bandMap,0,0,0);
		for (int b = 0;b<m_iBandCount;++b)
		{
			for(int p = 0;p<L;++p)
			{
				for (int q = 0;q<m_iXsize;++q)
				{
					if (p < L-1)
					{
						r_tempData[b*m_iXsize*L+p*m_iXsize+q] = r_tempData[b*m_iXsize*L+(p+1)*m_iXsize+q];
						f_tempData[b*m_iXsize*L+p*m_iXsize+q] = f_tempData[b*m_iXsize*L+(p+1)*m_iXsize+q];

					}
					else
					{
						r_tempData[b*m_iXsize*L+p*m_iXsize+q] = r_line[b*m_iXsize+q];
						f_tempData[b*m_iXsize*L+p*m_iXsize+q] = f_line[b*m_iXsize+q];

					}					
				}
			}
		}

		delete[] f_line;
		delete[] r_line;
		f_line = NULL;
		r_line = NULL;
	}
	double t2 = omp_get_wtime();
	std::cout<<t2-t1<<"s"<<std::endl;
	delete[] f_tempData;
	delete[] r_tempData;

	f_tempData = NULL;
	r_tempData = NULL;

	

	double siz = ((m_iXsize-L+1)*(m_iYsize-L+1));
	for (int i = 0;i<m_iBandCount;++i)
	{
		double uvr = r_Var[i]/siz;
		double uvf = f_Var[i]/siz;

		double sdr = (r_Var_sq[i]-2*uvr*r_Var[i]+siz*uvr*uvr)/(siz-1);
		double sdf = (f_Var_sq[i]-2*uvf*f_Var[i]+siz*uvf*uvf)/(siz-1);
		double c = (cov[i]-uvr*f_Var[i]-uvf*r_Var[i]+siz*uvr*uvf)/(siz-1);

		SSIM[i+m_iBandCount] = (4*uvf*uvr*c)/((uvr*uvr+uvf*uvf)*(sdr+sdf));
	}

	for (int i = 0;i<m_iBandCount;++i)
	{
		SSIM[i]/=siz;
	}
	delete[] C1;
	delete[] C2;
	delete[] Gaussion1D;
	delete[] bandMap;
	delete[] r_Var;
	delete[] f_Var;
	delete[] r_Var_sq;
	delete[] f_Var_sq;
	delete[] cov;
	return SSIM;	

}
double GlobalMetrics::Convolve(double* filter,double* in,int L)
{
	double* temp = new double[L];
	for (int i = 0;i<L;++i)
	{
		temp[i] = 0;
		for (int j = 0;j<L;++j)
		{
			temp[i]+=filter[j]*in[i*L+j];
		}
	}
	double re = 0;
	for (int i = 0;i<L;++i)
	{
		re+=temp[i]*filter[i];
	}
	delete[] temp;
	temp = NULL;
	return re;
}
