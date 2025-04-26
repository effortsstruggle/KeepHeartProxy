#include "PluginFm_Si4754.h"
#include "Drv_Si4754C.h"
#include <thread>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "cJSON.h"
#include <android/log.h>
#include "typedefs.h"
#include "si47xx_tuner_api_core.h"
#include <linux/ioctl.h>
#include <condition_variable>
#include <unistd.h>
#include <queue>
#include <AsyncSerialCall.h>
static uint8_t seeking = 0;  //搜台标记

PluginFm_Si4754 * instance = new PluginFm_Si4754 ;

static AsyncSerialCall * fm_asc = NULL;


static int  preVolume = 60 ;  // 之前的音量

static const char fm_vesion[] = "fm_ver: 0.00.001";


/**
 *@description:  // 逐步设置音量变化
 *@author: sky
 *@param p1[in] 原始值
 *@param p2[in] 目标值
 *@return 
 *@date: 2024-11-21 17:44:45
*/
static int  SlowVolumeChange(FuncAndParam const & fp)
{   
    int p1 = (int)fp.p1;
    int p2 = (int)fp.p2;

    LOGD("SlowVolumeChange p1:%d,p2 %d",p1,p2);
    int dif = p1 - p2;
    int sombl =    dif > 0? -1:1; // 符号判断
    int step =  2;        // 一次调节步长
    int interval = 1000;  // 间隔一毫秒
   
    dif = abs(dif);

  
    for(;dif >= step;dif -= step ){
        p1 += step * sombl;
    //   LOGD("volume %d",p1);
        SetVolume(p1);
        usleep(interval);
    }
    SetVolume(p2);

   LOGD("SlowVolumeChange end");
   return 0;

}

//初始化
static int D_Init_Si4754C(FuncAndParam const & fp)
{   
    LOGD("fm_vesion %s",fm_vesion);
    Init_Si4754C("",0);
 
    SlowVolumeChange(FuncAndParam(nullptr,0,0x63));

    return 0;
}
// 下电
static int  D_PowerDown(FuncAndParam const & fp)
{   
    //unused(fp);
    Powerdown();
}
// 自动搜台
static int D_startSeek(FuncAndParam const & fp){
    int p1 = (int)fp.p1;
    instance->startSeek(p1);     
} 

// 自动搜台停止
static int D_StopSeeking(FuncAndParam const & fp){
      StopSeeking();     
} 

// 静音
static int D_SetMute(FuncAndParam const & fp){
    
    int p1 = (int)fp.p1;
    int p2 = (int)fp.p2;

    int times = 10;
    int tmp = p1 == 1?0:1;

    while (times--)
    {
        SetMute(p1);     
        usleep(1000  *10);

        int res = GetMute();
        if(res == p1)
            break;

        SetMute(tmp);    
        usleep(1000  *10);
        LOGD("GetMute %d",res);
    }
   
} 

// 设置频率
static int D_Tuner_Tune(FuncAndParam const & fp){
    int p1 = (int)fp.p1;
   // LOGD ("设置频率 D_Tuner_Tune %d",p1);
    Tuner_Tune(p1);     
} 


void CheckErrorThread()
{
    LOGD("CheckErrorThread start success ");
    while (true)
    {
       int res = GetI2cError();
       if(res)
       {
            // 如果正在搜台，通知结束搜台
            LOGD("I2cError restart ");
            if(seeking == 1 )
            { 
                instance->Notify((int)RD_NOTIFY_SEEK_OVER,"nullptr");
                seeking = 0;
                fm_asc->AddFunctionToQueue(FuncAndParam(D_PowerDown,0.0,0.0,0.0,0.0,"restart，PowerDown"));
                fm_asc->AddFunctionToQueue(FuncAndParam(D_Init_Si4754C,0.0,0.0,0.0,0.0,"restart,init"));
      
            }
       }
       // test 主动调用
    //    LOGD("  cv.notify_one()  Warning")
    //    cv.notify_one();

       sleep(1);
    }
}
extern "C" PluginInterface* createPlugin() {

    return instance;
}



PluginFm_Si4754::PluginFm_Si4754(){

    fm_asc = new AsyncSerialCall();
    fm_asc->SetLogTag("asc-Si4754");
    fm_asc->AddFunctionToQueue(FuncAndParam(D_Init_Si4754C,0.0,0.0,0.0,0.0,"PluginFm_Si4754"));

    std::thread t2(CheckErrorThread);
    if(t2.joinable())
        t2.detach();

}
PluginFm_Si4754::~PluginFm_Si4754()
{

}
int PluginFm_Si4754::execute(int key,std::string const &data)
{
    return executeEx(key,data);
}   
int PluginFm_Si4754::executeEx(int key,std::string const &data ,double p1 ,double p2 , double p3 ,double p4 ){

    LOGD( "execute key:%d,data:%s ",key,data.c_str());
    int tmp =  atoi(data.c_str());
    int res = 0;
    
    switch (key)
    {
        
        case RD_SET_START:
        
            fm_asc->AddFunctionToQueue(FuncAndParam(D_Init_Si4754C,0.0,0.0,0.0,0.0,"RD_SET_START"));
            break;
        
        case RD_SET_STOP:
            LOGD("FM_STOP %d",RD_SET_STOP);
            fm_asc->AddFunctionToQueue(FuncAndParam(D_PowerDown,0.0,0.0,0.0,0.0,"RD_SET_STOP"));
            
            break;
        case RD_SET_SEEK:
            
            LOGD("RD_SET_SEEK %d",RD_SET_SEEK);  
            fm_asc->AddFunctionToQueue(FuncAndParam(D_startSeek,tmp,0.0,0.0,0.0,"RD_SET_SEEK"));
            break;
        case RD_SET_SEEK_STOP :
            
            fm_asc->AddFunctionToQueue(FuncAndParam(D_StopSeeking,tmp,0,0,0,"RD_SET_SEEK_STOP"));
            break;
        case RD_SET_SEEK_STEP_LEN:
           
            LOGD("RD_SET_SEEK_STEP_LEN %d",RD_SET_SEEK_STEP_LEN);
            SetSeekSpace(tmp);
            break;
        case RD_SET_SLEEP:
             LOGD("RD_SET_SLEEP un implement ");
            break; 
        case RD_SET_VOLUME:
  
            fm_asc->AddFunctionToQueue(FuncAndParam(SlowVolumeChange,res,tmp,0,0,"RD_SET_VOLUME"));
            break;
            
        case RD_SET_MUTE:
            
            fm_asc->AddFunctionToQueue(FuncAndParam(D_SetMute,tmp,0,0,0,"RD_SET_MUTE"));
            break;
            
             
        case RD_SET_RESET:
            LOGD("RD_SET_RESET un implement ");
            break;
        case RD_SET_TUNE:
             
            fm_asc->AddFunctionToQueue(FuncAndParam(D_Tuner_Tune,tmp,0,0,0,"RD_SET_TUNE"));
            break;
            
        case RD_SET_MODE:

            res = SetBandMode(tmp);

            break;
        case RD_GET_STATUS:           // 获取收音机状态 0 关闭； 1 运行中 
            res = GetStatus();
         break;
        case RD_GET_VOLUME:           // 获取当前音量

            res = GetVolume();
            break;
        case RD_GET_MUTE:             // 获取当前是否静音
            res = GetMute();
            break;
        case RD_GET_TUNE:             // 获取当前频率 
            res = GetFreq();
            break;
        case RD_GET_MODE:             // 设置当前模式 AM / FM
            res = GetBandMode();
            break;
        case RD_JSON:
             PraseJson(data.c_str());
            break;
    default:
        break;
    }
    
    return res;
}
int PluginFm_Si4754::PraseJson(const  char *  raw_string)
{

}
int PluginFm_Si4754::startSeek(int dir)
{
    if(seeking == 1)
    {
        LOGD("PluginFm_Si4754::startSeek  seeking %d",seeking) ;
        return -1;
    }
       

    seeking = 1;
    SetSeekUp(dir);
    std::thread t1(&PluginFm_Si4754::autoSeekThread,this);
    t1.detach();
   // std::swap(t1,mth);
   
  
    return 0;

}

void PluginFm_Si4754::stopSeek()
{
    StopSeeking();
}

extern "C" {

void callback_process_freq(uint16_t freq)
{
   
 //   printf("callback_process_freq: %d \n",freq)
    instance->Notify(RD_NOTIFY_SEEK_CUR_FREQ,std::to_string(freq));
}


void callback_found_freq(uint16_t freq)
{
    LOGD("callback_found_freq : %d ",freq);
    
    instance->Notify(RD_NOTIFY_SEEK_GOOD_FREQ,std::to_string(freq));
}
}

void  PluginFm_Si4754::autoSeekThread()
{
    LOGD("autoSeekThread start ");

    AutoSeek(callback_process_freq,callback_found_freq);
   
    Notify(RD_NOTIFY_SEEK_OVER,"nullptr");   
    seeking = 0;      
    
    LOGD("startSeek end "); 
}

// int main()
// {

//     instance->execute(1,"");
//     return 0;
// }