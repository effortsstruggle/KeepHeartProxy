#include "MusicClient.h"
#include <thread>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "cJSON.h"
#include <vector>
#include <map>
#include "FHelper.h"
#include "SkyIPCClient.h"
#include "SocketHelper.h"
#include "SkyConfig.h"
#include "functor.h"
#include "ID3v2.h"
#include <android/log.h>
#include <unistd.h>
using namespace std;


static char szBuf512[521];
static char mcver[]="mClientVer:0.0.001";
MusicClient * instance = new MusicClient;
#define  LOGD(...) __android_log_print(ANDROID_LOG_DEBUG, "SKY_MUSIC", __VA_ARGS__);
/*
TIT2 ：歌曲标题名字
TPE1 ：作者名字
TALB ：作品专辑
TYER ：作品产生年代
COMM ：备注信息
APIC ：专辑图片*/
std::map<std::string,std::string> filtermap = {
    {"TIT2","title"},
    {"TPE1","artist"},
    {"TALB","album"}};



extern "C" PluginInterface* createPlugin() {

    return instance;
}

MusicClient::MusicClient(){
  
    LOGD("mcver %d",mcver);
   mediaIpc = new SkyIPCClient(functor(*this,&MusicClient::onMediaMsg));
    if(mediaIpc->StartClient()== 0)
    {
        sleep(1);
        ismediaOk = true;
    }
}
MusicClient::~MusicClient()
{
    if(mediaIpc)
        delete mediaIpc;
}
int MusicClient::execute(int key,std::string const &data)
{
    return executeEx(key,data);
}
 int MusicClient::executeEx(int key,std::string const &data,double p1,double p2, double ,double p4 ) {

//    LOGD( "MusicClient execute key:%d,data:%s,datalen:%d",key,data,datalen);
 //   std::cout << "key:" << key << " ,data:" << (char* )data  << std::endl;

     int val  = 0;
    int res = 0;
    
    switch (key)
    {
       
        case P_SET_PAUSE : 
            res = mediaPause(false);
            break; 
        case P_SET_GOON :	   
            res =  mediaPause(true);
            break;
        case P_SET_URL:	  
            res = mediaPlay(data);
            break;
        case P_SET_STOP:         
            res =  mediaStop(0 );
            break;
        case P_SET_PLAY_SPEED: 
            val = atoi(data.c_str());
            res = mediaPlaySpeed(val);
          break;

        case P_SET_SETPROGRESS: 
        
         break;
    
        case P_SET_SETVOLUME:  
        
          break;
     
        case P_NO_MUSIC:   
          break;
        case  P_NT_CURPROGRESS:
            mediaGetCurPos();
            break;
        case P_NT_MUSICLISTS:  
        {
            getMusicFiles(data);
            break;
        }
    default:
        break;
    }
   
    return res;
 }
 int MusicClient::getMusicFiles(string path) {

    LOGD( "getMusicFiles: %s",path.c_str());
    if(path.empty())
        path = "/udisk"; // 替换为你的目标文件夹路径

    pathPre = path;

    LOGD( "getMusicFiles: %s",path.c_str());
    std::vector<std::string> files;

    FHelper::getFiles(path,files, "mp3");
  
    cJSON 	*jsonroot = NULL;
	//新建一个JSON项目：jsonroot
    jsonroot = cJSON_CreateObject(); 
    if(NULL == jsonroot)
    {
        LOGD( "cJSON_CreateObject failed ");
        return -1;
    }
	
    cJSON* jsonArr = cJSON_CreateArray();

    for (int i = 0; jsonArr && (i < files.size()); i++)
    {
        cJSON_AddItemToArray(jsonArr, cJSON_CreateString(files[i].c_str()));
    }

	//ArrStr加入到jsonroot
	cJSON_AddItemToObject(jsonroot, "filesArr", jsonArr);


    char* pStr  = cJSON_PrintUnformatted(jsonroot);

    
    Notify(P_NT_MUSICLISTS,pStr);
    delete pStr;
    return 0;
}
 

void MusicClient::onMediaMsg(char* str)
{
   // qDebug() << "SkyDvrProxy::onMediaMsg" ;
    MsgHeader * msg = (MsgHeader*)str;

    if(msg->dataLen < 0)
        return ;
         cJSON 	*jsonroot = NULL;
	//新建一个JSON项目：jsonroot
    jsonroot = cJSON_CreateObject(); 
    if(NULL == jsonroot)
    {
        std::cout << "cJSON_CreateObject failed :" << std::endl;
        return ;
    }
	
    switch(msg->msgType)
    {
        case P_NT_MUSICINFO:
        {
            const int * ptrdata  = (int*) msg->data;     
            cJSON_AddItemToObject(jsonroot, "duration",  cJSON_CreateNumber(ptrdata[0]));         
            cJSON_AddItemToObject(jsonroot, "curPos",  cJSON_CreateNumber(ptrdata[1]));        
            cJSON_AddItemToObject(jsonroot, "state",  cJSON_CreateNumber(ptrdata[2]));
            cJSON_AddItemToObject(jsonroot, "artist",  cJSON_CreateString("unknow"));
            cJSON_AddItemToObject(jsonroot, "album",  cJSON_CreateString("unknow"));
            
             std::cout << "ccururl :" <<  cururl <<std::endl;
            std::unique_ptr<ID3v2> id2 = std::make_unique<ID3v2>(cururl);

            if(id2->processTags()){
                for (auto iter = filtermap.begin(); iter != filtermap.end(); ++iter) {
                    auto key = iter->first;
                    auto val = iter->second; 
                    id2->get(key);
                    // qDebug() << "id2->get(key) :" <<  sa.c_str();
                    if(id2->strdata.size())
                        cJSON_AddItemToObject(jsonroot, val.c_str(),  cJSON_CreateString(id2->strdata.c_str()));
                  
                }
            }
            else
            {
                std::cout << "ID3v2 processTags error" << std::endl;
            }
  
            break;
        }
        case P_NT_CURPROGRESS:
        {
            const int * ptrdata  = (int*) msg->data;
           // std::cout << "P_NT_CURPROGRESS : " <<  *ptrdata <<  std::endl;
            cJSON_AddItemToObject(jsonroot, "curPos",  cJSON_CreateNumber(ptrdata[0]));
            break;
        }
        case P_NT_PLAYSTAE:
        {
            const int * ptrdata  = (int*) msg->data;

            cJSON_AddItemToObject(jsonroot, "state",  cJSON_CreateNumber(ptrdata[0]));
            break;
        }
        default:
        {
            std::cout << "unknow msgtype" << msg->msgType << __func__<< __FILE__<< __LINE__ <<  std::endl;
             return;
        }
    }
    
    char* pStr  = cJSON_PrintUnformatted(jsonroot);

    Notify(msg->msgType,pStr);
    delete pStr;
}


std::map<std::string,std::string>musicFilesMap;

int MusicClient::mediaPlay(const std::string &url)
{
    std::cout  << "MusicClienty::mediaPlayUrl:  " << url << std::endl;
    cururl = url;
   

    cururl = pathPre + "/" + cururl;
    std::cout  << "MusicClienty::mediaPlayUrl:  " << cururl << std::endl;
    skyMediaOperator(P_SET_URL,cururl.size(),cururl.c_str());
return 0;
}

int MusicClient::mediaPause(bool pa)
{
   // qDebug() << "MediaPause:" << pa;
    skyMediaOperator(pa?P_SET_GOON:P_SET_PAUSE,0,nullptr);
}

int MusicClient::mediaStop(bool val )
{
     skyMediaOperator(P_SET_STOP,val,nullptr);
}

int MusicClient::mediaVolume(int val)
{
    skyMediaOperator(P_SET_SETVOLUME,val,nullptr);
    return 0;
}


int MusicClient::mediaPlaySpeed(int val)
{
    std::cout << "MediaPlaySpeed1:" << val << std::endl;
    skyMediaOperator(P_SET_PLAY_SPEED,val,nullptr);
    return 0;
}



int MusicClient::mediaPlayProgress(int val)
{
    //qDebug() << "MediaPlayProgress:" << val;
    skyMediaOperator(P_SET_SETPROGRESS ,val,nullptr);
}

int MusicClient::mediaGetCurPos(){
     skyMediaOperator(P_NT_CURPROGRESS,0,nullptr);
}

int MusicClient::skyMediaOperator(int p1,int p2,const char*  p3)
{
    if(!ismediaOk)
        return -1;
    memset(szBuf512,0,sizeof(szBuf512));
   
    MsgHeader msg;
    msg.msgType = p1;
    msg.addtional = p2;

    2 == p1?msg.dataLen = p2:msg.dataLen = 0;
    
    memcpy(szBuf512,&msg,sizeof(MsgHeader));
    memcpy(&(szBuf512[sizeof(MsgHeader)]),p3,msg.dataLen);

   // msg.msgPrint("1");
    if(mediaIpc->SendData(szBuf512,sizeof(MsgHeader) + msg.dataLen) <= 0)
    {
        mediaIpc->CloseClient();
        TRACE("send failed.\n");
    }

}