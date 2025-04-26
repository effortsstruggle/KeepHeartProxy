
#include <thread>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
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

#include "lgnj_log.h"

std::mutex mtx;  // 创建一个互斥锁


std::condition_variable cv;

typedef int (*PFUNC)(int, int);

struct funcTemp
{
   PFUNC pFunc;
   int p1;
   int p2;
};



McuInterface * instance = new McuInterface ;
extern "C" PluginInterface* createPlugin() {

    return instance;
}


McuInterface::McuInterface(){

    if(ieai_mcu_open("/dev/ttyS4") == -1){
        LOGD("ieai_mcu_open : FAILED");
    }

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
    // LOGD( "McuInterface executeEx key:%d,stData:%s, data:%f , data2:%f , data3:%f , data4:%f",
    // key,
    // data,
    // p1 , p2 , p3 , p4 );

    int val = 0;
    int res = 0;
    if(data.empty())
    {

        if((int)p1 == 1)
        {
           res = ieai_mcu_get_int((MCU_INDEX)key);
        }
        else{
            ieai_mcu_set((MCU_COMMAND)key,p2,p3);
        }
    }
    else{
        _parser.SendCmd((MCU_COMMAND)key,data, p1, p2,p3,p4);
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
static void  OnMcuNotifhy(MCU_NOTIFY code, int data1, long data2)
{
    instance->Notify(code, "" , data1, data2 );
}

static void  OnMcuStrNotifhy(MCU_NOTIFY code, int data1, std::string & str)
{
    instance->Notify(code, str , data1 );
}

void McuInterface::ieai_mcu_set_notify()
{
	_parser.SetNotify(OnMcuNotifhy);

    _parser.SetNotify(OnMcuStrNotifhy);
}