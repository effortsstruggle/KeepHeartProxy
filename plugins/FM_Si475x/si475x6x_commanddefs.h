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
 *@file this header file define the common command for si475x headunit.
 *
 * They are from AN543.pdf released by silicom labs broadcast BU. You can
 *
 * add other command into this document if necessary.
 *
 * @author Michael.Yi@silabs.com
*/
#ifndef _COMMAND_DEFS_H_
#define _COMMAND_DEFS_H_

//==================================================================
// General Commands
//==================================================================

// STATUS bits - Used by all methods
#define STCINT  0x01    ///< STC bit postision in status
#define ACFINT  0x02    ///< ACF bit postision in status
#define RDSINT  0x04    ///< ACF bit postision in status
#define RSQINT  0x08    ///< RSQ bit postision in status
#define ASQINT  0x10    ///< ASQ bit postision in status
#define ERR     0x40    ///< ERR bit postision in status
#define CTS     0x80    ///< CTS bit postision in status

// Boot Commands
#define POWER_UP          0x01  ///< power up command
#define PART_INFO         0x02  ///< part info command

// Common Commands
#define POWER_DOWN        0x11  ///< power down command
#define FUNC_INFO         0x12  ///< fun info command
#define SET_PROPERTY      0x13  ///< set property command
#define GET_PROPERTY      0x14  ///< get property command
#define GET_INT_STATUS    0x15  ///< get interrupt status command
#define AGC_STATUS        0x17  ///< AGC status command
#define DIG_AUDIO_PIN_CFG 0x18  ///< digit audio pin config command
#define ZIF_PIN_CFG       0x19  ///< ZIF pin config command
#define IC_LINK_PIN_CFG   0x1A  ///< IC link pin config command
#define ANA_AUDIO_PIN_CFG 0x1B  ///< audio pin config command
#define GET_TEMP          0x1D  ///< get temperature command

// FM Receive Commands
#define FM_TUNE_FREQ      0x30  ///< FM tune command
#define FM_SEEK_START     0x31  ///< FM seek start command
#define FM_RSQ_STATUS     0x32  ///< FM get RSQ status command
#define FM_ACF_STATUS     0x35  ///< FM get ACF status command

// FM Receive RDS Commands
#define FM_RDS_STATUS     0x36  ///< FM RDS status command
#define FM_RDS_BLOCKCOUNT 0x37  ///< FM RDS blockcount command
#define FM_PHASE_DIVERSITY 0x38 ///< FM set phase diversity config command
#define FM_PHASE_DIV_STATUS 0x39///< FM get phase diversity status command

// AM Receive Commands
#define AM_TUNE_FREQ      0x40  ///< AM tune command
#define AM_SEEK_START     0x41  ///< AM seek start command
#define AM_RSQ_STATUS     0x42  ///< AM get RSQ status command
#define AM_ACF_STATUS     0x45  ///< AM get ACF status command
#define AM_AGC_STATUS     0x47  ///< AM get AGC status command

// WB Receive Commands
#define WB_TUNE_FREQ      0x50  ///< WB tune command
#define WB_SEEK_START     0x51  ///< WB seek start command
#define WB_RSQ_STATUS     0x52  ///< WB get RSQ status
#define WB_ASQ_STATUS     0x54  ///< WB get ASQ stauts
#define WB_ACF_STATUS     0x55  ///< WB get ACF stauts

// WB Receive SAME Commands
#define WB_SAME_STATUS    0x56  ///< WB same command
#define WB_SAME_SOFTuint8_t  0x57  ///< WB soft byte command

#endif
