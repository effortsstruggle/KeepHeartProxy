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


//-----------------------------------------------------------------------------
//
// Tuner_api.h
//
// Contains the function prototypes for the functions contained in AMRXtest.c
//
//-----------------------------------------------------------------------------
#ifndef _TUNERAPI_H_
#define _TUNERAPI_H_

//-----------------------------------------------------------------------------
// Globals
//-----------------------------------------------------------------------------
#define SEEK_DOWN 0
#define SEEK_UP 1

#define SEEK_WARP_BAND_LIMIT 0
#define SEEK_STOP_BAND_LIMIT 1

#define NUM_SEEK_PRESETS 6
extern uint16_t xdata seek_preset[];
extern uint8_t xdata s_nFoundValid;

#define MODE_STEREO 1
#define MODE_MONO 0

//-----------------------------------------------------------------------------
// Function prototypes
//-----------------------------------------------------------------------------
// boot with init band FM, AM and WB
// Band: BAND_FM, BAND_AM, BAND_WB
unsigned char Tuner_47xx_PowerUp(uint8_t nBandMode);
void PowerDown(void);

//value should between 0 ~ 63
void Tuner_47xx_SetVolume(uint8_t value);

//tune for fm or am, the unit for freq is 10k, am is 1k
uint8_t Tuner_47xx_Tune(uint16_t freq);


//seek to a valid station
//seekUp: SEEK_DOWN, SEEK_UP
//seekMode: SEEK_WARP_BAND_LIMIT, SEEK_STOP_BAND_LIMIT
uint8_t Tuner_47xx_Seek(uint8_t seekUp, uint8_t seekMode);

/**
 *@description: 调用搜台 am fm 自动搜台，实现了其中的默认回调函数
 *@author: sky
 *@return 
 *@date: 2024-07-31 10:20:53
*/
uint8_t AutoScan(void);

void AbortSeeking(void);
uint8_t Tuner_47xx_Get_Step(void);
void Tuner_47xx_Seek_ValueStatn(char bSeek);

#endif

