/*
*********************************************************************************************************
*
*	模块名称 : FFT
*	文件名称 : FFTInc.h
*	版    本 : V1.0
*	说    明 : 不限制点数FFT，点数由宏定义MAX_FFT_N配置。
*              对于8192和16384点，计算的sin和con数值存到了内部Flash里面。
*              最小支持16点，最大点不限，满足2^n即可。
*
*	修改记录 :
*		版本号  日期         作者       说明
*		V1.0    2021-06-23  Eric2013   正式发布
*
*	Copyright (C), 2021-2030, 安富莱电子 www.armfly.com
*
*********************************************************************************************************
*/
#ifndef __FFTInc_H_
#define __FFTInc_H_

#include "math.h"
#include "arm_math.h"

#define   MAX_FFT_N		 8192	

struct  compx 
{
	float32_t real, imag;
};   

void InitTableFFT(uint32_t n);
void cfft(struct compx *_ptr, uint32_t FFT_N );

#endif

/***************************** 安富莱电子 www.armfly.com (END OF FILE) *********************************/
