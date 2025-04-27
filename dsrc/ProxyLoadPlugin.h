#ifndef __PROXYLOADPLUGIN_H_
#define __PROXYLOADPLUGIN_H_

#include <functional>
#include <Singleton/singleton.h>
#include "PluginInterface.h"
#include <vector>
#include <map>

typedef PluginInterface* (*CreatePluginFunc)();

class ProxyLoadPlugin 
{

public:

   ProxyLoadPlugin();
   ~ProxyLoadPlugin();

   /**
   *@brief: load plugins
   */
   int LoadPlugnins( const std::string &path = "/home/qin/workspace/KeepHeartProxyDependsLib/plugins");
   int LoadPlugin(std::string const & path);

   /**
    *@brief: exec plugin
   */
   int executeFunction(int plugId,int key,std::string const & data  = "");
   int executeFunctionAsyn(int plugId,int key, std::string const & data  = "" );

   /**
    * @brief close plugin
    */
   int closePluginHandles();
   int closePluginHandle(int pid);

   /**
    * @brief add notify
    */
   void addMonitor(const CallBackFuncType & cb);
   void addMonitorAsync(const AsynCallBackFuncType & cb );

   /**
    * @brief get plugin id
    */
   int getPluginByName(std::string name);


private:
   std::vector<PluginInterface*> m_vecPlugins; // 创建出的插件实例
   std::vector<void*>  m_vecPluginHandles;     // 加载的插件库的句�?,卸载时使�?
   std::map<std::string,int>  m_mapNameToId;             // 插件名对应的ID
   int m_s32PluginCount;      
};
using Singleton_ProxyLoadPlugin = Singleton<ProxyLoadPlugin>;


 
#endif // __PROXYLOADPLUGIN_H_

