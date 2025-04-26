#include "PluginSE_bd3703.h"
#include <thread>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <android/log.h>
#include "bd37033.h"
#include <linux/ioctl.h>
#include <condition_variable>
#include <queue>
#include <unistd.h>
#include "log.h"
#include <AsyncSerialCall.h>
static const char version[] = "V0.0.006";
PluginSE_bd3703 * instance = new PluginSE_bd3703 ;

static AsyncSerialCall *bd_asc ;

static int  preVolume = 60 ;  // 之前的音量

/**
 *@description:  // 逐步设置音量变化
 *@author: sky
 *@param p1[in] 原始值
 *@param p2[in] 目标值
 *@return 
 *@date: 2024-11-21 17:44:45
*/
static int  D_bd_set_volume(FuncAndParam const & fp)
{   
    int p1 = (int)fp.p1;
    bd37033_set_volume(p1);
    return 0;
}

static int D_bd_slow_set_source(FuncAndParam const & fp)
{

    LOGD("set_source will sleep - channel : %d , time : %f", (int)fp.p1 , fp.p2);
    // usleep(fp.p2 * 1000);
    // LOGD("set_source sleeped- channel : %d , time : %f", (int)fp.p1 , fp.p2);
    // LOGD("SlowSetSource  preVolume:%d ",preVolume);


    bd37033_set_mute(1);

    int p1 = (int)fp.p1;
    bd37033_set_source(p1);
    
    bd37033_set_mute(0);
    return 0;
}



static int D_device_init(FuncAndParam const & fp)
{
    bd37033_device_init();
}

static int D_bd_get_status(FuncAndParam const & fp){
    int res = bd37033_opened();

    instance->Notify(PluginSE_bd3703::SE_GET_STATUS,std::to_string(res));
}
extern "C" PluginInterface* createPlugin() {

    return instance;
}

PluginSE_bd3703::PluginSE_bd3703(){
    
    bd_asc = new AsyncSerialCall();
    bd_asc->SetLogTag("bd3703");
    bd_asc->AddFunctionToQueue(FuncAndParam(D_device_init,0,0,0,0,"device_init"));
    LOGD("PluginSE_bd3703 version %s",version);
}
PluginSE_bd3703::~PluginSE_bd3703()
{

}

int PluginSE_bd3703::execute(int key,std::string const &data)
{
    return  executeEx(key,data);
}

int PluginSE_bd3703::executeEx(int key,std::string const &data,double p1 ,double p2, double p3 ,double p4 )
{
    LOGD( "PluginSE_bd3703 execute key:%d,data:%s",key,data.c_str());
 //   std::cout << "PluginSE_bd3703 key:" << key << " ,data:" << (char* )data  << std::endl;

    int val = 0;
    int res = 0;
    val = atoi(data.c_str());
    switch (key)
    {
        case SE_INIT:
            // 加载库的时候会进行默认初始化
            
           // res = bd37033_device_init();
          //  LOGD("bd37033_device_init");
            break;
        case SE_CLOSE:
           
            res=bd37033_device_close();
            LOGD("bd37033_device_close");
            break;
        
        case SE_SET_VOLUME:
        {
            LOGD("SE_SET_VOLUME  val %d",val);
           
            bd_asc->AddFunctionToQueue(FuncAndParam(D_bd_set_volume,val,0,0,0,"SE_SET_VOLUME"));   
            break;
        }
        case SE_SET_MUTE:
            LOGD("bd37033_set_mute %d",val);
            res = bd37033_set_mute(val);
           
            break;
        case SE_SET_SOURCE:
        {
            bd_asc->AddFunctionToQueue(FuncAndParam(D_bd_slow_set_source,val,p1,0,0,"SE_SET_SOURCE"));

            LOGD("SE_SET_SOURCE %d",val);
            break;
        }
        case SE_GET_STATUS:
        {
            bd_asc->AddFunctionToQueue(FuncAndParam(D_bd_get_status,0,0,0,0,"SE_GET_STATUS"));
        }
    default:
        break;
    }
   
    return res;
}
