#include "PluginRDAFM5808.h"
#include "rdafm_drv_V5.1.h"
#include <thread>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

PluginRDAFM5808::PluginRDAFM5808():seeking(0){
        RDAFM_init();
}
PluginRDAFM5808::~PluginRDAFM5808()
{

}
int PluginRDAFM5808::execute(int key,std::string const &data)
{

    int val = atoi(data.c_str());
    switch (key)
    {
        case FM_START:
            RDAFM_power_on();
        break;
        case FM_STOP:
            RDAFM_power_0ff();
        break;

        case FM_SEEK:
            startSeek();
        break;
        case FM_SEEK_STOP :
        break;
        case FM_SEEK_STEP_LEN:
        break;
        case FM_SLEEP:
        break;
        case FM_VOLUME:
            RDAFM_set_volume(val);
        break;
        case FM_RESET:
        break;
        case FM_SET_TUNE:
            // 设置频率
            RDAFM_set_freq(val);
        break;
 
    default:
        break;
    }
    std::cout << "key:" << key << "data:" << data.c_str()  << std::endl;
    return 1;
}

int PluginRDAFM5808::startSeek()
{
    std::thread t1(&PluginRDAFM5808::autoSeekThread,this);
    t1.join();
    std::swap(t1,mth);
    seeking = true;
    return 1;

}

void PluginRDAFM5808::stopSeek()
{
    seeking = false;

}


void  PluginRDAFM5808::autoSeekThread()
{
    unsigned short int curChan;
    unsigned short int tReg = 0;
    int i = 0;
    unsigned short minfreq = 8700;
    unsigned short maxfreq = 10800;
    char buf[32] = {0};

    for (i=minfreq;i<=maxfreq;i += 10 ){
        if(!seeking)
        break;
        if(RDAFM_valid_stop(i))
        {
            memset(buf,0,sizeof(buf));
            sprintf(buf, "%d", i/100);
                       // 记录
           
            Notify(FM_SEEK,buf);           
        }
    }
    Notify(FM_SEEK_OVER,nullptr);          
}
