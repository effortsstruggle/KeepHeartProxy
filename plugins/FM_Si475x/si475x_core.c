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
//#include "main.h"
#include "delay.h"
//#include "usart.h"
#include <android/log.h>

#include "Drv_Si4754C.h"
#include "si475x6x_seek.h"
#include "si475x_core.h"
#include "si475x_extend.h"
#include "Drv_Si4754C.h"
//#include "Hat_I2C.h"

static const char *TAG = "IEAI_Radio- ";

static uint8_t const preload_cmd[6] = {0x01, 0x77, 0x27, 0x27, 0x00, 0x00};
static uint8_t const force_cts_cmd[3] = {0xfb, 0x06, 0x80};
static uint8_t const patch_cmd_5x[6] = {0x01, 0x77, 0x27, 0x23, 0x00, 0x11};
static uint8_t const powerup_cmd_5x[6] = {0x01, 0x77, 0x27, 0x23, 0x10, 0x11};

unsigned char
si475x_command(uint8_t cmd_size, uint8_t idata *cmd, uint8_t reply_size, uint8_t idata *reply) {
    return si475x6x_command(cmd_size, cmd, reply_size, reply);
}

unsigned char si475x_set_property(uint16_t propNumber, uint16_t propValue) {
    return si475x6x_set_property(propNumber, propValue);
}

#if 0 == 0

uint16_t si475x_get_property(uint16_t propNumber) {
    return si475x6x_get_property(propNumber);
}

#endif

// Pull 6x reset pin to high then wait for about 100us and pull the pin to low.
#define RST_4754_HIGH        HAL_GPIO_WritePin(GPIOA, GPIO_PIN_12,  GPIO_PIN_SET)
#define RST_4754_LOW        HAL_GPIO_WritePin(GPIOA, GPIO_PIN_12,  GPIO_PIN_RESET)

static void si475x_reset(void) {
//	RST_4754_LOW;
    delay_us(100);
//	RST_4754_HIGH;
    delay_us(100);
}

uint16_t si475x_fm_get_frequency() {
    return si475x6x_fm_get_frequency();
}

uint16_t si475x_am_get_frequency() {
    return si475x6x_am_get_frequency();
}

void si475x_set_volume(uint8_t value) {
    si475x6x_set_volume(value);
}

void si475x_mute(uint8_t value) {
    si475x6x_mute(value);
}

static unsigned char si475x_powerup_A40(uint8_t band) {
    unsigned char ret = 0;
    uint8_t i = 0;
    // send patch data
    __android_log_print(ANDROID_LOG_ERROR, TAG, "si475x_powerup_A40    %d",
                        band);
    if (band == BAND_FM) {
        __android_log_print(ANDROID_LOG_ERROR, TAG, "si475x6x_load FM Patch    %d",
                            FM_PATCH_SIZE_5X);
        ret = si475x6x_loadPatch(si475x_fm_patch, FM_PATCH_SIZE_5X);
    } else if (band == BAND_AM) {
        __android_log_print(ANDROID_LOG_ERROR, TAG, "si475x6x_load AM Patch   %d",
                            AM_PATCH_SIZE_5X);
        ret = si475x6x_loadPatch(si475x6x_am_patch, AM_PATCH_SIZE_5X);
    }

    if (ret != CMD_SUCCESS) return HAL_ERROR;
    // power up
    for (i = 0; i < 6; i++) {
        cmd[i] = powerup_cmd_5x[i];
    }
    cmd[2] = si475x_xcLoad;

    if (band == BAND_AM)
        cmd[4] = 0x20;

    si475x_command(6, cmd, 1, rsp);

    // property setting
    if (band == BAND_FM) {
        for (i = 0; i < FM_PROPERTY_5X_A40; i++) {
            ret = si475x_set_property(si475x_a40_fm_property[i][0], si475x_a40_fm_property[i][1]);
            if (ret != CMD_SUCCESS) return HAL_ERROR;
        }
    } else if (band == BAND_AM) {
        for (i = 0; i < AM_PROPERTY_5X_A40; i++) {
            ret = si475x_set_property(si475x6x_am_property[i][0], si475x6x_am_property[i][1]);
            if (ret != CMD_SUCCESS) return HAL_ERROR;
        }
    }

#if 0 == 0
    //check patch & property
    si475x6x_get_FuncInfo();
#endif

    return ret;
}

static unsigned char si475x_powerup_A55(uint8_t band) {
    unsigned char ret;
    uint8_t i = 0;

    //power up
    for (i = 0; i < 6; i++) {
        cmd[i] = powerup_cmd_5x[i];
    }
    cmd[2] = si475x_xcLoad;

    if (band == BAND_AM)
        cmd[4] = 0x20;

    ret = si475x_command(6, cmd, 1, rsp);
    if (ret != CMD_SUCCESS) {
//		dbg_print("not si475x A55\n");
        return ret;
    }

    //property setting
    __android_log_print(ANDROID_LOG_ERROR, TAG, "si475x_powerup_A55    %d",
                        band);
    if (band == BAND_FM) {
        for (i = 0; i < FM_PROPERTY_5X_A55; i++) {
            __android_log_print(ANDROID_LOG_ERROR, TAG,
                                "si475x_set_property  si475x_a55_fm_property   %d",
                                FM_PROPERTY_5X_A55);
            ret = si475x_set_property(si475x_a55_fm_property[i][0], si475x_a55_fm_property[i][1]);
            if (ret != CMD_SUCCESS) return ret;
        }
    } else if (band == BAND_AM) {
        for (i = 0; i < AM_PROPERTY_5X_A55; i++) {
            __android_log_print(ANDROID_LOG_ERROR, TAG,
                                "si475x_set_property si475x_a55_am_property    %d",
                                AM_PROPERTY_5X_A55);
            ret = si475x_set_property(si475x_a55_am_property[i][0], si475x_a55_am_property[i][1]);
            if (ret != CMD_SUCCESS) return ret;
        }
    }

#if 0// == 0
    //check property
    if (band == BAND_FM)
    {
        for (i=0; i<FM_PROPERTY_5X_A55; i++)
        {
            assert(si475x_get_property(si475x_a55_fm_property[i][0]) == si475x_a55_fm_property[i][1]);
        }
    }
    else
    {
        for (i=0; i<AM_PROPERTY_5X_A55; i++)
        {
            assert(si475x_get_property(si475x_a55_am_property[i][0]) == si475x_a55_am_property[i][1]);
        }
    }
#endif

    return ret;
}

static unsigned char si475x_powerup_without_reset(uint8_t band) {
    unsigned char ret = 0;
    uint8_t i = 0;

/*    1. preload for 4M crystal */
    for (i = 0; i < 6; i++) {
        cmd[i] = preload_cmd[i];
    }

    cmd[2] = si475x_xcLoad;

    if (CMD_SUCCESS != i2c_write(si475x_chipAddress, cmd, 6))
        return HAL_ERROR;

    delay_ms(100);

/*    2. force cts */
    for (i = 0; i < 3; i++) {
        cmd[i] = force_cts_cmd[i];
    }
    
    if (CMD_SUCCESS != i2c_write(si475x_chipAddress, cmd, 3))
        return HAL_ERROR;

    delay_ms(5);

/*    3. check part */
    //send power up boot command
    for (i = 0; i < 6; i++) {
        cmd[i] = patch_cmd_5x[i];
    }
    cmd[2] = si475x_xcLoad;
    if (CMD_SUCCESS != si475x_command(6, cmd, 1, rsp))
        return HAL_ERROR;

    if (CMD_SUCCESS != GetPartInfo(0))
        return HAL_ERROR;

/*    4. check power up for each part */
//    __android_log_print(ANDROID_LOG_ERROR, TAG,"gTunerFWRevision %d", gTunerFWRevision);
    if (gTunerFWRevision == 40) {
        ret = si475x_powerup_A40(band);
    } else if (gTunerFWRevision == 55) {
        ret = si475x_powerup_A55(band);
    }

    //5. configure audio output as analog audio
    cmd[0] = ANA_AUDIO_PIN_CFG;
    cmd[1] = 0x02;
//
    ret |= si475x_command(2, cmd, 0, NULL);

#if SUPPORT_RDS
    //6. configure interrupt for RDS
    if (band == BAND_FM)
    {
        //enable RDS interrupt
        //si475x_set_property(INT_CTL_ENABLE, 0x0004);
        
        //  Setup the RDS Interrupt to interrupt when RDS data is available (RDSRECV).
        //si475x_set_property(FM_RDS_INTERRUPT_SOURCE, 0x0002);

        // Enable the RDS and allow all blocks so we can compute the error
        // rate later.
        ret |=si475x_set_property(FM_RDS_CONFIG, 0x00F1);
    }
#endif

    return ret;
}

unsigned char si475x_powerup(uint8_t band) {
    si475x_reset();

    return si475x_powerup_without_reset(band);
}

void si475x_powerdown(void) {
    si475x6x_powerdown();
}

void si475x_bandswitch(uint8_t band) {
    si475x_powerdown();

    si475x_powerup_without_reset(band);
}


//tuneMode:  Determines the tune mode used by the device.
// 			0 = Validated normal tune
//			1 = Unvalidated fast tune
//			2 = Validated AF tune
//          3 = Validated AF check
//
//frequency:  in 10kHz units. 
//			eg, if tune 97.1M pass value 9710
unsigned char si475x_fm_tune(uint8_t tuneMode, uint16_t frequency) {
    return si475x6x_fm_tune(tuneMode, frequency);
}


//frequency: in 10Hz units.
//		      eg, if tune 1M pass value 1000
unsigned char si475x_am_tune(uint16_t frequency) {
    return si475x6x_am_tune(1, frequency);
}


uint8_t si475x_fm_seek(uint8_t seekup, uint8_t seekmode,
                       seek_process_callback process_freq) {
    return si475x6x_fm_seek(seekup, seekmode, process_freq);
}


uint8_t si475x_fm_autoseek(seek_process_callback process_freq, seek_found_callback found_freq) {
    return si475x6x_fm_autoseek(process_freq, found_freq);
}


uint8_t si475x_am_seek(uint8_t seekup, uint8_t seekmode, seek_process_callback process_freq) {
    return si475x6x_am_seek(seekup, seekmode, process_freq);
}

uint8_t si475x_am_autoseek(seek_process_callback process_freq, seek_found_callback found_freq) {
    return si475x6x_am_autoseek(process_freq, found_freq);
}

void si475x_abortseeking() {
    si475x6x_abortseeking();
}


