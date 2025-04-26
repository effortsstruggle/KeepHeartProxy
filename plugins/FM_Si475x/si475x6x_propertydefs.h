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
 *@file this header file define the property address for si475x headunit.
 *
 * They are from AN543.pdf released by silicom labs broadcast BU. You can
 *
 * add other property into this document if necessary.
 *
 * @author Michael.Yi@silabs.com
*/

#ifndef _PROPERTY_DEFS_H_
#define _PROPERTY_DEFS_H_

// General Properties
#define INT_CTL_ENABLE                         0x0000   ///< interrupt enable 
#define AUDIO_ANALOG_VOLUME                    0x0300   ///< audio analog volume 
#define AUDIO_MUTE                             0x0301   ///< audio mute 


// AM Receive Properties
#define AM_SEEK_BAND_BOTTOM                    0x1100   ///< am seek band botoom frequency
#define AM_SEEK_BAND_TOP                       0x1101   ///< am seek band top frequency
#define AM_SEEK_FREQUENCY_SPACING              0x1102   ///< am seek space
#define AM_VALID_SNR_THRESHOLD                 0x2003   ///< am valid SNR threshold
#define AM_VALID_RSSI_THRESHOLD                0x2004   ///< am valid RSSI threshold

#define FM_VALID_AF_TIME                       0x2007   ///<fm af valid time

// FM Receive Properties
#define AUDIO_DE_EMPHASIS                      0x0302   ///< audio de-emphasis
#define FM_AGC_FE_CONFIG                       0x0700   ///< fm agc frontend config
#define FM_AGC_PD_CONFIG                       0x0701   ///< fm agc PD config
#define FM_MIXER_AGC_PD_THRESHOLD              0x0707   ///< fm mixer age PD threshold
#define FM_TUNE_AF_CHBW                        0x1008   ///< fm af tune chbw
#define FM_SEEK_BAND_BOTTOM                    0x1100   ///< fm seek band bottom frequncy
#define FM_SEEK_BAND_TOP                       0x1101   ///< fm seek band top frequncy
#define FM_SEEK_FREQUENCY_SPACING              0x1102   ///< fm seek space
#define FM_SEEK_CHBW                           0x1104   ///< fm seek ch bangwidth
#define FM_VALID_SNR_THRESHOLD                 0x2003   ///< fm valid SNR threshold
#define FM_VALID_RSSI_THRESHOLD                0x2004   ///< fm valid RSSI threshold
#define FM_CAL_CAPACITOR					   0x8000   ///< fm cal capacitor 
#define FM_CAL_FMAX							   0x8001   ///< fm cal fmax

//FM RDS AF
#define FM_VALID_AF_RDS_TIME                   0x2008
#define FM_VALID_AF_PI                         0x2009
#define FM_VALID_AF_PI_MASK                    0x200b

// FM RDS Receive Properties
#define FM_RDS_INTERRUPT_SOURCE               0x4000   ///< fm rds interrupt shource
#define FM_RDS_INTERRUPT_FIFO_COUNT           0x4001   ///< fm rds interrupt fifo count
#define FM_RDS_CONFIG                         0x4002   ///< fm rds config

#endif
