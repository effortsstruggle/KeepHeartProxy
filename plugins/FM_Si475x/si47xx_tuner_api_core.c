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
// Includes
//-----------------------------------------------------------------------------
//#include "main.h"

#include <stddef.h>
#include <string.h>

#include "Drv_Si4754C.h"

//#include "tuner_api_common.h"
#include "si47xx_tuner_api_core.h"
#include "si47xx_tuner_api_extend.h"


#include "si475x6x_propertydefs.h"
#include "si475x_core.h"
//#include "si476x_core.h"
//#include "si476x_pd_core.h"
//#include "UI.h"
#include "delay.h"

#if SUPPORT_RDS
#include "rds.h"
#endif

uint8_t xdata g_band;
uint8_t xdata s_nFoundValid;
uint16_t xdata seek_preset[NUM_SEEK_PRESETS];
int8_t xdata seek_preset_snr[NUM_SEEK_PRESETS];

uint8_t Tuner_47xx_Get_Step(void) {
    uint8_t space = 0;

    space = GetSpace();

    return space;
}


void seek_process_cb(uint16_t freq) {
//    Display_SeekFreq();
    if (gBandMode == BAND_FM) {
        gFmFreq = freq;
    } else {
        gAmFreq = freq;
    }
}

void seek_found_cb(uint16_t freq) {//TODO 搜台回调
//    Insert_Station(uint16_t frequency, int8_t snr)
    unused(freq);
}

unsigned char Tuner_47xx_PowerUp(uint8_t nBandMode) {
    unsigned char ret = HAL_ERROR;
    uint8_t space = 0;

    switch (nBandMode) {
        case BAND_FM:
            if (g_tuner_type == TUNER_SI475X_SINGLE)
                ret = si475x_powerup(BAND_FM);

            g_band = nBandMode;
            space = GetSpace();
            if (space == 20) {
                gFmSpace = 2;
                gFmFreq = gFmFreq - gFmFreq % 20;
            } else if (space == 5) {
                gFmSpace = 0;
                gFmFreq = gFmFreq - gFmFreq % 5;
            } else {
                gFmSpace = 1;
                gFmFreq = gFmFreq - gFmFreq % 10;
            }

            break;

        case BAND_AM:
            if (g_tuner_type == TUNER_SI475X_SINGLE)
                ret = si475x_powerup(BAND_AM);

            g_band = nBandMode;
            space = GetSpace();
            if (space == 9) {
                gAmSpace = 0;
                gAmFreq = gAmFreq - gAmFreq % 9;
            } else {
                gAmSpace = 1;
                gAmFreq = gAmFreq - gAmFreq % 10;
            }

            break;
    }

    return ret;
}

void PowerDown() {
    if (g_tuner_type == TUNER_SI475X_SINGLE)
        si475x_powerdown();
#if 0
    else if (g_tuner_type == TUNER_SI476X_SINGLE)
        si476x_powerdown();
    else if (g_tuner_type == TUNER_SI476X_DUAL)
        si476x_pd_powerdown(g_band);
#endif
}

void Tuner_47xx_SetVolume(uint8_t value) {
    if (g_tuner_type == TUNER_SI475X_SINGLE)
        si475x_set_volume(value);
#if 0
    else if (g_tuner_type == TUNER_SI476X_SINGLE)
         si476x_set_volume(value);
     else if (g_tuner_type == TUNER_SI476X_DUAL)
         si476x_pd_set_volume(value);
#endif
}

uint8_t Tuner_47xx_Tune(uint16_t freq) {
    unsigned char ret = HAL_ERROR;

    if (g_tuner_type == TUNER_SI475X_SINGLE) {
        switch (g_band) {
            case BAND_FM:
                ret = si475x_fm_tune(0, freq);
                break;
            case BAND_AM:
                ret = si475x_am_tune(freq);
                break;
        }
    }

#if 0
    else if (g_tuner_type == TUNER_SI476X_SINGLE)
    {
        switch(g_band)
        {
            case BAND_FM:
                si476x_fm_tune(0, freq);
            break;
            case BAND_AM:
                si476x_am_tune(freq);
            break;
        }
    }
    else if (g_tuner_type == TUNER_SI476X_DUAL)
    {
        switch(g_band)
        {
            case BAND_FM:
                si476x_pd_fm_tune(0, freq);
            break;
            case BAND_AM:
                si476x_pd_am_tune(freq);
            break;
        }
    }

#endif

#if SUPPORT_RDS
    //Open RDS display.
    tune_action = 1;

    rds_reset();
#endif


    return ret;
}

//seek to a valid station
//seekUp: SEEK_DOWN, SEEK_UP
//seekMode: SEEK_WARP_BAND_LIMIT, SEEK_STOP_BAND_LIMIT
uint8_t Tuner_47xx_Seek(uint8_t seekUp, uint8_t seekMode) {
    uint8_t xdata found = 0;
    if (g_tuner_type == TUNER_SI475X_SINGLE) {
        switch (g_band) {
            case BAND_FM:
                found = si475x_fm_seek(seekUp, seekMode, seek_process_cb);
                break;

            case BAND_AM:
                found = si475x_am_seek(seekUp, seekMode, seek_process_cb);
                break;

        }
    }

#if 0
    else if (g_tuner_type == TUNER_SI476X_SINGLE)
    {
        switch(g_band)
        {
            case BAND_FM:
                found = si476x_fm_seek(seekUp, seekMode, seek_process_cb);
            break;

            case BAND_AM:
                found = si476x_am_seek(seekUp, seekMode, seek_process_cb);
            break;

        }
    }
    else if (g_tuner_type == TUNER_SI476X_DUAL)
    {
        switch(g_band)
        {
            case BAND_FM:
                found = si476x_pd_fm_seek(seekUp, seekMode, seek_process_cb);
            break;

            case BAND_AM:
                found = si476x_pd_am_seek(seekUp, seekMode, seek_process_cb);
            break;

        }
    }
#endif
    return found;
}

uint8_t AutoScan() {
    uint8_t xdata nSeek = 0;
    uint8_t i = 0;
    s_nFoundValid = 0;
    for (i = 0; i < NUM_SEEK_PRESETS; i++) {
        seek_preset_snr[i] = 0;
        seek_preset[i] = 0;
    }
    if (g_tuner_type == TUNER_SI475X_SINGLE) {
        switch (g_band) {
            case BAND_FM:
                nSeek = si475x_fm_autoseek(seek_process_cb, seek_found_cb);
                break;

            case BAND_AM:
                nSeek = si475x_am_autoseek(seek_process_cb, seek_found_cb);
                break;

        }
    }
#if 0
    else if (g_tuner_type == TUNER_SI476X_SINGLE)
    {
        switch(g_band)
        {
            case BAND_FM:
                nSeek = si476x_fm_autoseek(seek_process_cb, seek_found_cb);
            break;
            
            case BAND_AM:
                nSeek = si476x_am_autoseek(seek_process_cb, seek_found_cb);
            break;

        }
    }
    else if (g_tuner_type == TUNER_SI476X_DUAL)
    {
        switch(g_band)
        {
            case BAND_FM:
                nSeek = si476x_pd_fm_autoseek(seek_process_cb, seek_found_cb);
            break;
            
            case BAND_AM:
                nSeek = si476x_pd_am_autoseek(seek_process_cb, seek_found_cb);
            break;

        }
    }
#endif
    return nSeek;
}

void AbortSeeking() {
    if (g_tuner_type == TUNER_SI475X_SINGLE)
        si475x_abortseeking();
#if 0
    else if (g_tuner_type == TUNER_SI476X_SINGLE)
        si476x_abortseeking();
    else if (g_tuner_type == TUNER_SI476X_DUAL)
        si476x_pd_abortseeking();
#endif
}


void Tuner_47xx_Seek_ValueStatn(char bSeek) {
    Tuner_47xx_SetVolume(0);
    delay_ms(2);

    if (Tuner_47xx_Seek(bSeek, SEEK_WARP_BAND_LIMIT) == 0) {
        uint16_t freq;

        delay_ms(5);

        freq = GetFreqence(gBandMode);

        if (gBandMode == BAND_FM)
            gFmFreq = freq;
        else
            gAmFreq = freq;
    }

    Tuner_47xx_Tune((gBandMode == BAND_FM) ? gFmFreq : gAmFreq);
    Tuner_47xx_SetVolume(63);
//	Display_Item(DISPLAY_ITEM_DEVICE);
}

