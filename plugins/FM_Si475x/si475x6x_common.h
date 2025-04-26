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
#ifndef _SI475X6X_COMMON_H_
#define _SI475X6X_COMMON_H_

#include "typedefs.h"

#ifdef __cplusplus
extern "C" {
#endif


#include "si475x6x9x_common.h"

#define CMD_BUFFER_SIZE 16
#define RSP_BUFFER_SIZE 16

extern uint8_t   cmd[CMD_BUFFER_SIZE];
extern uint8_t   rsp[RSP_BUFFER_SIZE];


#define VALID_DATA_INDEX 2

#define OFFSET_DATA_INDEX 5
#define RSSI_DATA_INDEX 6
#define SNR_DATA_INDEX 7

void si475x6x_powerdown(void);

uint16_t si475x6x_fm_get_frequency(void);
uint16_t si475x6x_am_get_frequency(void);

void si475x6x_set_volume(uint8_t value);
void si475x6x_set_seek_freq_step(uint8_t value);

void si475x6x_mute(uint8_t value);
unsigned char si475x6x_loadPatch(const uint8_t* patchData, uint16_t patchSize);

void si475x6x_get_FuncInfo(void);
unsigned char si475x6x_get_PartInfo(void);

int8_t si475x6x_fm_get_OFFSET(void);
int8_t si475x6x_fm_get_RSSI(void);
int8_t si475x6x_fm_get_SNR(void);
int8_t si475x6x_fm_get_valid(void);
uint8_t si475x6x_fm_get_blend(void);

int8_t si475x6x_am_get_OFFSET(void);
int8_t si475x6x_am_get_RSSI(void);
int8_t si475x6x_am_get_SNR(void);
int8_t si475x6x_am_get_valid(void);

uint8_t si475x6x_get_DieTemp(void);
uint8_t si475x6x_get_space(void);

unsigned char si475x6x_command(uint8_t cmd_size, uint8_t *cmd, uint8_t reply_size, uint8_t *reply);

unsigned char si475x6x_set_property(uint16_t propNumber, uint16_t propValue);

uint16_t si475x6x_get_property(uint16_t add);


void si475x6x_fm_rsq_status(uint8_t attune, uint8_t cancel, uint8_t intack, tuner_metrics_t *p_tuner_metrics);
void si475x6x_am_rsq_status(uint8_t attune, uint8_t cancel, uint8_t intack, tuner_metrics_t *p_tuner_metrics);

unsigned char si475x6x_wait_STC(void);


#ifdef __cplusplus
}
#endif

#endif
