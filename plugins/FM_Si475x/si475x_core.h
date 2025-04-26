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
 *@file this header file declear the core function for si475x headunit.
 *
 * @author Michael.Yi@silabs.com
*/

#ifndef _SI475X_CORE_H_
#define _SI475X_CORE_H_

#ifdef __cplusplus
extern "C" {
#endif

#define SUPPORT_OFFICAIL_SET_5X6X_SINGLE 1

#include "si475x6x_common.h"
#include "si475x6x_commanddefs.h"
#include "si475x6x_propertydefs.h"

/**
*   @breif command sent to tuner
*
*   The command sent to tuner is by i2c interface. Refer the tuner document to know the expected data reply.
*
*   @param cmd_size: the command data size to sent to tuner.
*   @param cmd: the sent command data.
*   @param reply_size: the reply data expect to read from tuner.
*   @param reply: the reply data buffer.
*
*   @retval void
*/
unsigned char si475x_command(uint8_t cmd_size, uint8_t  *cmd, uint8_t reply_size, uint8_t  *reply);

/**
*   @breif set property to tuner
*
*   Be sure the tuner is working when call this routine.
*
*   @param propNumber: property address.
*   @param propValue: property value.
*
*   @retval void
*/
unsigned char si475x_set_property(uint16_t propNumber, uint16_t propValue);
uint16_t si475x_get_property(uint16_t propNumber);
/**
*   @breif Power up function.
*
*   After call this function, the tuner should start to work.
*
*   @param band: The band type, BAND_FM or BAND_AM.
*
*   @retval void
*/
unsigned char si475x_powerup(uint8_t band);

/**
*   @breif Power down function.
*
*   After call this function, the tuner should exit to work.
*
*   @param : none.
*
*   @retval void
*/
void si475x_powerdown(void);

/**
*   @breif Switch band.
*
*   Switch band between AM and FM
*
*   @param band: The band type to switch, BAND_FM or BAND_AM.
*
*   @retval void
*/
void si475x_bandswitch(uint8_t band);


/**
*   @breif FM tune function.
*
*   @param tuneMode: The tune mode: 0 = Validated normal tune; 1 = Unvalidated fast tune;
*                                   2 = Validated AF tune; 3 = Validated AF check
*   @param frequency: The frenquency unit is in 10K Hz.
*
*   @retval void
*/
unsigned char si475x_fm_tune(uint8_t tuneMode, uint16_t frequency);

/**
*   @breif AM tune function.
*
*   @param frequency: The frenquency unit is in 100 Hz.
*
*   @retval void
*/
unsigned char si475x_am_tune(uint16_t frequency);

/**
*   @breif Set the tune volume.
*
*   After first tune you should set the volume.
*
*   @param value: The value is from 0~63.
*
*   @retval void
*/
void si475x_set_volume(uint8_t value);

/**
   @param value: zero, unmute and none-zero to mute.
*
*   @retval void
*/
void si475x_mute(uint8_t value);

/**
*   @breif Seek the next valid channel.
*
*   The function will call process_freq when start to tune a new channel, user can show GUI in this callback function.
*
*   @param seekup: 1 up and 0 down.
*   @param seekmode: 1 stop at band limits 0 wrap at aband limits.
*   @param process_freq: callback function when begin to seek a new channel.
*
*   @retval 0 find a valid channel
*   @retval 1 do not find a valid channel
*/
uint8_t si475x_fm_seek(uint8_t seekup, uint8_t seekmode, seek_process_callback process_freq);


/**
*   @breif Auto seek the full band.
*
*   This function will sweep the whole band and find the valid channel. When find a valid channel found_freq will be called.
*
*   @param process_freq: callback function when begin to handle a new channel.
*   @param found_freq: callback function when find valid channel.
*
*   @retval: the valid channel count
*/
uint8_t si475x_fm_autoseek(seek_process_callback process_freq, seek_found_callback found_freq);


/**
*   @breif Seek the next valid channel.
*
*   The function will call process_freq when start to tune a new channel, user can show GUI in this callback function.
*
*   @param seekup: 1 up and 0 down.
*   @param seekmode: 1 stop at band limits 0 wrap at aband limits.
*   @param process_freq: callback function when begin to seek a new channel.
*
*   @retval 0 find a valid channel
*   @retval 1 do not find a valid channel
*/
uint8_t si475x_am_seek(uint8_t seekup, uint8_t seekmode, seek_process_callback process_freq);

/**
*   @breif Auto seek the full band.
*
*   This function will sweep the whole band and find the valid channel. When find a valid channel found_freq will be called.
*
*   @param process_freq: callback function when begin to handle a new channel.
*   @param found_freq: callback function when find valid channel.
*
*   @retval: the valid channel count
*/
uint8_t si475x_am_autoseek(seek_process_callback process_freq, seek_found_callback found_freq);

/**
*   @breif Get current frequency.
*
*   This function will return current channel frequncy.
*
*   @param : void
*
*   @retval: the channel in 10KHz
*/
uint16_t si475x_fm_get_frequency(void);

/**
*   @breif Get current frequency.
*
*   This function will return current channel frequncy.
*
*   @param : void
*
*   @retval: the channel in 100Hz
*/
uint16_t si475x_am_get_frequency(void);

/**
*   @breif Abord seeking.
*
*   This function will stop seek or auto seek operation.
*
*   @param : void
*
*   @retval: void
*/
void si475x_abortseeking(void); 

extern uint8_t  si475x_chipAddress;    ///<i2c address, please ask you hardware engineer the acctual i2c address.
extern uint8_t  si475x_xcLoad;         ///<xcLoad value usally is set to 0x27, user can modify it according AN543Rev0.8 8.1.12 

extern uint16_t  FM_PATCH_SIZE_5X;       ///< fm patch data size
extern uint16_t  AM_PATCH_SIZE_5X;    ///< am patch data size
extern uint8_t  FM_PROPERTY_5X_A40;      ///< fm a40 property group to modified
extern uint8_t  AM_PROPERTY_5X_A40;      ///< am a40 property group to modified
extern uint8_t  FM_PROPERTY_5X_A55;      ///< fm a55 property group to modified
extern uint8_t  AM_PROPERTY_5X_A55;      ///< am a55 property group to modified

extern const uint8_t  si475x_fm_patch[];    ///<fm patch data
extern const uint8_t si475x6x_am_patch[];    ///< am patch data

extern const uint16_t  si475x_a40_fm_property[][2];   ///< fm a40 property group
extern const uint16_t  si475x_a55_fm_property[][2];   ///< fm a55 property group
extern const uint16_t  si475x6x_am_property[][2];   ///< am a40/a42 property group
extern const uint16_t  si475x_a55_am_property[][2];   ///< am a55 property group

#ifdef __cplusplus
}
#endif

#endif
