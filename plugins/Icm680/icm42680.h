#pragma once 

#include "mahony_filter.h"


/**
 *@description: 获取打开状态
 *@author: sky
 *@return -1 :失败
 *@date: 2024-11-13 10:07:51
*/
int get_satus();


/**
 *@description: 打开i2c 并;尝试通信
 *@author: sky
 *@return -1 :失败
 *@date: 2024-11-13 10:07:51
*/
int  open_dev();

/**
 *@description: 姿态矫正
 *@author: sky
 *@param times[in] 校正次数,统计次数
 *@param alg[in] 校正算法
 *@return 
 *@date: 2024-11-13 10:09:10
*/

void attitude_correction(int times,int alg);

/**
 *@description: 获取当前角度 ,读取5次取平均值，减小波动
 *@author: sky
 
 *@return 当前角度 x: 与x轴。。。 还有点问题,应该计算的时与xy平面的夹角
 *@date: 2024-11-13 10:11:24
*/

Point3f getCurAngle();

/**
 *@description: 获取标定后的初始角度
 *@author: sky
 
 *@return 当前角度 x: 与x轴。。。 还有点问题,应该计算的时与xy平面的夹角
 *@date: 2024-11-13 10:11:24
*/

Point3f getInitAngle();


/**
 *@description: 设置初始角度替代标定的角度
 *@author: sky
 
 *@return 当前角度 x: 与x轴。。。 还有点问题,应该计算的时与xy平面的夹角
 *@date: 2024-11-13 10:11:24
*/

void  setInitAngle(Point3f const &);

/**
 *@description: 获取当前加速度
 *@author: sky
 
 *@return 当前角度，xyz 加速度在各州的分量
 *@date: 2024-11-13 10:11:24
*/
Point3f getCurAcc();

/**
 *@description: 获取当前角速度
 *@author: sky
 
 *@return 当前角度，xyz 加速度在各州的分量
 *@date: 2024-11-13 10:11:24
*/
Point3f getCurGyro();
