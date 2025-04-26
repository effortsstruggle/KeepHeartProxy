/**
 *@brief: 配置读取存储等操作
 *@author: sky
 *@version 
 *@param i2[in] 输入参数2
 *@param o3[out] 输出参数1
 *@since 
 *@date: 2024-09-12 10:18:43
*/


#pragma once 


#include "PluginInterface.h"
#include <thread>


class PluginCfg :public PluginInterface
{
public:

    enum{
        CFG_INIT,   // 打开
        CFG_CLOSE,  // 关闭
        CFG_SYNC,  // 手动同步的硬盘
        CFG_READ,   // 读取 对于整数等类型直接返回
        CFG_WRITE,   // 写入
        CFG_RESET,   // 删除文件重置
        CFG_READ_JSON, // 要求返回json 字符串，在回调函数中
        CFG_READ_JSON_ALL // 返回文件中所有数据
    };
    PluginCfg();
    virtual ~PluginCfg();
    int execute(int key,std::string const &data);
    int executeEx(int key,std::string const &data = "",double p1 = 0,double p2 = 0, double p3 = 0,double p4 = 0);

    std::string GetPluginName()
    {
      return "pluginCfg";
    }

  
  private:
  
    int init();
    /**
     *@brief: 从json 中读取数据
     *@param id[in] 传入要读取的字符的名字
     *@since 
     *@date: 2024-09-13 11:32:25
    */
    int read(const char* id);
    /**
     *@brief: 
     *@author: sky
     *@version 
     *@param str[in] 需要传入的字符串 json 格式

     *@since 
     *@date: 2024-09-13 11:35:25
    */
    int write(const char* str);

    /**
     *@brief: 删除配置文件重置
     *@author: sky
     *@since 
     *@date: 2024-12-11 09:58:56
    */

    int reset();

    /**
     *@brief: 写文件操作，同步到硬盘
     *@author: sky
     *@since 
     *@date: 2024-12-11 09:58:56
    */
    int sync();
    int close();
    int getJson(const char* str);
    int getAll();
    int fillEmptyRoot();
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
