#include "PluginSE_pt2313.h"
#include <thread>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <android/log.h>
#include "pt2313e.h"
#include <linux/ioctl.h>
#include <condition_variable>
#include <queue>
#include <unistd.h>
#include "log.h"
#include <AsyncSerialCall.h>
static const char version[] = "0.0.1";
static PluginSe_pt2313 * instance = new PluginSe_pt2313 ;



static int  preVolume = 60 ;  // 之前的音量

static AsyncSerialCall *se_asc; // 必须使用 new 出来的

/**
 *@description:  // 逐步设置音量变化
 *@author: sky
 *@param p1[in] 原始值
 *@param p2[in] 目标值
 *@return 
 *@date: 2024-11-21 17:44:45
*/
static int  SlowVolumeChange(FuncAndParam const &fp)
{   
   
    int p1 = fp.p1;
    int p2 = fp.p2;
    LOGD("SlowVolumeChange p1:%d,p2 %d",p1,p2);
    int dif = int(fp.p1 - fp.p2);
    int sombl =    dif > 0? -1:1; // 符号判断
    int step =  2;        // 一次调节步长
    int interval = 3000;  // 间隔一毫秒
   
    dif = abs(dif);
    if(abs(dif) < step)
    {
        pt2313_set_volume((int)fp.p2);
        
        return 0;
    }
  
    
    for(;dif >= step;dif -= step ){
        p1 += step * sombl;
      //  LOGD("volume %d",p1);
        pt2313_set_volume(p1);
        usleep(interval);
    }
     pt2313_set_volume(p2);

   
   return 0;

}

static int SlowSetSource(FuncAndParam const & fp)
{
    int p1 = (int)fp.p1;
    LOGD("SlowSetSource  preVolume:%d ",p1);
    pt2313_set_mute(1);
    SlowVolumeChange(FuncAndParam(nullptr,preVolume,1));
  
    //usleep(100000);
    pt2313_set_input_gain(0);
    pt2313_set_source(p1);
   // usleep(1000*1500);
    pt2313_set_mute(0);
    SlowVolumeChange(FuncAndParam(nullptr,1,preVolume));
    //pt2313_set_mute(0);
   

   return 0;

}


    

extern "C" PluginInterface* createPlugin() {

    return instance;
}


static int device_init(FuncAndParam const & fp){

    LOGD("PluginSe_pt2313  device_init");
    pt2313_device_init();
    
}

static int get_status(FuncAndParam const & fp){

    LOGD("PluginSe_pt2313  get_status");
    int res =  pt2313_opened();
    instance->Notify(PluginSe_pt2313::SE_GET_STATUS,std::to_string(res));
}


PluginSe_pt2313::PluginSe_pt2313(){
    
    se_asc = new AsyncSerialCall();
    se_asc->SetLogTag("pt2313");
    se_asc->AddFunctionToQueue(FuncAndParam(device_init,0,0,0,0,"device_init"));
    LOGD("PluginSe_pt2313 ");
}
PluginSe_pt2313::~PluginSe_pt2313()
{

}

int PluginSe_pt2313::execute(int key,std::string const &data)
{
  return  executeEx(key,data);
}

int PluginSe_pt2313::executeEx(int key,std::string const &data,double p1 ,double p2, double p3 ,double p4 )
{
    LOGD( "PluginSe_pt2313 execute key:%d,data:%s,datalen:%d",key,data);
 //   std::cout << "PluginSe_pt2313 key:" << key << " ,data:" << (char* )data  << std::endl;

    int val = 0;
    int res = -1;
    val = atoi(data.c_str());
    switch (key)
    {
        case SE_INIT:
           
            se_asc->AddFunctionToQueue(FuncAndParam(device_init,0,0,0,0,"SE_INIT"));
          
            break;
        case SE_CLOSE:
           
            res=pt2313_device_close();
            break;
        
        case SE_SET_VOLUME:
        {
            LOGD("SE_SET_VOLUME  preVolume:%d,val %d",preVolume,val);
           { 
                se_asc->AddFunctionToQueue(FuncAndParam(SlowVolumeChange,preVolume,val,0,0,"SE_SET_VOLUME"));
                preVolume = val;
           }
           
           
            break;
        }
        case SE_SET_MUTE:
            
            res = pt2313_set_mute(val);
            break;
        case SE_SET_SOURCE:
        
            se_asc->AddFunctionToQueue(FuncAndParam(SlowSetSource,val,0,0,0,"SE_SET_SOURCE"));
            LOGD("SE_SET_SOURCE ");
            break;
        
        case SE_GET_STATUS:{
            LOGD("SE_SET_SOURCE ");
            se_asc->AddFunctionToQueue(FuncAndParam(get_status,0,0,0,0,"SE_GET_STATUS"));
           
        }
           
        
    default:
        break;
    }
   
    return res;
}
