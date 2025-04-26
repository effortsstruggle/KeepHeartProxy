#include "SysCtl.h"
#include <thread>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "cJSON.h"
#include <sys/stat.h>
#include <android/log.h>
#include <mutex>
#include <stdio.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <unistd.h>
#include <string.h>
#include <sys/ioctl.h>
#include <linux/ioctl.h>
#include <condition_variable>
#include <queue>

#define  LOGD(...) __android_log_print(ANDROID_LOG_DEBUG, "SKY_SYS", __VA_ARGS__);

static std::mutex mtx;  // 创建一个互斥锁


static std::condition_variable cv;

typedef int (*PFUNC)(int, int);

struct funcTemp
{
   PFUNC pFunc;
   int p1;
   int p2;
};


// 简单的线程函数执下;有新的需要执行的函数就添加到队列，队列中负责执行;
static std::queue<funcTemp>  pFuncs ; //函数队列

static int setSysTime(int p1,int p2){
    struct timeval tv;
    tv.tv_sec = p1;  // 设置为一个新的时间，例如 1970-08-02 15:30:00
    tv.tv_usec = p2;

    if (settimeofday(&tv, NULL) == -1) {

        LOGD("settimeofday err ");
        return  0;
    }

   // LOGD("System time set successfully.\n");

}
 
 
// 背光设置强度
static  int lightarr[6] = {0,0,64,127,191,255};

int SetBacklight(int p1,int p2){

// command  dbglvl   info     name     param    start
   // p1*= 2.5;
    system("echo lcd0 > /sys/kernel/debug/dispdbg/name");

    system("echo setbl > /sys/kernel/debug/dispdbg/command");
    char buf[128] = {0};
    if(p1 >= 5)
        p1 = 5;
    if(p1 < 1)
        p1 =1;

    LOGD("SetBacklight p1 : %d， lightarr[p1] : %d",p1,lightarr[p1]);
    sprintf(buf,"echo %d > /sys/kernel/debug/dispdbg/param",lightarr[p1]);
    system(buf);  //# 范围 0~255
    //usleep(200);
    system("echo 1 > /sys/kernel/debug/dispdbg/start ");

}


SysCtl * instance = new SysCtl ;
extern "C" PluginInterface* createPlugin() {

    return instance;
}

void runthread()
{

    // 屏幕使能
    system("echo PE19 > /sys/kernel/debug/sunxi_pinctrl/sunxi_pin");
    system("echo PE19 1 > /sys/kernel/debug/sunxi_pinctrl/data");
    system("echo PE19 1 >  /sys/kernel/debug/sunxi_pinctrl/function");
    system("echo PE19 1 > /sys/kernel/debug/sunxi_pinctrl/data");


    //临时屏幕亮度调节 
    system("echo /sys/kernel/debug/dispdbg/lcd0 > name");
    system("echo /sys/kernel/debug/dispdbg/setbl > command");
    system("echo 100 > /sys/kernel/debug/dispdbg/param");
    usleep(2000);
    system("echo 1 > /sys/kernel/debug/dispdbg/start ");

    //声卡打开
    system(" amixer -Dhw:0 cset numid=4 on ");
    system(" amixer -Dhw:0 cset numid=9 1 ");
    system(" amixer -Dhw:0 cset numid=10 1 ");
    system(" amixer -Dhw:0 cset numid=5 on ");
    system(" amixer -Dhw:0 cset numid=6 off ");
    system(" amixer -Dhw:0 cset numid=7 off ");
    system(" amixer -Dhw:0 cset numid=8 on ");
    system(" amixer -Dhw:0 cset numid=2 30% ");
    system(" amixer -Dhw:0 cset numid=3 95%");

    int tempcount = 0;
    while (true)
    {
        funcTemp ft;
        {
            std::unique_lock<std::mutex> lock(mtx);

            cv.wait(lock, [] { return pFuncs.size(); });
      
            ft = pFuncs.front();
            pFuncs.pop();
        }
       
        ft.pFunc(ft.p1,ft.p2);
      //  LOGD("runthread : %d",tempcount++);
    }

}

// 这个sd卡检测方式不好用，有时没了会显示还有，结果同 df -h ,
// 改用dvr 的 sd卡检测

void loop_detection(){


    while (true)
    {
        usleep(1000000); // 一秒检测一次  
     
        system("mountpoint /sdcard | grep  sdcard | grep no > /tmp/tmp.txt");

        struct stat statbuf;
        stat("/tmp/tmp.txt", &statbuf);

	    statbuf.st_size;
        if(statbuf.st_size == 0)
            instance->Notify(SysCtl::SYS_NT_SDCARD_ST,1," ");
        else 
            instance->Notify(SysCtl::SYS_NT_SDCARD_ST,0," ");

        system("mountpoint /udisk | grep  udisk | grep no > /tmp/tmp.txt");
        stat("/tmp/tmp.txt", &statbuf);

	    statbuf.st_size;
        if(statbuf.st_size == 0)
            instance->Notify(SysCtl::SYS_NT_UDISK_ST,1," ");
        else 
            instance->Notify(SysCtl::SYS_NT_UDISK_ST,0," ");

        
    }

}
SysCtl::SysCtl(){

    static std::thread t(runthread);

    t.detach();

    // static std::thread t1(loop_detection);
    // t1.detach();
}
SysCtl::~SysCtl()
{

}
int SysCtl::execute(int key,std::string const &data)
{
 //   LOGD( "SysCtl execute key:%d,data:%s,datalen:%d, data%d",key,data,datalen,atoi((const char *)data));
   
    return executeEx( key,data);
}


int SysCtl::executeEx(int key,std::string const &data,double p1 ,double p2, double p3 ,double p4 )
{
     int val = 0;
    int res = 0;
    
    switch (key)
    {
        case SET_BACKLIGHT:{
            std::unique_lock <std::mutex> lck(mtx);
            pFuncs.push(funcTemp{SetBacklight,atoi(data.c_str()),1});
            cv.notify_all();
            break;
        }
        case SET_SYS_TIME:
        {
            std::unique_lock <std::mutex> lck(mtx);
            pFuncs.push(funcTemp{setSysTime,atoi(data.c_str()),0});
            cv.notify_all();
            break;
        }
    default:
        break;
    }
}