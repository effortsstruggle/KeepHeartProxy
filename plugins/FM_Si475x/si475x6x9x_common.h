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

/**
 *@file this header file declear the common data for si475x/6x headunit.
 *
 * @author Michael.Yi@silabs.com
*/
#ifndef _SI475X6X9X_COMMON_H_
#define _SI475X6X9X_COMMON_H_

#include "typedefs.h"


#ifdef __cplusplus
extern "C" {
#endif


#define unused(x) {x = x;}


#define BAND_FM 0   ///< fm band
#define BAND_AM 1   ///< am band
#define BAND_DAB 2  ///< dab band


#define FM_TOP_FREQ 10800
#define FM_BOT_FREQ 8750

#define AM_TOP_FREQ 1710
#define AM_BOT_FREQ 522
#define AM_BOT_FREQ_10K 520

#define VALID_FM_RSSI_THRESHOLD         12
#define VALID_FM_SNR_THRESHOLD          8
#define VALID_FM_ABS_FREQOFF_THRESHOLD  50

#if SUPPORT_RDS
#define AFCHECK_FM_RSSI_THRESHOLD         18
#define AFCHECK_FM_SNR_THRESHOLD          12
#endif

#define VALID_AM_RSSI_THRESHOLD         10
#define VALID_AM_SNR_THRESHOLD          5


extern int8_t  fm_rssi_threshold;  ///<rssi threshold for fm validate good channel, customer can modify it by himself
extern int8_t  fm_snr_threshold;   ///<snr threshold for fm validate good channel, customer can modify it by himself
extern uint8_t  fm_freqoff_threshold;///<offset for validate good channel, generally speaking, we set 50, usaully do NOT change this value.

extern int8_t  am_rssi_threshold;  ///<rssi threshold for am validate good channel, customer can modify it by himself
extern int8_t  am_snr_threshold;   ///<snr threshold for am validate good channel, customer can modify it by himself

#if SUPPORT_RDS
extern int8_t  fm_afcheck_rssi_threshold;
extern int8_t  fm_afcheck_snr_threshold;
#endif    

typedef struct _tuner_metrics_t
{
    uint8_t Valid;
    int8_t RSSI;
    int8_t SNR;
    int8_t FreqOff;
    uint16_t Freq;
}tuner_metrics_t;

extern tuner_metrics_t  g_tuner_metrics;

typedef void (*seek_found_callback)(uint16_t freq); ///< define the callback when find a good channel
typedef void (*seek_process_callback)(uint16_t freq); ///<define the callback when begin to seek current channel

/**
*   @breif part info structure
*
*   This structure includes the tuner part information
*
*/
typedef struct _part_info_t
{
	uint8_t chipRW; ///< chip revision
	uint8_t PartNum; ///<part number eg. 57
	char FWMajorRW; ///< FW major eg. 4
	char FWMinorRW; ///< FW min revision eg. 0
	uint8_t FWBuildRW;   ///< Build revision
	uint8_t ROMId;       ///ROM ID
}part_info_t;

/**
*   @breif function info structure
*
*   This structure includes the tuner function information, you can fetch the patch id by structure
*
*/
typedef struct _func_info_t
{
    uint8_t FWMajor;     ///<FW Major eg 12
    uint8_t FWMinor1;    ///<FW Minor1 eg 0
    uint8_t FWMinor2;    ///<FW Minor2 eg10
    uint8_t patchIdH;    ///<Patch ID High byte eg 0xA3
    uint8_t patchIdL;    ///<Patch ID Low byte eg 0xF6
}func_info_t;
extern func_info_t  g_func_info;  ///<global data for function information

extern part_info_t  g_part_info;  ///<global data for tuner part information
#ifdef __cplusplus
}
#endif

#endif
