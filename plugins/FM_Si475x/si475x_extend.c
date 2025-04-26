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


#include "Drv_Si4754C.h"

#include "si475x_extend.h"
#include "si475x6x_common.h"

int8_t si475x_fm_get_RSSI(void)
{
	return si475x6x_fm_get_RSSI(); 
}
int8_t si475x_fm_get_SNR(void)
{
	return si475x6x_fm_get_SNR(); 
}

int8_t si475x_fm_get_valid(void)
{
    return si475x6x_fm_get_valid();
}

uint8_t si475x_fm_get_blend(void)
{
    return si475x6x_fm_get_blend();
}
int8_t si475x_fm_get_offset(void)
{
    return si475x6x_fm_get_OFFSET();
}
int8_t si475x_am_get_offset(void)
{
    return si475x6x_am_get_OFFSET();
}

uint8_t si475x_get_space(void)
{
    return si475x6x_get_space();
}
void  si475x_set_space(uint16_t space)
{
    return si475x6x_set_seek_freq_step(space);
}
void si475x_get_FuncInfo(void)
{
    si475x6x_get_FuncInfo();
}

unsigned char si475x_get_PartInfo(void)
{
    return si475x6x_get_PartInfo();
}

int8_t si475x_am_get_RSSI(void)
{
	return si475x6x_am_get_RSSI(); 
}
int8_t si475x_am_get_SNR(void)
{
	return si475x6x_am_get_SNR(); 
}

int8_t si475x_am_get_valid(void)
{
    return si475x6x_am_get_valid();
}

uint8_t si475x_get_DieTemp(void)
{
    return si475x6x_get_DieTemp();
}


