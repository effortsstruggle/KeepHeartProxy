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

#include "typedefs.h"

#include "si475x6x_common.h"

uint8_t   cmd[CMD_BUFFER_SIZE];
uint8_t   rsp[RSP_BUFFER_SIZE];

tuner_metrics_t  g_tuner_metrics;
part_info_t  g_part_info;
func_info_t  g_func_info;

///following values are host-based seek/autoseek channel validation thresholds

int8_t  fm_rssi_threshold;///<rssi threshold for fm validate good channel, customer can modify it by himself

int8_t  fm_snr_threshold;///<snr threshold for fm validate good channel, customer can modify it by himself

uint8_t  fm_freqoff_threshold;///<offset for validate good channel, generally speaking, we set 50, usaully do NOT change this value.

int8_t  am_rssi_threshold;///<rssi threshold for am validate good channel, customer can modify it by himself

int8_t  am_snr_threshold;///<snr threshold for am validate good channel, customer can modify it by himself

#if SUPPORT_RDS
int8_t  fm_afcheck_rssi_threshold;///<rssi threshold for fm do af check, once the signal drop down below it and af check begin

int8_t  fm_afcheck_snr_threshold;///<snr threshold for fm do af check, once the signal drop down below it and af check begin
#endif

