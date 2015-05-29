/*********************************************************************************
*
* Time: 2014-02-15
* Project: 分类数据下采样
* Purpose: 对分类数据进行Majority Aggregation采样。
* Author: 王继成(Wang Jicheng, @Kevin)
* Copyright(c) 2014, wangjicheng11@163.com
* Describe: 提供限制行的Majority采用。
*
**********************************************************************************/

/**
* \file DownScale.h
* @brief 分类数据下采样
*
*/
#ifndef DOWNSCALE__HH
#define DOWNSCALE__HH
#include <gdal_priv.h>
#include "param.h"

/**
* \class DownScale
* @describe 影像尺度下推
* @Initia DownScale new = new DownScale(file);
*/
class DownScale
{
public:

	/**
	* @brief 构造函数
	* @param pszSrcFile       初始化文件路径
	*/
	DownScale(const char* pszSrcFile);

	/**
	* @brief 析构函数
	*/
	~DownScale();
public:
	/**
	* @brief 执行下推
	* @param pszOutFile        输出文件路径
	* @param msize             aggregate size
	* @return                  执行成功或错误
	*/
	int ExecuteAggregation(const char* pszOutFile,int msize);
	/**
	* @brief 执行聚合
	* @param pszOutFile        输出文件路径
	* @param msize             aggregate size（如果是需要变成250，则设置为250）
	* @return                  执行成功或错误
	*/
	int ExecuteAggregationEx(const char* pszOutFile,int msize);

	/**
	* @brief 执行具有优先级的下推
	* @param pszOutFile        输出文件路径
	* @param msize             aggregate size
	* @param option            选择定性(1 默认)或者定量的优先级算法
	* @return                  执行成功或错误
	*/
	int ExecuteAggregationWithPriority(const char* pszOutFile,int msize,int option);

	/**
	* @brief 执行下推
	* @param pszOutFile        输出文件路径
	* @param msize             aggregate size
	* @param option            选择定性(1 默认)或者定量的优先级算法
	* @return                  执行成功或错误
	*/
	int ExecuteAggregationWithPriorityEx(const char* pszOutFile,int msize,int option);
	/**
	* @brief 执行平滑的下推
	* @param pszOutFile        输出文件路径
	* @param msize             aggregate size
	* @param smoothwindowSize  smooth window size
	* @return                  执行成功或错误
	*/
	int ExecuteSmooth(const char* pszOutFile,int msize,int smoothwindowSize);

private:
	/**
	* @brief 预处理数据，获取文件信息
	* @return                 执行成功或错误
	*/
	int PreProcessData();
	/**
	* @brief 读取数据
	* @return                 数据第一波段，如果错误，返回NULL
	*/
	Byte* ReadData();

	/**
	* @brief 执行聚合单元的众数聚合（ 修正，直接使用影像数据指针）
	* @param oi               指针初始位置，行
	* @param oj               指针初始位置，列
	* @param data             影像数据
	* @param msize            单元大小，msize*msize
	* @return                 返回聚合结果
	*/
	Byte Marjority_Aggregation(int oi,int oj,Byte* data, int msize);

	//int Marjority_Aggregation();
	/**
	* @brief 执行众数平滑
	* @param data             数据指针数组，包含需要滤波的
	* @param size             单元大小
	* @param centerPriority   中间像元权重
	* @return                 返回聚合结果
	*/
	Byte Marjority_Smoothing(Byte** data, int size,int centerPriority);

	/**
	* @brief 执行众数聚合扩展，（测试版，及适用于30m分辨率的聚合）
	* @param oi              输入带计算的影像位置坐标
	* @param oj              输入带计算的影像位置
	* @param data            中间像素
	* @param msize           单元大小，msize*msize（250m的分辨率即25）
	*/
	Byte Marjority_AggregationEx(int oi,int oj,Byte* data, int msize);


	/**
	* @brief 执行聚合单元的众数聚合（修正，直接使用影像数据指针）
	* @param oi               指针初始位置，行
	* @param oj               指针初始位置，列
	* @param data             影像数据
	* @param msize            单元大小，msize*msize
	* @return                 返回聚合结果
	*/
	Byte Marjority_AggregationwithPriority1(int oi,int oj,Byte* data,int size);

	/**
	* @brief 执行优先级定性算法扩展测试版
	* @param oi
	* @param oj
	* @param data
	* @param msize
	* @return
	*/
	Byte Marjority_AggregationwithPriority1Ex(int oi,int oj,Byte* data,int size);
	/**
	* @brief 执行聚合单元的众数聚合（修正，直接使用影像数据指针）,对优先级进行量化
	* @param oi               指针初始位置，行
	* @param oj               指针初始位置，列
	* @param data             影像数据
	* @param msize            单元大小，msize*msize
	* @return                 返回聚合结果
	*/
	Byte Marjority_AggregationwithPriority2(int oi,int oj,Byte* data,int size);

	/**
	* @brief 执行定量优先级的众数聚合扩展版
	* @param oi               指针初始位置，行
	* @param oj               指针初始位置，列
	* @param data             影像数据
	* @param msize            单元大小，msize*msize
	* @return                 返回聚合结果
	*/
	Byte Marjority_AggregationwithPriority2Ex(int oi,int oj,Byte* data,int size);

	/**
	* @brief 排序操作（从小到大）
	* @param a                输入数组
	* @param n				  输入数组个数
	* @param id               输入序号（默认，0,1,2,3.......）
	* @param m                输入需要运算的元素个数（从头开始）
	*/
	void Sort(int* a,int n,int* id,int m);
private:
	const char* m_pszSrcFile;          /*<! 需要进行采样的文件路径 */
	//文件信息
	int m_iBandCount;                  /*<! 波段个数 */
	int m_iXsize;                      /*<! 栅格宽度 */
	int m_iYsize;                      /*<! 栅格高度 */
	//int* m_pColor;                   /*<! 颜色数码 */
	const char* m_projection;          /*<! 投影信息 */
	double* m_pGeoAffine;              /*<! 6个仿射参数 */
	GDALColorTable* m_pColorTable;     /*<! 颜色表 */
	bool m_isInitial;                  /*<! 是否初始化数据*/

	//double *m_pBandMean;             /*<! 波段均值 */
	//double *m_pBandStd;              /*<! 波段均方差 */
	GDALDataset *m_pSrcDS;             /*<! 源文件指针 */
};

struct MyStruct
{

};
#endif

