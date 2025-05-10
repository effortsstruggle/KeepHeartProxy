/**
 *@brief: 配置读取存储等操作
*/


#pragma once 


#include "PluginInterface.h"
#include <Singleton/singleton.h>
#include <cJSON.h>
#include <Async/AsyncCall.h>

class PluginConfig :public PluginInterface
{
public:

    enum ePluginConfig{
        CFG_INIT,   // 打开
        CFG_CLOSE,  // 关闭
        CFG_READ,   // 读取 对于整数等类型直接返回
        CFG_WRITE,   // 写入
        CFG_RESET,   // 删除文件重置
        CFG_READ_JSON, // 要求返回json 字符串，在回调函数中
        CFG_READ_JSON_ALL // 返回文件中所有数据
    };

    PluginConfig();
    virtual ~PluginConfig();
    /**
     * key : enum 
     * data : json 
     */
    virtual int execute(int key,std::string const &data = "" ) override;
    virtual int executeAsync(int key, std::string const &data = "" ) override ;

    virtual std::string GetPluginName() override 
    {
      return "PluginConfig";
    }

  
protected:

  static int asyncInit(FuncAndParam const &param );
  NotifyParam init();

  /**
   *@brief: 从json 中读取数据
  */
  static int asyncRead(FuncAndParam const &param );
  NotifyParam read(const char* id);

  /**
   *@brief: 
  */
  static int asyncWrite(FuncAndParam const &param );
  NotifyParam write(const char* str);

  /**
   *@brief: 删除配置文件重置
  */
  static int asyncReset(FuncAndParam const &param );
  NotifyParam reset();



  static int asyncClose(FuncAndParam const &param );
  NotifyParam close();


  static int asyncGetJson(FuncAndParam const &param );
  NotifyParam getJson(const char* str);

  static int asyncGetAll(FuncAndParam const &param );
  NotifyParam getAll();

private:
  /**
   *@brief: 写文件操作，同步到硬盘
  */
  NotifyParam sync();


  NotifyParam fillEmptyRoot();

  /**
   * @brief append json
   */
  std::string makeNotifyJson( NotifyParam &objNotifyParam);


private:
    cJSON *m_pRootJson;
    AsyncCall *m_pAsyncCall ;
    std::string m_stCfgPath;
};
using Singleton_PluginConfig = Singleton<PluginConfig>;

/**
 *@description: 实例化
 *@author: sky
 *@return 返回对象指针
 *@date: 2024-06-25 17:56:45
*/
extern "C" PluginInterface* createPlugin() ;
// 释放删插件，确保插件未在使用中
extern "C" void destroyPlugin(PluginInterface* plugin) {
  
}
