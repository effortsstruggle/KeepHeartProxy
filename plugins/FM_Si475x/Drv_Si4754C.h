
#ifndef _DRV_SI4754C_H
#define _DRV_SI4754C_H

#ifdef __cplusplus
extern "C" {
#endif

#include "typedefs.h"
#include "si475x6x9x_common.h"
//------------------------4754C
#define xdata
#define idata
#define code const


#define i2c_write(id, buf, cnt)    I2C1_Write(id,buf,cnt)
#define i2c_read(id,  buf , cnt)     I2C1_Read(id,buf,cnt)


extern uint8_t g_band;

extern uint8_t g_tuner_type;
extern uint8_t g_tuner_count;

extern uint16_t gFmFreq;
extern uint16_t gAmFreq;
extern int8_t gFreqOffset;


extern uint16_t gTempFreq;
extern uint8_t gBandMode;
extern uint8_t gFmSnr, gAmSnr;
extern uint8_t gStatn;
extern uint8_t gFmRssi, gAmRssi, gFmBlend, gFmValid, gAmValid;
extern uint8_t gAmValidStatn, gFmValidStatn;
extern uint8_t gTunerFWRevision;

#define NUM_SEEK_PRESETS 6

typedef enum _TUNER_TYPE {
    TUNER_SI475X_SINGLE,
    TUNER_SI476X_SINGLE,
    TUNER_SI476X_DUAL,
    TUNER_UNKOWN
} TUNER_TYPE;


/**
 *@description: 初始化芯片
 *@author: sky
 *@param iic_addr[in] 传入i2c 地址,示例 "/dev/i2c-1"
 *@param slave_addr[in] 寄存器地址 传入无读写位的地址，为0时使用默认地址 0x60
 *@return 0 success
          -1 error
 *@date: 2024-08-16 14:40:09
*/
unsigned char Init_Si4754C(const char* iic_addr,uint8_t slave_addr);

unsigned char GetPartInfo(uint8_t tunerId);

/**
 *@description: 返回当前芯片状态
 *@author: sky
 *@return 
        0 未启动 
        1 已启动 
        2 开启或关闭中
 *@date: 2024-09-11 16:17:23
*/
uint8_t GetStatus();


/**
 *@description: 获取音量
 *@author: sky
 *@return 音量

 *@date: 2024-09-11 16:17:23
*/
uint16_t GetVolume();


/**
 *@description: 设置AM FM 模式
 *@author: sky
 *@param mode[in] 0 fm ;1 am
 *@return 
 *@date: 2024-09-11 16:11:48
*/
uint8_t SetBandMode(uint8_t mode);

uint8_t GetBandMode();
/**
 *@description: 设置播放的频率 
 *@author: sky
 *@param freq[in] freq 设置的频率
 *@return 
 *@date: 2024-08-16 14:50:45
*/
uint8_t Tuner_Tune(uint16_t freq);

/**
 *@description: 音量设置 //value should between 0 ~ 63 ，设置为
 *@author: sky
 *@param value[in] 音量的值
 *@return 
 *@date: 2024-08-16 14:49:18
*/
void SetVolume(uint8_t value);

void SetMute(uint8_t value);
uint16_t GetMute();
/**
 *@description: 打开i2c,初始化收音机
 *@author: sky
 *@param i1[in] 输入参数1
 *@return 
    -1 失败
 *@date: 2024-09-11 10:18:58
*/

int  PowerUp();
// 下电
void Powerdown(void);

void SetStereoMono(uint8_t mode);

uint16_t GetFreq();

uint8_t FmValidate(uint16_t freq);

void Tuner_Poll(void);

//设置搜索模式为向上搜索
void SetSeekUp(uint8_t );

//unit is 10k for fm, unit 1k for am, and range is from 1~31
void SetSeekSpace(uint8_t);

void AutoSeek(seek_process_callback process_freq, seek_found_callback found_freq);
/**
 *@brief: 停止搜台，设置全局变量为停止，来通知下层接口
 *@author: sky
 *@version 
 *@date: 2024-08-28 15:21:15
*/
void StopSeeking();


int GetI2cError();
#ifdef __cplusplus
}
#endif
#endif