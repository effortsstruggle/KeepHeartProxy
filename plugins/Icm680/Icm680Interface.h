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


class Icm680Interface :public PluginInterface
{
public:
    enum  ICM680OP{
        ICM_INIT ,// 初始化 ,目前不需要调用

        ICM_GET_ANG, // 获取角度 ,调用然后回调函数里边通过通过枚举值接收
        ICM_START_CALI,   // 开始标定
        ICM_SET_INIT_ANG, // 设置陀螺仪的初始角度
        //---------------------------------------------
        ICM_NT_CALI ,   // 回调标定结束信号
        ICM_NT_ANG      // 回调当前角度
    };

    Icm680Interface();
    virtual ~Icm680Interface();
    int execute(int key,std::string const &data);
    int executeEx(int key,std::string const &data = "",double p1 = 0,double p2 = 0, double p3 = 0,double p4 = 0);

    std::string GetPluginName()
    {
      return "Icm680";
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
