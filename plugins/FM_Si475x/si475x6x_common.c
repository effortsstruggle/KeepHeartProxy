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

#include "si475x6x_common.h"
#include "si475x6x_commanddefs.h"
#include "si475x6x_propertydefs.h"
#include "si475x6x_seek.h"

#include "delay.h"
#include "Drv_Si4754C.h"
//#include "Hat_I2c.h"
#include "si475x_core.h"
#include <android/log.h>

#define i2c_chipAddress si475x_chipAddress
// When the user sends any command, the CTS bit will immediately reset to 0. CTS will remain 0 while the chip
// processes the command. When the chip is finished processing the command, the CTS bit will be set back to 1.
unsigned char si475x6x_wait_CTS (void)
{
	uint8_t status = 0;
	uint32_t i;
	uint8_t timeout_ms = 100;

	for(i=0; i < timeout_ms; i++)
	{
		i2c_read(i2c_chipAddress,&status,  1);

		if(status & CTS) 
		{
			return CMD_SUCCESS;
		}

		// delay function for 1 ms
		delay_ms(1);
	}

	return TIMEOUT;
}

// Seek and Tune commands may take longer to complete than most other commands, 
// so they also use the STC bit (Seek/Tune Complete) to indicate they have completed.
unsigned char si475x6x_wait_STC (void)
{
	uint8_t status = 0;
	uint32_t i;
	int timeout_ms = 500;

	for(i=0; i < timeout_ms; i++)
	{
		i2c_read(i2c_chipAddress,&status,  1);

		if(status & STCINT) 
		{
			return CMD_SUCCESS;
		}

		// delay function for 1 ms
		delay_ms(1);
	}

	return TIMEOUT;
}

unsigned char si475x6x_command(uint8_t cmd_size, uint8_t idata *cmd, uint8_t reply_size, uint8_t idata *reply)
{
	unsigned char ret;
	
	//wait for cts bit 
	ret = si475x6x_wait_CTS();

	if(ret != CMD_SUCCESS)
		__android_log_print(ANDROID_LOG_ERROR, "si475x6x_wait_CTS: ","%0x", ret);
	//write the command data
	if(ret == CMD_SUCCESS)
		ret = i2c_write(i2c_chipAddress, cmd, cmd_size);
		   
	if(ret == CMD_SUCCESS)
		ret = si475x6x_wait_CTS();
		
	//read reply data
	if(reply_size)
	{
		ret = i2c_read(i2c_chipAddress,reply, reply_size);
	}
	if(ret != CMD_SUCCESS)
	{
		printf("si475x6x_command err : %0x", ret);
	}
	return ret;
}

unsigned char si475x6x_set_property(uint16_t propNumber, uint16_t propValue)
{
    // Put the ID for the command in the first byte.
    cmd[0] = SET_PROPERTY;

	// Initialize the reserved section to 0
    cmd[1] = 0;

	// Put the property number in the third and fourth bytes.
    cmd[2] = (uint8_t)(propNumber >> 8);
	cmd[3] = (uint8_t)(propNumber & 0x00FF);

	// Put the property value in the fifth and sixth bytes.
    cmd[4] = (uint8_t)(propValue >> 8);
    cmd[5] = (uint8_t)(propValue & 0x00FF);

    // Invoke the command
	return si475x6x_command(6, cmd, 0, NULL);
}

uint16_t si475x6x_get_property(uint16_t add)
{
	// Put the ID for the command in the first byte.
    cmd[0] = GET_PROPERTY;
	cmd[1] = 0;
	cmd[2] = (uint8_t)(add >> 8);
	cmd[3] = (uint8_t)add;

	// Invoke the command
	si475x6x_command(4, cmd, 4, rsp);

	return ((uint16_t)rsp[2] << 8) | (((uint16_t)rsp[3]) & 0x00ff);
}
// update  si475x6x_fm_rsq_status
void si475x6x_fm_rsq_status(uint8_t attune, uint8_t cancel, uint8_t intack, tuner_metrics_t *p_tuner_metrics)
{
    // Put the ID for the command in the first byte.
    cmd[0] = FM_RSQ_STATUS;

	// Depending on the passed parameters set the second argument
	cmd[1] = 0;
	
	if(intack) cmd[1] |= 0x01;
	if(cancel) cmd[1] |= 0x02;
	if(attune) cmd[1] |= 0x04;

	// Invoke the command
	si475x6x_command(2, cmd, 16, rsp);

    // Parse the results
    p_tuner_metrics->Valid   = !!(rsp[2] & 0x01);
    p_tuner_metrics->Freq   = ((uint16_t)rsp[3] << 8) | (uint16_t)rsp[4];
    p_tuner_metrics->FreqOff = rsp[5];
    p_tuner_metrics->RSSI   = rsp[6];
    p_tuner_metrics->SNR   = rsp[7];
}

void si475x6x_am_rsq_status(uint8_t attune, uint8_t cancel, uint8_t intack, tuner_metrics_t *p_tuner_metrics)
{
    // Put the ID for the command in the first byte.
    cmd[0] = AM_RSQ_STATUS;

	// Depending on the passed parameters set the second argument
	cmd[1] = 0;
	
	if(intack) cmd[1] |= 0x01;
	if(cancel) cmd[1] |= 0x02;
	if(attune) cmd[1] |= 0x04;

	// Invoke the command
	si475x6x_command(2, cmd, 13, rsp);

    // Parse the results
    p_tuner_metrics->Valid   = !!(rsp[2] & 0x01);
    p_tuner_metrics->Freq   = ((uint16_t)rsp[3] << 8) | (uint16_t)rsp[4];
    p_tuner_metrics->FreqOff = rsp[5];
    p_tuner_metrics->RSSI   = rsp[6];
    p_tuner_metrics->SNR   = rsp[7];
}


uint16_t si475x6x_fm_get_frequency()
{
	// Get the tune status which contains the current frequency
    si475x6x_fm_rsq_status(0, 0, 0, &g_tuner_metrics);

    // Return the frequency
    return g_tuner_metrics.Freq;
}

uint16_t si475x6x_am_get_frequency(void)
{
	// Get the tune status which contains the current frequency
    si475x6x_am_rsq_status(0, 0, 0, &g_tuner_metrics);

    // Return the frequency
    return g_tuner_metrics.Freq;
}


void si475x6x_set_volume(uint8_t value)
{
	// Turn off the mute
    si475x6x_set_property(AUDIO_MUTE, 0);

    // Set the volume to the passed value
    si475x6x_set_property(AUDIO_ANALOG_VOLUME, value);
}

void si475x6x_set_seek_freq_step(uint8_t value)
{
	// Turn off the mute
    si475x6x_set_property(AUDIO_MUTE, 0);

    // Set the volume to the passed value
    si475x6x_set_property(FM_SEEK_FREQUENCY_SPACING, value);
}

void si475x6x_mute(uint8_t value)
{
	if (value)
		si475x6x_set_property(AUDIO_MUTE, 3);
	else
		si475x6x_set_property(AUDIO_MUTE, 0);
}

unsigned char si475x6x_loadPatch(const uint8_t* patchData, uint16_t patchSize)
{
	unsigned char ret;
	uint16_t xdata increment = 0;
	uint8_t  i = 0;

	for(;increment < patchSize; increment += 8)
	{
		for(i=0; i < 8; i++)
		{
			cmd[i] = patchData[increment+i];
		}
		
		ret = si475x6x_command(8, cmd, 1, rsp);
		if(CMD_SUCCESS != ret)
			break;
	}

	return ret;
}

void si475x6x_powerdown(void)
{
	// Put the ID for the command in the first byte.
	cmd[0] = POWER_DOWN;

	// Invoke the command
	si475x6x_command(1, cmd, 0, NULL);
}

static int8_t si475x6x_fmRsqData(uint8_t attune, uint8_t cancel, uint8_t intack, uint8_t dataindex)
{
    // Put the ID for the command in the first byte.
    cmd[0] = FM_RSQ_STATUS;

	// Depending on the passed parameters set the second argument
	cmd[1] = 0;
	
	if(intack) cmd[1] |= 0x01;
	if(cancel) cmd[1] |= 0x02;
	if(attune) cmd[1] |= 0x04;

	// Invoke the command
	si475x6x_command(2, cmd, 16, rsp);

    if (dataindex < 16)
        return rsp[dataindex];
    else 
        return 0;
}

int8_t si475x6x_fm_get_OFFSET(void)
{
	return si475x6x_fmRsqData(0, 0, 0, OFFSET_DATA_INDEX); 
}
int8_t si475x6x_fm_get_RSSI(void)
{
	return si475x6x_fmRsqData(0, 0, 0, RSSI_DATA_INDEX); 
}
int8_t si475x6x_fm_get_SNR(void)
{
	return si475x6x_fmRsqData(0, 0, 0, SNR_DATA_INDEX); 
}

int8_t si475x6x_fm_get_valid(void)
{
    int8_t v = si475x6x_fmRsqData(0, 0, 0, VALID_DATA_INDEX); 

    return (v & 0x01);
}

uint8_t si475x6x_fm_get_blend(void)
{
	// Put the ID for the command in the first byte.
    cmd[0] = FM_ACF_STATUS;

	// Depending on the passed parameters set the second argument
	cmd[1] = 1;
	// Invoke the command
	si475x6x_command(2, cmd, 8, rsp);

    // Return the RSSI level
    return rsp[7];//
}

uint8_t si475x6x_get_space(void)
{
	// Put the ID for the command in the first byte.
    cmd[0] = GET_PROPERTY;
	cmd[1] = 0;
	cmd[2] = 0x11;
	cmd[3] = 0x02;

	// Invoke the command
	si475x6x_command(4, cmd, 4, rsp);
//	__android_log_print(ANDROID_LOG_INFO,"si475x6x_get_space","rsp[2] = %0x,rsp[3] = %0x",rsp[2],rsp[3]);

	return (rsp[3] & 0x1F);
}


void si475x6x_get_FuncInfo(void)
{
	cmd[0] = FUNC_INFO;

	si475x6x_command(1, cmd, 7, rsp);
    g_func_info.FWMajor = rsp[1];
    g_func_info.FWMinor1 = rsp[2];
    g_func_info.FWMinor2 = rsp[3];
    g_func_info.patchIdH = rsp[4];
    g_func_info.patchIdL = rsp[5];
}

unsigned char si475x6x_get_PartInfo(void)
{
	cmd[0] = PART_INFO;

	// Invoke the command
	if(CMD_SUCCESS != si475x6x_command(1, cmd, 9, rsp))
		return HAL_ERROR;

	// Now take the result and put in the variables we have declared
	// Status is in the first element of the array so skip that.
	g_part_info.chipRW = rsp[1];
	g_part_info.PartNum = rsp[2];
	g_part_info.FWMajorRW = (char)rsp[3];
	g_part_info.FWMinorRW = (char)rsp[4];
	g_part_info.FWBuildRW = rsp[5];
	g_part_info.ROMId = rsp[8];

	__android_log_print(ANDROID_LOG_ERROR, "info","chipRW: %0X ", g_part_info.chipRW );
	__android_log_print(ANDROID_LOG_ERROR, "info","PartNum: %0X ", g_part_info.PartNum );
	__android_log_print(ANDROID_LOG_ERROR, "info","FWMajorRW: %0X ", g_part_info.FWMajorRW -0x30);
	__android_log_print(ANDROID_LOG_ERROR, "info","FWMinorRW: %0X ", g_part_info.FWMinorRW -0x30);
	__android_log_print(ANDROID_LOG_ERROR, "info","FWBuildRW: %0X ", g_part_info.FWBuildRW );
	__android_log_print(ANDROID_LOG_ERROR, "info","ROMId: %0X ", g_part_info.ROMId );

	return CMD_SUCCESS;
}


static int8_t si475x6x_amRsqData(uint8_t attune, uint8_t cancel, uint8_t intack, uint8_t dataindex)
{
    // Put the ID for the command in the first byte.
    cmd[0] = AM_RSQ_STATUS;

	// Depending on the passed parameters set the second argument
	cmd[1] = 0;
	
	if(intack) cmd[1] |= 0x01;
	if(cancel) cmd[1] |= 0x02;
	if(attune) cmd[1] |= 0x04;

	// Invoke the command
	si475x6x_command(2, cmd, 13, rsp);

    if (dataindex < 13)
        return rsp[dataindex];
    else
        return 0;
}

int8_t si475x6x_am_get_OFFSET()
{
	return si475x6x_amRsqData(0, 0, 0, OFFSET_DATA_INDEX); 
}
int8_t si475x6x_am_get_RSSI()
{
	return si475x6x_amRsqData(0, 0, 0, RSSI_DATA_INDEX); 
}
int8_t si475x6x_am_get_SNR(void)
{
	return si475x6x_amRsqData(0, 0, 0, SNR_DATA_INDEX); 
}

int8_t si475x6x_am_get_valid(void)
{
    int8_t v = si475x6x_amRsqData(0, 0, 0, VALID_DATA_INDEX); 

    return (v & 0x01);
}

uint8_t si475x6x_get_DieTemp(void)
{
    uint8_t temp = 0;
    // start the loop
    cmd[0] = GET_TEMP;
    cmd[1] = 0x00;
    si475x6x_command(2, cmd, 3, rsp);
    
    delay_ms(100);
    
    //get the temp
    si475x6x_command(2, cmd, 3, rsp);
    temp = rsp[2];
    
    //stop the loop
    cmd[1] = 0x01;
    // Invoke the command
    si475x6x_command(2, cmd, 3, rsp);

    return temp;
}

