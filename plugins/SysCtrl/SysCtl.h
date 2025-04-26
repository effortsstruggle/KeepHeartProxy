/**
 *@brief: 系统控制相关操作，背光、声卡等，这些操作与rom相关，不同的项目操作方式可能不同，
 *        因此独立出来处理
 *@author: sky
 *@version 
 *@since 
 *@date: 2024-10-22 10:28:55
*/

#pragma once 


#include "PluginInterface.h"
#include <thread>


class SysCtl :public PluginInterface
{
public:
    enum SYSOP{
      SET_BACKLIGHT,  // 设置背光
      SET_SYS_TIME,   // 设置系统当前时间

      SYS_NT_SDCARD_ST,   // sdcard 状态 0，为插入，1 插入
      SYS_NT_UDISK_ST,    // u盘 状态  0，为插入，1 插入

    };

    SysCtl();
    virtual ~SysCtl();
    int execute(int key,std::string const &data);
    int executeEx(int key,std::string const &data = "",double p1 = 0,double p2 = 0, double p3 = 0,double p4 = 0);

    std::string GetPluginName()
    {
      return "sysCtl_GLB";
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
