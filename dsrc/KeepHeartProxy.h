#ifndef __KEEPHEARTPROXY_H_
#define __KEEPHEARTPROXY_H_


#include <QObject>
#include <Singleton/singleton.h>

class KeepHeartProxy : public QObject
{
    Q_OBJECT

public:

    KeepHeartProxy() ;
    ~KeepHeartProxy() ;
 
    /**
     *@description: 依据插件的名字获取插件的ID，插件，执行;命令时需要使用ID
     *@author: sky
     *@param pname[in] 插件的名字
     *@return -1 插件名字错误
              其他值 ：插件的ID
     *@date: 2024-08-26 14:36:06
    */
     Q_INVOKABLE int getPluginsID(QString pname);

    /**
     *@description: 执行插件命令
    *@author: sky
    *@param dllid[in] 插件ID
    *@param key[in] 执行名命令的ID
    *@param data[out] 数据，所有数据都转换为字符类型，如果时多个参数时，使用JSON 的结构
    *@return 
    *@date: 2024-08-26 14:32:59
    */

    Q_INVOKABLE int pluginsExecute(int dllid,int key,QString data);

    Q_INVOKABLE int pluginsExecuteAsync(int dllid,int key,QString data );

    int onPluginNotify(int,int,std::string);
    int onPluginNotifyAsyn(int,int,std::string);

signals:

     void pluginsNotify(int pid,int code,QString data);
     void pluginsNotifyAsyn(int pid,int code,QString data);

private:

};

using Singleton_KeepHeartProxy = Singleton<KeepHeartProxy>;



#endif // __KEEPHEARTPROXY_H_
