#ifndef _BD37033_H
#define _BD37033_H

#include <stdint.h>

int bd37033_device_init(void);
int bd37033_device_close(void);
/**
 *@brief: 芯片打开是否成功
 *@author: sky
 *@return: 0 ；打开成功
           -1 打开失败
*/
int bd37033_opened();
/**
 *@brief: 切换通道
 *@author: sky
 *@version 
 *@param i2[in] 输入通道 区值 [0~4]
*/
int bd37033_set_source(uint8_t source);

int bd37033_set_loudness(uint8_t enable);
/**
 *@brief:  设置音量大小
 *@author: sky
 *@version 
 *@param vol [in] 取值范围 0 ～ 95  蓝牙音乐最大

*/
int bd37033_set_volume(uint8_t vol);
/**
 *@brief: 
 *@author: sky
 *@version 
 *@param i2[in] 输入参数2
 *@param o3[out] 输出参数1
 *@since 
 *@date: 2025-01-22 15:44:54
*/
int bd37033_get_volume(void);
int bd37033_set_bass(int bass);
int bd37033_set_treble(int treble);
int bd37033_set_speaker(int channel, int volume);
/**
 *@brief: 
 *@author: sky
 *@version 
 *@param enable[in] 1 静音 ; 0 不
*/
int bd37033_set_mute(int enable);
int bd37033_set_fader(int val);
int bd37033_set_gain(uint8_t gain);
#endif

