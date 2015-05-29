/*******************************************************
*
*Time:2013-10-16
*Project:全局指标计算
*Purpose:计算图像全局指标
*Author:王继成
*Copyright(c)2013年，wangjicheng11@163.com
*Describe:提供图像全局指标
*******************************************************/


#ifndef GLOBALMETRICS_HH
#define GLOBALMETRICS_HH
#include <string>
using std::string;
/** 
* \file GlobalMetrics.h  
* @brief 图像全局指标计算
* 
* 用于计算图像全局指标 
*/  

#include <gdal_priv.h>

namespace Metrics
{
	/** 
	* \class 全局指标计算类，GlobalMetrics.h  
	* @brief 主成分变换类Principal Component Analysis 
	* 
	*/
	const int EXE_OK = 1;
	const int EXE_WRONG = -1;
	class GlobalMetrics
	{

	public:
		/**
		 *@brief:构造函数,初始化各类参数
		 *@param fileName:影像文件
		 */
		GlobalMetrics(const char* fileName);
		/**
		 *@brief:析构函数
		 */
		~GlobalMetrics();
		/**
		 *@brief 设置参考影像
		 *@param r_filename 参考影像路径
		 *@return 成功返回ExE_OK,失败返回EXE_ERROR
		 */
		int SetRefImage(const char* r_filename);
		/**
		 *@brief Test method
		 */
		int Test()
		{
			if (PreProcessData() == EXE_OK)
			{
				//double* xx = Gaussion_Function(1.5);
				return EXE_OK;
			}
			else
			{
				return EXE_WRONG;
			}

		}
		//Variable for some basic index

		/**
		 *@brief 获取每波段的平均值
		 *@return 返回波段均值
		 */
		double* GetBandMean() const;
		/**
		 *@brief 获取每波段的标准差
		 *@return 返回标准差
		 */
		double* GetBandStd() const;
		/**
		 *@brief 获取波段数
		 *@return 返回波段数
		 */
		int GetBandCount() const;
		/**
		 *@brief 生成高斯核
		 *@param sigma 必须，standard deviation!!!
		 *@param L,核长度，默认为11。即11！！！
		 *@return 高斯核函数
		 */
		double* Gaussion_Function(double sigma,int L = 11) const;
		/**
		 *@brief 一维卷积运算
		 *@param filter 高斯一维函数窗口
		 *@param in 输入
		 *@param L 宽度
		 */
		double Convolve(double* filter,double* in,int L);

		/**
		 *@brief 计算单块UIQI
		 *@param data 块数据(L*L)
		 *@param L 块大小
		 *@return UIQI for image block
		 */
		double* GenerateUIQIperBlock(double* data_f,double* data_r,int L);
		/**
		 *@brief 计算单块Q4
		 *@param data 块数据(L*L*bandcount)
		 *@param L 块大小
		 *@return Q4 for image block
		 */
		double GenerateQ4perBlock(double* data_f,double* data_r,int L);

		
		/************************************************************************/
		/* 计算指标                                                                     */
		/************************************************************************/
		/**
		 *@brief 1. 计算平均梯度AG（Average Gradient）,
		 *@return 返回AG值
		 */
		double* GetAG() const;

		/**
		 *@brief 2. 计算两种影像的相关系数CC(Correlation Coefficient)
		 *@return CC per band
		 */
		double* GetCC() const;

		/**
		 *@brief 8. 计算均方差MSE（Mean square error)
		 *@return MSE per band
		 */
		double* GetMSE();

		/**
		 *@brief 9. 计算NLSE(normalised least square error)
		 *@return NLSE per band
		 */
		double* GetNLSE() const;

		/**
		 *@brief 10. Calculate PSNR (Peak signal-to-noise ratio)
		 *@return PSNR per band;
		 */
		double* GeneratePSNR();
		 /**
		 *@brief 11. Calculate Q4 (Quaternion theory-based quality index)
		 *@param L the size of block,default value is 10
		 *@return Q4 per band;
		 */
		double GenerateQ4(int L = 10);

		/**
		 *@brief 15. RASE(Relative average spectral error)
		 *@return A RASE for Image.Else Return -1.0 denote the Wrong Results.
		 */
		double GenerateRASE();
		/**
		 *@brief 16. RM(Relative shift in Mean)
		 *@return RM per band;
		 */
		double* GenerateRM();
		/**
		 *@brief 17. RMSE(Root of mean square error)
		 *@return RMSE per band;
		 */
		double* GenerateRMSE();

		/**
		 *@brief 19. SAM(Spectral angle mapper)
		 *@return SAM(Rad),  if Failure, return -1;
		 */
		double GenerateSAM();

		/**
		 *@brief 20. SD(Standard deviation)
		 *@return SD per band
		 */
		double* GetSD();
		/**
		 *@brief 21. SF(Spatial frequency)
		 *@return SF per band;
		 */
		double* GenerateSF() const;
		/**
		 *@brief 22. SNR(Signal-to-noise ratio)
		 *@return SNR per band;
		 */
		double* GenerateSNR() const;
		/**
		 *@brief 23.13. SSIM(Structure similarity index) And QILV(Quality index based on local variance)
		 *@param K1 构成C1的乘数，默认为0.01
		 *@param K2 构成C2的乘数，默认为0.03
		 *@param sigma 高斯标准差，默认为1.5
		 *@param L 高斯平滑窗口大小，默认为11
		 *@param Out_SSIMImage SSIM文件输出（暂时未实现）
		 *@return SSIM per band And QILV per band: first bandCount value are SSIM and the last bandCount value 
					are  QILV per band
		 */
		double* GenerateSSIMAndQILV(double K1 = 0.01,double K2 = 0.03,
			double sigma=1.5,int L = 11,string Out_SSIMImage = "");
		/**
		 *@brief 25.UIQI (Universal image quality index)
		 *@param L the size of block,default value is 10
		 *@return UIQI per band;
		 */
		double* GenerateUIQI(int L = 10);
	//For Methods	
	private:
		/**
		 *@brief 数据预处理，进行图像信息的统计等
		 *@return 返回执行成功与否！
		 */
		int PreProcessData();


	//For variables

	private:
		//CProcessBase* m_pProcess;
		const char* m_fileName;
		bool m_fileisOK;
		GDALDataset* m_dataset;
		int m_iBandCount;
		double* m_pBandMean;
		double* m_pBandStd;
		int m_iXsize;
		int m_iYsize;
		double* m_dMax;
		double* m_dMin;
		

		//Variables about RefImage
		const char* m_rfile;
		GDALDataset* m_prdataset;
		double* m_rmean;
		double* m_rstd;
		bool hasRefFile;

		//Variables for indexes
		double* V2_CC;
		bool hasV2_CC;

		double* V8_MSE;
		bool hasV8_MSE;

		
	};
}

#endif