
#include <thread>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <android/log.h>
#include <mutex>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/time.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <linux/ioctl.h>
#include <condition_variable>

#include "McuSerial.h"
#include "McuParser.h"
#include "McuInterface.h"

#include <memory.h>

#include <queue>
static McuSerial _serial;
static McuParser _parser;


#define  LOGD(...) __android_log_print(ANDROID_LOG_DEBUG, "SKY_MCU", __VA_ARGS__);

std::mutex mtx;  // 创建一个互斥锁


std::condition_variable cv;

typedef int (*PFUNC)(int, int);

struct funcTemp
{
   PFUNC pFunc;
   int p1;
   int p2;
};


// 简单的线程函数执下;有新的需要执行的函数就添加到队列，队列中负责执行;
static std::queue<funcTemp>  pFuncs ; //函数队列



McuInterface * instance = new McuInterface ;
extern "C" PluginInterface* createPlugin() {

    return instance;
}

void runthread()
{
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
        LOGD("runthread : %d",tempcount++);
    }
}
McuInterface::McuInterface(){

    static std::thread t(runthread);
    if(ieai_mcu_open("/dev/ttyS4") == -1){
        LOGD("ieai_mcu_open : FAILED");
    }
    t.detach();

    ieai_mcu_set_notify();
}
McuInterface::~McuInterface()
{

}
int McuInterface::execute(int key,std::string const &data){
     LOGD( "McuInterface execute ,interface deprecated");
}
int McuInterface::executeEx(int key,std::string const &data,double p1 ,double p2, double p3 ,double p4 )
{
 //  LOGD( "McuInterface executeEx key:%d,data:%s,datalen:%d, data%d",key,data,datalen,atoi((const char *)data));

    int val = 0;
    int res = 0;
    if(key == 999)
    {
        ieai_mcu_close();
    }
    if(key < 100){
        res = ieai_mcu_get_int((MCU_INDEX)key);
    }
    else{
        ieai_mcu_set((MCU_COMMAND)key,p1,p2);
    }

 
    return res;
}



int McuInterface::ieai_mcu_open(const char* port)
{
	if(!_serial.Open(port, 115200, 8, 'N', 1))
		return -1;

	_serial.SetParser(&_parser);
	_serial.StartListen();

	return 0;
}

int McuInterface::ieai_mcu_close()
{
	_serial.Close();

	return 0;
}

int McuInterface::ieai_mcu_get_int(MCU_INDEX index)
{
	return _parser.GetInt(index);
}

int McuInterface::ieai_mcu_set(MCU_COMMAND cmd, int data1, long data2)
{
	return _parser.SendCmd(cmd, data1, data2);
}

//������SkyProxyע��Ļص��ӿ�
static void  OnMcuNotifhy(MCU_NOTIFY code, int data1, long data2)
{
    instance->Notify(code,data1,std::to_string(data2));
}

static void  OnMcuStrNotifhy(MCU_NOTIFY code, int data1, std::string & str)
{
    instance->Notify(code,data1,str);
}

void McuInterface::ieai_mcu_set_notify()
{
	_parser.SetNotify(OnMcuNotifhy);

    _parser.SetNotify(OnMcuStrNotifhy);
}