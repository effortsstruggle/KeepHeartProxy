/***************************************************************************************
                  Silicon Laboratories Broadcast Si475x/6x/9x module code

   EVALUATION AND USE OF THIS SOFTWARE IS SUBJECT TO THE TERMS AND CONDITIONS OF
     THE SOFTWARE LICENSE AGREEMENT IN THE DOCUMENTATION FILE CORRESPONDING
     TO THIS SOURCE FILE.
   IF YOU DO NOT AGREE TO THE LIMITED LICENSE AND CONDITIONS OF SUCH AGREEMENT,
     PLEASE RETURN ALL SOURCE FILES TO SILICON LABORATORIES.

   Date: June 06 2014
  (C) Copyright 2014, Silicon Laboratories, Inc. All rights reserved.
****************************************************************************************/

#ifndef _SI475X_EXTEND_H_
#define _SI475X_EXTEND_H_
#include "typedefs.h"

#ifdef __cplusplus
extern "C" {
#endif

#include "si475x_core.h"

/**
*   @breif get current channel FM rssi
*
*   @param : void.
*
*   @retval : the rssi value
*/
int8_t si475x_fm_get_RSSI(void);

/**
*   @breif get current channel FM snr
*
*   @param : void.
*
*   @retval : the snr value
*/
int8_t si475x_fm_get_SNR(void);


/**
*   @breif get current channel whether a valid or invalid channel
*
*   @param : void.
*
*   @retval none-zero : valid channel
*   @retval zero : invalid channel
*/
int8_t si475x_fm_get_valid(void);

/**
*   @breif get current channel blend value
*
*   None-zero indicate its stereo.
*
*   @param : void.
*
*   @retval none-zero : stereo
*   @retval zero : mono
*/
uint8_t si475x_fm_get_blend(void);

/**
*   @breif get current tune/seek spacing both for AM and FM
*
*   @param : void.
*
*   @retval : return the space value
*/
uint8_t si475x_get_space(void);

/**
 *@description: 设置搜台的时的步长
 *@author: sky
 *@param i1[in] 输入参数1
 *@return  0 成功
           ～0 错误
 *@date: 2024-07-31 10:34:24
*/
void si475x_set_space(uint16_t);

/**
*   @breif get function info
*
*   Can work only when tuner power up mode. After call this function, the data will be stored in g_funcinfo
*
*   @param : void.
*
*   @retval : void
*/
void si475x_get_FuncInfo(void);

/**
*   @breif get part info
*
*   Can work only when tuner power up mode. After call this function, the data will be stored in g_partinfo
*
*   @param : void.
*
*   @retval : void
*/
unsigned char si475x_get_PartInfo(void);

/**
*   @breif get current channel AM rssi
*
*   @param : void.
*
*   @retval : the rssi value
*/
int8_t si475x_am_get_RSSI(void);

/**
*   @breif get current channel AM snr
*
*   @param : void.
*
*   @retval : the snr value
*/
int8_t si475x_am_get_SNR(void);

/**
*   @breif get current channel whether a valid or invalid channel
*
*   @param : void.
*
*   @retval none-zero : valid channel
*   @retval zero : invalid channel
*/
int8_t si475x_am_get_valid(void);

/**
*   @breif get die temperature
*
*   @param : void.
*
*   @retval : the temperature degree.
*/
uint8_t si475x_get_DieTemp(void);

int8_t si475x_fm_get_offset(void);
int8_t si475x_am_get_offset(void);



#ifdef __cplusplus
}
#endif

#endif
