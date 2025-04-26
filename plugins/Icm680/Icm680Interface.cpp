#include "Icm680Interface.h"
#include <thread>
#include <stdlib.h>
#include <string.h>
#include "cJSON.h"

#include <mutex>

#include <condition_variable>
#include <queue>
#include "log.h"
#include "icm42680.h"
static std::mutex icm_mtx;  // 创建一个互斥锁


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

static Icm680Interface * instance = new Icm680Interface ;
extern "C" PluginInterface* createPlugin() {

    return instance;
}

// 获取相对，倾斜角度
static int GetCurAngle(int p1,int p2){

    Point3f ang = getCurAngle();
    cJSON * root = cJSON_CreateObject();

    cJSON_AddNumberToObject(root,"x",ang.x);
    cJSON_AddNumberToObject(root,"y",ang.y);
    cJSON_AddNumberToObject(root,"z",ang.z);

    char * ptr = cJSON_PrintUnformatted(root);

    instance->Notify(p1,ptr);
    cJSON_free(ptr) ;
    cJSON_Delete(root);
    return 0;
}
static int AttitudeCorrection(int p1,int p2){
    attitude_correction(100,0);
    

    Point3f ang =  getInitAngle();

    cJSON * root = cJSON_CreateObject();
    cJSON_AddNumberToObject(root,"x",ang.x);
    cJSON_AddNumberToObject(root,"y",ang.y);
   // cJSON_AddNumberToObject(root,"z",ang.z);

    
    LOGD("AttitudeCorrection %d ",p1);
    char * ptr = cJSON_PrintUnformatted(root);
   
    instance->Notify(p1,ptr);

    cJSON_free(ptr) ;
    cJSON_Delete(root);
    return 0;
}


static void runthread()
{

    int res = open_dev();
    if(res < 0 )
    {
         LOGD("Icm680 open i2c bus error");
         return ;
    }

    while (true)
    {
        funcTemp ft;
        {
            std::unique_lock<std::mutex> lock(icm_mtx);

            cv.wait(lock, [] { return pFuncs.size(); });
      
            ft = pFuncs.front();
            pFuncs.pop();
        }
       
        ft.pFunc(ft.p1,ft.p2);
      //  LOGD("runthread : %d",tempcount++);
    }
    

}
Icm680Interface::Icm680Interface(){

    static std::thread t(runthread);
   
    t.detach();
}
Icm680Interface::~Icm680Interface()
{

}
int Icm680Interface::execute(int key,std::string const &data)
{
   // LOGD( "Icm680Interface execute key:%d,data:%s,datalen:%d, data%d",key,data,datalen,atoi((const char *)data));
    return executeEx(key,data.c_str());
}


int Icm680Interface::executeEx(int key,std::string const &data,double p1 ,double p2, double p3 ,double p4 )
{
    if (get_satus())
    {
        return -1;
    }
  //  LOGD( "Icm680Interface::executeEx key:%d ",key);
    switch (key)
    {
        case ICM_START_CALI:{
            std::unique_lock <std::mutex> lck(icm_mtx);
            pFuncs.push(funcTemp{AttitudeCorrection,ICM_NT_CALI,1});
            cv.notify_all();
            break;
        }
    
        case ICM_GET_ANG:{
            std::unique_lock <std::mutex> lck(icm_mtx);
            pFuncs.push(funcTemp{GetCurAngle,ICM_NT_ANG,1});
            cv.notify_all();
            break;
        }

    default:
        break;
    }

    return 0;
}

