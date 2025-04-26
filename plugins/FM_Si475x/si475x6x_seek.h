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
 *@file this header file for internal API. Not API for customer directly using.
 *
 * @author Michael.Yi@silabs.com
*/
#ifndef _SI475X6X_SEEK_H_
#define _SI475X6X_SEEK_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "typedefs.h"

typedef void (*_seek_found_callback)(
        uint16_t freq); ///< define the callback when find a good channel
typedef void (*_seek_process_callback)(
        uint16_t freq); ///<define the callback when begin to seek current channel


void si475x6x_abortseeking(void);

uint8_t si475x6x_am_autoseek(_seek_process_callback process_freq, _seek_found_callback found_freq);

uint8_t si475x6x_am_seek(uint8_t seekup, uint8_t seekmode, _seek_process_callback process_freq);

uint8_t si475x6x_fm_autoseek(_seek_process_callback process_freq, _seek_found_callback found_freq);

uint8_t si475x6x_fm_seek(uint8_t seekup, uint8_t seekmode, _seek_process_callback process_freq);

uint8_t si475x6x_am_tune(uint8_t checkHA, uint16_t frequency);

uint8_t si475x6x_fm_tune(uint8_t tuneMode, uint16_t frequency);

uint8_t fm_validate(uint16_t freq);

uint8_t am_validate(uint16_t freq);

#ifdef __cplusplus
}
#endif

#endif
