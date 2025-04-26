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
// Tuner_api_extend.h
//
// Contains the function prototypes for the functions contained in AMRXtest.c
//
//-----------------------------------------------------------------------------
#ifndef _TUNERAPI_EXTEND_H_
#define _TUNERAPI_EXTEND_H_



#include "si475x_extend.h"
//#include "si476x_extend.h"
#include "si475x6x_common.h"

//-----------------------------------------------------------------------------
// Globals
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Function prototypes
//-----------------------------------------------------------------------------
// boot with init band FM, AM and WB
// Band: BAND_FM, BAND_AM, BAND_WB
void GetCurrentValid(uint8_t BandMode);
void GetCurrentOffset(uint8_t nBandMode);
void GetCurrentSNR(uint8_t BandMode);
void GetCurrentRssi(uint8_t BandMode);
void GetCurrentBlend(void);

void Select_Space(uint8_t BandMode);

// switch band between FM, AM and WB
// Band: BAND_FM, BAND_AM, BAND_WB
void SwitchBand(uint8_t BandMode);

//get current freqence
uint16_t GetFreqence(uint8_t BandMode);

//get current freqence
uint8_t GetSpace(void);


//unit is 10k for fm, unit 1k for am, and range is from 1~31
void SetFreqSeekSpace(uint8_t space);

/// FM specail
//mode: MODE_STEREO, MODE_MONO
void SwitchStereoMono(uint8_t mode);

uint8_t GetDieTempereture(uint8_t tunerId);
void GetFuncInfo(uint8_t tunerId);
unsigned char GetPartInfo(uint8_t tunerId);


#if SUPPORT_RDS
uint8_t IsRDSPsAvailable(void);

uint8_t IsRDSRtAvailable(void);

uint8_t xdata *GetRDSPS(void);

uint8_t xdata *GetRDSRT(void);

void ProcessRDS(void);

uint16_t AFCheck(void);

uint8_t AFTune(uint16_t freq);
#endif


extern uint16_t xdata gWaitScan;
extern uint8_t xdata gFmSpace,gAmSpace;
extern uint8_t xdata gFmRssi,gFmSnr,gFmBlend,gFmValid,gAmValid;
extern uint8_t xdata gAmRssi,gAmSnr;

#if SUPPORT_RDS
extern uint8_t xdata tune_action;
#endif

#endif

