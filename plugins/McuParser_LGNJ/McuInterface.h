/**
 *@brief: mcu 通信协议解析库，同时保持与上层qml的交互
 *@author: sky
 *@version 
 *@since 
 *@date: 2024-10-22 10:28:55
*/

#pragma once 


#include "PluginInterface.h"
#include "McuDef.h"
#include <thread>


class McuInterface :public PluginInterface
{
public:

    McuInterface();
    virtual ~McuInterface();
    int execute(int key,std::string const &data);
    /**
     *@brief: 
     *@author: sky
     *@version 
     *@param p1[in] 1 ：获取数据，0 设置数据
     *@param p2[out] 输出参数1
     *@since 
     *@date: 2025-03-07 11:00:30
    */
    int executeEx(int key,std::string const &data = "",double p1 = 0,double p2 = 0, double p3 = 0,double p4 = 0);
    
    int 	ieai_mcu_open(const char* port);
    int 	ieai_mcu_close();
    int   ieai_mcu_get_int(MCU_INDEX index);
    int   ieai_mcu_set(MCU_COMMAND cmd, int data1, long data2);
    void	ieai_mcu_set_notify();
    std::string GetPluginName()
    {
      return "mcuPraser_LGNJ";
    }

  
  private:


};

/**
 *@description: 实例化
 *@author: sky
 *@return 返回对象指针
 *@date: 2024-06-25 17:56:45
*/
extern "C" PluginInterface* createPlugin() ;
// 释放删插件，确保插件未在使用中
extern "C" void destroyPlugin(PluginInterface* plugin) {
    delete plugin;
}
