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

#include <stddef.h>
#include <stdio.h>
#include <string.h>

#include "si475x6x_seek.h"
#include "si475x6x_common.h"
#include "si475x6x_commanddefs.h"
#include "si475x6x_propertydefs.h"
#include <android/log.h>

extern uint8_t gStopSeeking ;
extern uint8_t gSeekUp ;
//tuneMode:  Determines the tune mode used by the device.
// 			0 = Validated normal tune
//			1 = Unvalidated fast tune
//			2 = Validated AF tune
//          3 = Validated AF check
//
//frequency:  in 10kHz units. 
//			eg, if tune 97.1M pass value 9710
uint8_t si475x6x_fm_tune(uint8_t tuneMode, uint16_t frequency) {
    cmd[0] = FM_TUNE_FREQ;
    cmd[1] = tuneMode;

    cmd[2] = (uint8_t) (frequency >> 8);;
    cmd[3] = (uint8_t) (frequency & 0x00FF);

    si475x6x_command(4, cmd, 0, NULL);

    return si475x6x_wait_STC();
}

//frequency: in 10Hz units.
//		      eg, if tune 1M pass value 1000
uint8_t si475x6x_am_tune(uint8_t checkHA, uint16_t frequency) {
    cmd[0] = AM_TUNE_FREQ;
    cmd[1] = 0x00;
    cmd[2] = (uint8_t) (frequency >> 8);
    cmd[3] = (uint8_t) (frequency & 0x00FF);
    cmd[4] = 0x00;
    cmd[5] = 0x00;
    cmd[6] = 0x08;

    if (checkHA == 0) {
        si475x6x_command(4, cmd, 0, NULL);
        return si475x6x_wait_STC();
    } else {
        si475x6x_command(7, cmd, 0, NULL);
        return si475x6x_wait_STC();
    }
}

static uint8_t si475x6x_fm_validate(uint16_t freq) {
    si475x6x_fm_tune(0, freq);

    si475x6x_fm_rsq_status(1, 0, 1, &g_tuner_metrics);

    if (g_tuner_metrics.Valid != 0 &&
        g_tuner_metrics.RSSI >= fm_rssi_threshold &&
        g_tuner_metrics.SNR >= fm_snr_threshold &&
        (g_tuner_metrics.FreqOff <= fm_freqoff_threshold &&
         g_tuner_metrics.FreqOff >= (0 - fm_freqoff_threshold))) {
        return 0;
    } else {
        return 1;
    }
}

uint8_t si475x6x_fm_seek(uint8_t seekup, uint8_t seekmode,
                         _seek_process_callback process_freq) {
    uint8_t found = 1;
    uint8_t space;
    uint16_t tuneFreq = 0;
    uint16_t startFreq = 0;

    startFreq = si475x6x_fm_get_frequency();
    
    space = si475x6x_get_space();
    if(!space )
    {
        space = 10;
        si475x_set_property(FM_SEEK_FREQUENCY_SPACING, 10);
    }
  
    LOGD("startFreq = %d,space = %d ,seekmode %d \n",startFreq,space,seekmode);
  
    if (seekup == 1) {
        tuneFreq = startFreq + space;
    } else {
        tuneFreq = startFreq - space;
    }

    while (found == 1) {
        //user abort seeking
        if (gStopSeeking) {
            gStopSeeking = 0;
            return 1;
        }

        if (seekmode == 0) {
            if (seekup == 1 && tuneFreq > FM_TOP_FREQ) {
                tuneFreq = FM_BOT_FREQ;
            } else if (seekup == 0 && tuneFreq < FM_BOT_FREQ) {
                tuneFreq = FM_TOP_FREQ;
            }
        } else {
            if (seekup == 1 && tuneFreq > FM_TOP_FREQ ||
                seekup == 0 && tuneFreq < FM_BOT_FREQ) {
                found = 1;
                return found;
            }
        }

        //begin to process current frequncy, notify user the seeking progress
#if 0
        unused(process_freq);
        Display_SeekFreq(tuneFreq);
#else
        if (process_freq != NULL)
            (*process_freq)(tuneFreq);
#endif
        found = si475x6x_fm_validate(tuneFreq);

        //if it's the current freqence, stop here
        if (tuneFreq == startFreq) {
            return found;
        }

      
        if (seekup == 1) {
            tuneFreq += space;
        } else {
            tuneFreq -= space;
        }
    }

    return found;
}

// update  si475x6x_fm_autoseek
uint8_t si475x6x_fm_autoseek(_seek_process_callback process_freq, _seek_found_callback found_freq) {
    uint8_t seek_fail;
    uint8_t found_channel = 0;
    uint16_t current_frequency = FM_BOT_FREQ;

    si475x6x_mute(1);

    if (si475x6x_fm_validate(FM_BOT_FREQ) == 0) {

        if (found_freq != NULL)
            (*found_freq)(current_frequency);

    }

    seek_fail = si475x6x_fm_seek(1, 1, process_freq);
    LOGD("seek_fail = %d", seek_fail);
    while (seek_fail == 0) {
        current_frequency = si475x6x_fm_get_frequency();

        if (found_freq != NULL)
            (*found_freq)(current_frequency);

        found_channel++;

        seek_fail = si475x6x_fm_seek(1, 1, process_freq);
         LOGD("seek_fail = %d", seek_fail);
    }
    LOGD("si475x6x_fm_rsq_status clear ");
    si475x6x_fm_rsq_status(1,0,1,&g_tuner_metrics);
    si475x6x_mute(0);

    return found_channel;
}

static uint8_t si475x6x_am_validate(uint16_t freq) {
    si475x6x_am_tune(0, freq);

    si475x6x_am_rsq_status(1, 0, 1, &g_tuner_metrics);

    if (g_tuner_metrics.Valid != 0 &&
        g_tuner_metrics.RSSI >= am_rssi_threshold &&
        g_tuner_metrics.SNR >= am_snr_threshold) {
        return 0;
    } else {
        return 1;
    }
}


 uint8_t fm_validate(uint16_t freq) {
    return si475x6x_fm_validate(freq);
}

 uint8_t am_validate(uint16_t freq) {
    return si475x6x_am_validate(freq);
}

uint8_t si475x6x_am_seek(uint8_t seekup, uint8_t seekmode,
                         _seek_process_callback process_freq) {
    uint8_t found = 1;
    uint8_t space;
    uint16_t tuneFreq = 0;
    uint16_t startFreq = 0;

    startFreq = si475x6x_am_get_frequency();
    space = si475x6x_get_space();

    if (seekup == 1) {
        tuneFreq = startFreq + space;
    } else {
        tuneFreq = startFreq - space;
    }

    while (found == 1) {
        //user abort seeking
        if (gStopSeeking) {
            gStopSeeking = 0;
            return 1;
        }

        if (seekmode == 0) {
            if (seekup == 1 && tuneFreq > AM_TOP_FREQ) {
                if (space == 10) {
                    tuneFreq = AM_BOT_FREQ_10K;
                } else {
                    tuneFreq = AM_BOT_FREQ;
                }
            } else if (seekup == 0 && tuneFreq < AM_BOT_FREQ_10K) {
                tuneFreq = AM_TOP_FREQ;
            }
        } else {
            if (seekup == 1 && tuneFreq > AM_TOP_FREQ ||
                seekup == 0 && tuneFreq < AM_BOT_FREQ) {
                found = 1;
                return found;
            }
        }

        //begin to process current frequncy, notify user the seeking progress
#if 0
        unused(process_freq);
        Display_SeekFreq(tuneFreq);
#else
        if (process_freq != NULL)
            (*process_freq)(tuneFreq);
#endif
        found = si475x6x_am_validate(tuneFreq);

        //if it's the current freqence, stop here
        if (tuneFreq == startFreq) {
            return found;
        }

     
        if (seekup == 1) {
            tuneFreq += space;
        } else {
            tuneFreq -= space;
        }
    }

    return found;
}

uint8_t si475x6x_am_autoseek(_seek_process_callback process_freq, _seek_found_callback found_freq) {
    uint8_t seek_fail;
    uint8_t space;
    uint8_t found_channel = 0;
    uint16_t current_frequency = AM_BOT_FREQ;

    si475x6x_mute(1);

    space = si475x6x_get_space();
    if (space == 10) {
        current_frequency = AM_BOT_FREQ_10K;
    }

    if (si475x6x_am_validate(current_frequency) == 0) {
#if 0
        unused(found_freq);
        Insert_Station(current_frequency, si475x6x_am_get_SNR());
#else
        if (found_freq != NULL)
            (*found_freq)(current_frequency);
#endif
    }

    seek_fail = si475x6x_am_seek(1, 1, process_freq);
    while (seek_fail == 0) {
        current_frequency = si475x6x_am_get_frequency();

#if 0
        Insert_Station(current_frequency, si475x6x_am_get_SNR());
#else
        if (found_freq != NULL)
            (*found_freq)(current_frequency);
#endif

        found_channel++;

        seek_fail = si475x6x_am_seek(1, 1, process_freq);
    }

    si475x6x_mute(0);

    return found_channel;
}

void si475x6x_abortseeking() {
    gStopSeeking = 1;
}

