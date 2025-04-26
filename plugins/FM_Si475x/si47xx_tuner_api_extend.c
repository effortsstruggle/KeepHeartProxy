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
//#include <main.h>

#include <stddef.h>
#include "Drv_Si4754C.h"
#include "si47xx_tuner_api_extend.h"
#include "si47xx_tuner_api_core.h"

#include <string.h>


#include "si475x_core.h"
#include "si475x_extend.h"

//#include "si476x_core.h"
//#include "si476x_extend.h"
//#include "si476x_pd_core.h"
//#include "si476x_pd_extend.h"

#if SUPPORT_RDS
#include "rds.h"
uint8_t xdata tune_action;
#endif

void GetCurrentSNR(uint8_t nBandMode)
{
    if (g_tuner_type == TUNER_SI475X_SINGLE)
    {
        if(nBandMode==BAND_FM)
        {
            gFmSnr = si475x_fm_get_SNR();
    	}
        else
        {
    		gAmSnr = si475x_am_get_SNR();
    	}
    }
#if 0	
    else if (g_tuner_type == TUNER_SI476X_SINGLE)
    {
        if(nBandMode==BAND_FM)
        {
            gFmSnr = si476x_fm_get_SNR();
        }
        else
        {
            gAmSnr = si476x_am_get_SNR();
        }
    }
    else if (g_tuner_type == TUNER_SI476X_DUAL)
    {
        if(nBandMode==BAND_FM)
        {
            gFmSnr = si476x_pd_fm_get_SNR();
        }
        else
        {
            gAmSnr = si476x_pd_am_get_SNR();
        }
    }
#endif	
}

void GetCurrentRssi(uint8_t nBandMode)
{
    if (g_tuner_type == TUNER_SI475X_SINGLE)
    {
        if (nBandMode==BAND_FM)
        {
            gFmRssi = si475x_fm_get_RSSI();
        }
        else
        {
            gAmRssi = si475x_am_get_RSSI();
        }
    }
#if 0	
    else if (g_tuner_type == TUNER_SI476X_SINGLE)
    {
        if (nBandMode==BAND_FM)
        {
            gFmRssi = si476x_fm_get_RSSI();
        }
        else
        {
            gAmRssi = si476x_am_get_RSSI();
        }        
    }
    else if (g_tuner_type == TUNER_SI476X_DUAL)
    {
        if (nBandMode==BAND_FM)
        {
            gFmRssi = si476x_pd_fm_get_RSSI();
        }
        else
        {
            gAmRssi = si476x_pd_am_get_RSSI();
        }        
    }
#endif
}

void GetCurrentValid(uint8_t nBandMode)
{
    if (g_tuner_type == TUNER_SI475X_SINGLE)
    {
        if (nBandMode==BAND_FM)
            gFmValid= si475x_fm_get_valid();
        else
            gAmValid = si475x_am_get_valid();
    }
#if 0
    else if (g_tuner_type == TUNER_SI476X_SINGLE)
    {
        if (nBandMode==BAND_FM)
            gFmValid = si476x_fm_get_valid();
        else
            gAmValid = si476x_am_get_valid();
    }
    else if (g_tuner_type == TUNER_SI476X_DUAL)
    {
        if (nBandMode==BAND_FM)
            gFmValid = si476x_pd_fm_get_valid();
        else
            gAmValid = si476x_pd_am_get_valid();
    }
#endif	
}

void GetCurrentBlend(void)
{
    if (g_tuner_type == TUNER_SI475X_SINGLE)
	    gFmBlend = si475x_fm_get_blend();
#if 0	
    else if (g_tuner_type == TUNER_SI476X_SINGLE)
        gFmBlend = si476x_fm_get_blend();
    else if (g_tuner_type == TUNER_SI476X_DUAL)
        gFmBlend = si476x_pd_fm_get_blend();
#endif	
}

void GetCurrentOffset(uint8_t nBandMode)
{
    if (g_tuner_type == TUNER_SI475X_SINGLE)
    {
        if (nBandMode==BAND_FM)
            gFreqOffset= si475x_fm_get_offset();
        else
            gFreqOffset = si475x_am_get_offset();
    }
}

void Select_Space(uint8_t nBandMode)
{
    if (g_tuner_type == TUNER_SI475X_SINGLE)
    {
    	switch(nBandMode)
    	{
    		case BAND_FM:
    			if(gFmSpace==0)
                {
    				si475x_set_property(FM_SEEK_CHBW, 0x50); //80k
          	  		si475x_set_property(FM_SEEK_FREQUENCY_SPACING, 5); // 50 kHz Spacing
    			}
                else
                {
    				si475x_set_property(FM_SEEK_CHBW, 0x6e); //110k

                    if(gFmSpace==1)
    					si475x_set_property(FM_SEEK_FREQUENCY_SPACING, 10); // 100 kHz Spacing
    				else
    					si475x_set_property(FM_SEEK_FREQUENCY_SPACING, 20); // 200 kHz Spacing
    			}
        		break;

    		case BAND_AM:
    			if(gAmSpace==0)
                {
    				si475x_set_property(AM_SEEK_FREQUENCY_SPACING, 9); // Set spacing to 9kHz
    				si475x_set_property(AM_SEEK_BAND_BOTTOM, AM_BOT_FREQ); // Set the band bottom to 520kHz
    				si475x_set_property(AM_SEEK_BAND_TOP, AM_TOP_FREQ);	 // Set the band top to 1710kHz
    			}
                else// if(gAmSpace==1)
                {
    				si475x_set_property(AM_SEEK_FREQUENCY_SPACING, 10); // Set spacing to 10kHz
    				si475x_set_property(AM_SEEK_BAND_BOTTOM, AM_BOT_FREQ_10K); // Set the band bottom to 520kHz
    				si475x_set_property(AM_SEEK_BAND_TOP, AM_TOP_FREQ);	 // Set the band top to 1710kHz
    			}
        		break;

            default:
    			break;
    	}	
    }
#if 0	
    else if (g_tuner_type == TUNER_SI476X_SINGLE)
    {
        switch(nBandMode)
    	{
    		case BAND_FM:
    			if(gFmSpace==0)
                {
    				si476x_set_property(FM_SEEK_CHBW, 0x50); //80k
          	  		si476x_set_property(FM_SEEK_FREQUENCY_SPACING, 5); // 50 kHz Spacing
    			}
                else
                {
    				si476x_set_property(FM_SEEK_CHBW, 0x6e); //110k

                    if(gFmSpace==1)
    					si476x_set_property(FM_SEEK_FREQUENCY_SPACING, 10); // 100 kHz Spacing
    				else
    					si476x_set_property(FM_SEEK_FREQUENCY_SPACING, 20); // 200 kHz Spacing
    			}
        		break;

    		case BAND_AM:
    			if(gAmSpace==0)
                {
    				si476x_set_property(AM_SEEK_FREQUENCY_SPACING, 9); // Set spacing to 9kHz
    				si476x_set_property(AM_SEEK_BAND_BOTTOM, AM_BOT_FREQ); // Set the band bottom to 520kHz
    				si476x_set_property(AM_SEEK_BAND_TOP, AM_TOP_FREQ);	 // Set the band top to 1710kHz
    			}
                else// if(gAmSpace==1)
                {
    				si476x_set_property(AM_SEEK_FREQUENCY_SPACING, 10); // Set spacing to 10kHz
    				si476x_set_property(AM_SEEK_BAND_BOTTOM, AM_BOT_FREQ_10K); // Set the band bottom to 520kHz
    				si476x_set_property(AM_SEEK_BAND_TOP, AM_TOP_FREQ);	 // Set the band top to 1710kHz
    			}
        		break;

            default:
    			break;
    	}
    }
    else if (g_tuner_type == TUNER_SI476X_DUAL)
    {
        switch(nBandMode)
    	{
    		case BAND_FM:
    			if(gFmSpace==0)
                {
    				si476x_pd_primary_set_property(FM_SEEK_CHBW, 0x50); //80k
          	  		si476x_pd_primary_set_property(FM_SEEK_FREQUENCY_SPACING, 5); // 50 kHz Spacing
          	  		
    				si476x_pd_secondary_set_property(FM_SEEK_CHBW, 0x50); //80k
          	  		si476x_pd_secondary_set_property(FM_SEEK_FREQUENCY_SPACING, 5); // 50 kHz Spacing
    			}
                else
                {
    				si476x_pd_primary_set_property(FM_SEEK_CHBW, 0x6e); //110k
    				si476x_pd_secondary_set_property(FM_SEEK_CHBW, 0x6e); //110k

                    if(gFmSpace==1)
                    {
    					si476x_pd_primary_set_property(FM_SEEK_FREQUENCY_SPACING, 10); // 100 kHz Spacing
    					si476x_pd_secondary_set_property(FM_SEEK_FREQUENCY_SPACING, 10); // 100 kHz Spacing
                    }
                    else
                    {
    					si476x_pd_primary_set_property(FM_SEEK_FREQUENCY_SPACING, 20); // 200 kHz Spacing
    					si476x_pd_secondary_set_property(FM_SEEK_FREQUENCY_SPACING, 20); // 200 kHz Spacing
			        }
                }
        		break;

    		case BAND_AM:
    			if(gAmSpace==0)
                {
    				si476x_pd_primary_set_property(AM_SEEK_FREQUENCY_SPACING, 9); // Set spacing to 9kHz
    				si476x_pd_primary_set_property(AM_SEEK_BAND_BOTTOM, AM_BOT_FREQ); // Set the band bottom to 520kHz
    				si476x_pd_primary_set_property(AM_SEEK_BAND_TOP, AM_TOP_FREQ);	 // Set the band top to 1710kHz
    			}
                else// if(gAmSpace==1)
                {
    				si476x_pd_primary_set_property(AM_SEEK_FREQUENCY_SPACING, 10); // Set spacing to 10kHz
    				si476x_pd_primary_set_property(AM_SEEK_BAND_BOTTOM, AM_BOT_FREQ_10K); // Set the band bottom to 520kHz
    				si476x_pd_primary_set_property(AM_SEEK_BAND_TOP, AM_TOP_FREQ);	 // Set the band top to 1710kHz
    			}
        		break;

            default:
    			break;
    	}
    }
#endif	
}

uint16_t GetFreqence(uint8_t nBandMode)
{
    if (g_tuner_type == TUNER_SI475X_SINGLE)
    {
        if (nBandMode == BAND_FM)
        {
            return si475x_fm_get_frequency();
        }
        else
        {
            return si475x_am_get_frequency();
        }
    }
#if 0	
    else if (g_tuner_type == TUNER_SI476X_SINGLE)
    {
        if (nBandMode == BAND_FM)
        {
            return si476x_fm_get_frequency();
        }
        else
        {
            return si476x_am_get_frequency();
        }
    }
    else if (g_tuner_type == TUNER_SI476X_DUAL)
    {
        if (nBandMode == BAND_FM)
        {
            return si476x_pd_fm_get_frequency();
        }
        else
        {
            return si476x_pd_am_get_frequency();
        }
    }
#endif
	return 0;
}

uint8_t GetSpace(void)
{
    if (g_tuner_type == TUNER_SI475X_SINGLE)
	    return si475x_get_space();
#if 0	
    else if (g_tuner_type == TUNER_SI476X_SINGLE)
        return si476x_get_space();
    else if (g_tuner_type == TUNER_SI476X_DUAL)
        return si476x_pd_get_space();
#endif
	return 0;
}

//unit is 10k for fm, unit 1k for am, and range is from 1~31
void SetFreqSeekSpace(uint8_t space){
    si475x_set_space(space);
}

void SwitchStereoMono(uint8_t stereo)
{
    if (g_tuner_type == TUNER_SI475X_SINGLE)
    {
    	if (stereo != 0)
    		si475x_set_property(0x3501, (uint16_t)0x2D00);
    	else
    		si475x_set_property(0x3501, (uint16_t)0x0000);
    }
#if 0
	else if (g_tuner_type == TUNER_SI476X_SINGLE)
    {
        if (stereo != 0)
    		si476x_set_property(0x3501, (uint16_t)0x2D00);
    	else
    		si476x_set_property(0x3501, (uint16_t)0x0000);
    }
    else if (g_tuner_type == TUNER_SI476X_DUAL)
    {
        if (stereo != 0)
    		si476x_pd_primary_set_property(0x3501, (uint16_t)0x2D00);
    	else
    		si476x_pd_primary_set_property(0x3501, (uint16_t)0x0000);
    }
#endif	
}

void GetFuncInfo(uint8_t tunerId)
{
    if (g_tuner_type == TUNER_SI475X_SINGLE)
        si475x_get_FuncInfo();
#if 0
	else if (g_tuner_type == TUNER_SI476X_SINGLE)
        si476x_get_FuncInfo();
    else if (g_tuner_type == TUNER_SI476X_DUAL)
    {
        if (tunerId == DUAL_PRIMARY_TUNER)
            si476x_pd_get_primary_FuncInfo();
        else
            si476x_pd_get_second_FuncInfo();
    }
#endif	
}

extern uint8_t xdata gTunerFWRevision;  
unsigned char GetPartInfo(uint8_t tunerId)
{
	unsigned char ret = HAL_ERROR;
	
    if (g_tuner_type == TUNER_SI475X_SINGLE)
        ret = si475x_get_PartInfo();
#if 0
	else if (g_tuner_type == TUNER_SI476X_SINGLE)
        si476x_get_PartInfo();
    else if (g_tuner_type == TUNER_SI476X_DUAL)
    {
        if (tunerId == DUAL_PRIMARY_TUNER)
            si476x_pd_get_primary_PartInfo();
        else
            si476x_pd_get_second_PartInfo();
    }
#endif	
	if(ret == CMD_SUCCESS)
    gTunerFWRevision = ((g_part_info.FWMajorRW-0x30)*10)+(g_part_info.FWMinorRW-0x30);

	return ret;
}

/**
 *@description: 切换AM/FM模式
 *@author: sky
 *@param nBandMode[in] 制定的模式

 *@date: 2024-07-25 14:47:10
*/
void SwitchBand(uint8_t nBandMode)
{
	g_band = nBandMode;
    if (g_tuner_type == TUNER_SI475X_SINGLE)
        si475x_bandswitch(nBandMode);
#if 0
	else if (g_tuner_type == TUNER_SI476X_SINGLE)
        si476x_bandswitch(nBandMode);
    else if (g_tuner_type == TUNER_SI476X_DUAL)
        si476x_pd_bandswitch(nBandMode);
#endif	
}

#if SUPPORT_RDS
//0 not avaiable
//0x01 PS available
//0x02 RT available
uint8_t IsRDSPsAvailable(void)
{
    return rds_ps_available();
}

uint8_t IsRDSRtAvailable(void)
{
    return rds_rt_available();
}

uint8_t xdata *GetRDSPS(void)
{
    return rds_ps_data();
}

uint8_t xdata *GetRDSRT(void)
{
    return rds_rt_data();
}

void ProcessRDS(void)
{
    rds_process();
}

uint16_t AFCheck(void)
{
    return rds_af_check();
}


uint8_t AFTune(uint16_t freq)
{
    uint8_t result;

    result = rds_af_tune(freq);

    if (result != 0)
    {        
        tune_action = 1;
    
        rds_reset();
    }

    return result;
}

#endif

uint8_t GetDieTempereture(uint8_t tunerId)
{
    if (g_tuner_type == TUNER_SI475X_SINGLE)
        return si475x_get_DieTemp();
 #if 0
 	else if (g_tuner_type == TUNER_SI476X_SINGLE)
        return si476x_get_DieTemp();
    else if (g_tuner_type == TUNER_SI476X_DUAL)
    {
        if (tunerId == DUAL_PRIMARY_TUNER)
            return si476x_pd_get_primary_DieTemp();
        else if (tunerId == DUAL_SECONDARY_TUNER)
            return si476x_pd_get_second_DieTemp();
        else 
            return 0;
    }
   #endif 
	return 0;
}

