#ifndef __SKYPROXY_H_
#define __SKYPROXY_H_


#include <QObject>
#include <QRect>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <iostream>


class SkyProxy : public QObject
{
    Q_OBJECT
public:
 

    static SkyProxy *getInstance();
    ~SkyProxy();
public:
 

    /**
     *@description: 依据插件的名字获取插件的ID，插件，执行;命令时需要使用ID
     *@author: sky
     *@param pname[in] 插件的名字
     *@return 
            -1 插件名字错误
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

    Q_INVOKABLE int pluginsExecuteEx(int dllid,int key,QString data ,double p1 = 0,double p2= 0,double p3= 0,double p4= 0);
   


    int onPluginNotify(int,int,std::string,double p1 = 0,double p2 = 0,double p3 = 0,double p4 = 0);
    int onPluginNotifyEx(int,int,int,std::string);
signals:
    // 信号：可以直接在QML中访问信号e

    void messageNotify(int notify, QString msg);

     void skyPluginsNotify(int pid,int code,QString data,double p1 = 0,double p2 = 0,double p3 = 0,double p4 = 0);
     void skyPluginsNotifyEx(int pid,int code,int p1,QString data);
public slots:
    // 槽函数：可以直接在QML中访问public槽函数

 //   void cppSlotB(const QString &str, int value); // 一个带参数槽函数

private:

    SkyProxy();
    // 禁止外部拷贝构造
    SkyProxy(const SkyProxy &signal): QObject(nullptr){Q_UNUSED(signal)}
    // 禁止外部赋值操作
    const SkyProxy &operator=(const SkyProxy &signal){Q_UNUSED(signal) return *this;};
};



#endif // DEBUG
