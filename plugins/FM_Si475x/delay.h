#ifndef __DELAY_H
#define __DELAY_H 			   
#include "typedefs.h"

#define TIMER_TICK	1


extern uint32_t Timer_Cnt_Rotary;
extern uint32_t Timer_Cnt_Key;

void delay_ms(uint32_t nms);
void delay_us(uint32_t nus);
//void TimerPoll(void);
//u8 TimerHasExpired (u32 *STimer);
//char TimerHasExpiredOrStop (uint32_t *STimer);
//void TimerSet(u32 *STimer, u32 TimeLength);

#endif

