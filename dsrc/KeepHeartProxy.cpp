#include "KeepHeartProxy.h"
#include <iostream>
#include <map>
#include "ProxyLoadPlugin.h"
#include "KeepHeartProxyLog.h"

using namespace std::placeholders; 


KeepHeartProxy::KeepHeartProxy()
 {
    Singleton_ProxyLoadPlugin::getInstance()->LoadPlugnins();

    Singleton_ProxyLoadPlugin::getInstance()->addMonitor( CallBackFuncType ( std::bind(&KeepHeartProxy::onPluginNotify, this,_1,_2,_3) )); 
    Singleton_ProxyLoadPlugin::getInstance()->addMonitor( CallBackFuncType ( std::bind(&KeepHeartProxy::onPluginNotify, this,_1,_2,_3) )); 
 }
 
KeepHeartProxy::~KeepHeartProxy()
{

    std::cout << "KeepHeartProxy::~KeepHeartProxy(1)" << std::endl;

    Singleton_ProxyLoadPlugin::getInstance()->closePluginHandles();

    std::cout << "KeepHeartProxy::~KeepHeartProxy(2)" << std::endl;
}

Q_INVOKABLE int KeepHeartProxy::getPluginsID(QString pname)
{
    return Singleton_ProxyLoadPlugin::getInstance()->getPluginByName( pname.toStdString() );
}


Q_INVOKABLE int KeepHeartProxy::pluginsExecute(int dllid , int key , QString data)
{
    return Singleton_ProxyLoadPlugin::getInstance()->executeFunction(dllid, key,data.toStdString());
}

Q_INVOKABLE int KeepHeartProxy::pluginsExecuteAsync(int dllid , int key , QString data )
{
    return Singleton_ProxyLoadPlugin::getInstance()->executeFunctionAsyn(dllid , key , data.toStdString() );
}


 int KeepHeartProxy::onPluginNotify(int pid , int code , std::string data)
 {
    QString str = data.c_str();
    emit pluginNotify(pid , code , str );
    return 0;
 }

 int KeepHeartProxy::onPluginNotifyAsyn(int pid , int code , std::string data)
 {
    QString str = data.c_str();
    emit pluginNotifyAsyn(pid , code , str);
    return 0;
 }
