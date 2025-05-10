#include "MplayerClient.h"
#include <thread>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "cJSON.h"
#include <vector>
#include <map>
#include "FHelper.h"
#include "mplayerApi.h"
#include "dvrApi.h"
#include "dvrFiles.h"
#include "dvrTypes.h"
#include <iostream>
#include "properties.h"

#include <unistd.h>
#include <mutex>

using namespace std;

static char szBuf512[521];
static char mcver[]="mplayerClient Version:0.0.002";

MplayerClient * instance = new MplayerClient;


static void mp_status_notify(MPLAYER_STATUS istatus)
{
     //instance->mcb(instance->m_id,MplayerClient::P_NT_CURPROGRESS, std:(istatus));
}

static void mp_position_notify(uint32_t iposition)
{
 //  mplayPlayStatus(1, iposition);
}
static void mp_duration_notify(uint32_t iduraiton)
{
  //  instance->mplayPlayStatus(2, iduraiton);
}


extern "C" PluginInterface* createPlugin() {

    return instance;
}

static MPLAYER_NOTIFY_CB mpCB;
MplayerClient::MplayerClient(){
   
    mpCB.pStatusCB = mp_status_notify;
    mpCB.pPositionCB = mp_position_notify;
    mpCB.pDurationCB = mp_duration_notify;
   
    ismediaOk = mplayer_create(&mpCB);
    if (ismediaOk == YPSMP_ERR)
    {   
        LOGD("mplayer_create failed ");
    }
  
    
}
MplayerClient::~MplayerClient()
{
    ismediaOk = -1;
    int res =  mplayer_destory();

     if (res == YPSMP_ERR)
    {   
        LOGD("mplayer_destory failed ");
    }
}
int MplayerClient::execute(int key,std::string const &data)
{
    LOGD( "MusicClient execute key:%d,data:%s",key,data.c_str());
   // std::cout << "key:" << key << " ,data:" << (char* )data  << std::endl;

    int val = 0;
    int res = 0;
    switch (key)
    {
        case P_SET_PAUSE : 
            res = mplayer_pause();
            break; 
        case P_SET_GOON :	   
            res =  mplayer_resume();
            break;
      
            break;
        case P_SET_STOP:         
            res =  mplayer_stop();
            break;
        case P_SET_SETPROGRESS: 
        
         break;

        case P_SET_SETVOLUME:  
        
          break;
 
        case P_JSON:
    
            PraseJsonCommand(data.c_str());
            break;
    default:
     LOGD("PraseJsonCommand111 \n");
        break;
    }
   
    return res;
}

int MplayerClient::PraseJsonCommand(const char* str)
{
    // var jsonstr = '{"filepath":"%1","x":130,"y":88,"w":1088,"h":352}'.arg(path.substring(7))
    
    cJSON* data = cJSON_Parse(str);
    
    cJSON* next = data->child;

    int res = 0;

    cJSON *item = NULL;
    char * path = nullptr;
    int x,y,w,h;
    int count = 0;
    item =  cJSON_GetObjectItem(data, "filepath");
    if(item)
    {
        path = item->valuestring;
        count++;
    }

    item =  cJSON_GetObjectItem(data, "x");
    if(item)
    {
        x = item->valueint;
        count++;
    }
    item =  cJSON_GetObjectItem(data, "y");
    if(item)
    {
        y = item->valueint;
        count++;
    }
    item =  cJSON_GetObjectItem(data, "w");
    if(item)
    {
        w = item->valueint;
        count++;
    }
    item =  cJSON_GetObjectItem(data, "h");
    if(item)
    {
        h = item->valueint;
        count++;
    }
  
    if(count != 5)
    {
        LOGD(" wrong param,right  is \"filpath,x,y,w,h\"%s",str);
        return -1;
    }

    LOGD(" mplayer_playmplayer_play(path,x,y,w,h):  %s,%d,%d,%d,%d",path,x,y,w,h);
    int ret = mplayer_play(path,x,y,w,h);
    if( ret == -1)
    {
        LOGD(" mplayer_play failed %s",str);
    }

    LOGD(" mplayer_play ret %d",ret);
}

