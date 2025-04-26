#ifndef _MAHONY_FILTER_H
#define _MAHONY_FILTER_H


/*************************************
完成时间：2023年09月02日 
功能介绍：实现mahony姿态角解算算法的模块封装
知乎账号：龙胆也
B站账号：华南小虎队
***************************************/


struct Axis3f1;

typedef struct Axis3f1
{
    /* data */
    float x,y,z;
}Axis3f;

typedef  Axis3f Point3f;
#ifndef ANGLE_H
#define ANGLE_H
#include "math.h"
typedef struct 
{
    float LastP;//上次估算协方差 初始化值为0.02
    float Now_P;//当前估算协方差 初始化值为0
    float out;//卡尔曼滤波器输出 初始化值为0
    float Kg;//卡尔曼增益 初始化值为0
    float Q;//过程噪声协方差 初始化值为0.001
    float R;//观测噪声协方差 初始化值为0.543
}KFP;//Kalman Filter parameter


extern KFP KFP_accelX;
extern KFP KFP_accelY;
extern KFP KFP_accelZ;
extern KFP KFP_gyroX;
extern KFP KFP_gyroY;
extern KFP KFP_gyroZ;
extern float  Q_ANGLE_X, Q_ANGLE_Y, Q_ANGLE_Z; 
float kalmanFilter(KFP *kfp,float input);
void IMUupdate(float gx, float gy, float gz, float ax, float ay, float az);

extern float pitch,roll,yaw;
void algorithm(float ax,float ay,float az,float gx,float gy,float gz);
#endif

float get_angle3d(Axis3f data,Axis3f ref);
#endif