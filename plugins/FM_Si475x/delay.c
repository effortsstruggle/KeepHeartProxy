
#include "delay.h"
#include <unistd.h>

//////////////////////////////////////////////////////////////////////////////////
//********************************************************************************
uint32_t Timer_Cnt_Rotary;
uint32_t Timer_Cnt_Key;

//////////////////////////////////////////////////////////////////////////////////	 

void delay_ms(uint32_t nms) {
    usleep(nms * 1000);
}

void delay_us(uint32_t nus) {
    usleep(nus);
}

//every TIMER_TICK 
//void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim) {
//    if (htim->Instance == TIM14)//rotary timer
//    {
//        Get_Rotary_GPIO();
//    }
//}

/*****************************************************************************
*  Name        : TimerHasExpired
*  Description : Checks whether given timer has expired
*        With timer tick at 1ms maximum timer period is 10000000 ticks
*        When *STimer is set (SoftTimer-*STimer) has a min value of 0xFFF0BDBF
*            and will be more than this while the timer hasn't expired
*        When the timer expires
*                (SoftTimer-*STimer)==0
*            and (SoftTimer-*STimer)<=7FFF FFFF for the next 60 hours
*  Params      : STimer pointer to timer value
*  Returns     :  TRUE if timer has expired or timer is stopped, otherwise FALSE
*****************************************************************************/
//u8 TimerHasExpired(u32 *STimer) {
//    if (*STimer == 0)
//        return 0;
//    else if ((HAL_GetTick() - *STimer) <= 0x7fffffff) {
//        *STimer = 0;    //set timer to stop
//        return 1;
//    } else
//        return 0;
//}
//
//char TimerHasExpiredOrStop(uint32_t *STimer) {
//    if (*STimer == 0)
//        return 1;
//    else if ((HAL_GetTick() - *STimer) <= 0x7fffffff) {
//        *STimer = 0;    //set timer to stop
//        return 1;
//    } else
//        return 0;
//}

/*****************************************************************************
*  Name        : TimerSet
*  Description : Sets a timer. timer must not equ 0 because 0 means timer is stop
*  Params      : STimer pointer to timer value
*                TimeLength - timer period
*  Returns     : none
*****************************************************************************/
//void TimerSet(u32 *STimer, u32 TimeLength) {
//    if (TimeLength) {
//        *STimer = HAL_GetTick() + TimeLength / TIMER_TICK;
//
//        if (*STimer == 0) *STimer = 1; //not set timer to 0 for timer is running
//    } else
//        *STimer = 0;//stop timer
//}
//
//static uint32_t Timer_tuner;

//void Tuner_Poll(void);
//
//void TimerPoll(void) {
////---------------Rotary
//    if (TimerHasExpired(&Timer_Cnt_Rotary)) {
//        TimerSet(&Timer_Cnt_Rotary, 30 / TIMER_TICK);
//
//        if (Rotary_Knob != 0) {
//            Process_Rotary(Rotary_Knob);
//
//            Rotary_Knob = 0;
//        }
//    }
//
////---------------Key
//    if (TimerHasExpired(&Timer_Cnt_Key)) {
//        TimerSet(&Timer_Cnt_Key, 30 / TIMER_TICK);//check key
//
//        Check_Key();
//    }
//
////---------------tuner poll
//    if (TimerHasExpiredOrStop(&Timer_tuner)) {
//        TimerSet(&Timer_tuner, 300 / TIMER_TICK);//check key
//
//        Tuner_Poll();
//    }
//}

