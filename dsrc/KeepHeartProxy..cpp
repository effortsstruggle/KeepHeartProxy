#include "KeepHeartProxy.h"
#include <iostream>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QString>
#include <QDebug>
#include <QDir>
#include <QStringList>
#include <QTextCodec>
#include <map>
#include <Log/Log.h>
#include "ProxyLoadPlugin.h"

using namespace std::placeholders; 


KeepHeartProxy::KeepHeartProxy()
 {
    LoadPlugnins();
   
   // QTextCodec::setCodecForLocale(QTextCodec::codecForName("utf-8"));
    addMonitor( CallBackFuncType (std::bind(&KeepHeartProxy::onPluginNotify, this,_1,_2,_3,_4,_5,_6,_7))); 
    addMonitorEx( CallBackFuncIIIS (std::bind(&KeepHeartProxy::onPluginNotifyEx, this,_1,_2,_3,_4)));
    

 }
 
 KeepHeartProxy::~KeepHeartProxy()
{

}

Q_INVOKABLE int KeepHeartProxy::getPluginsID(QString pname)
{
    return getPluginByName(pname.toStdString());
}


Q_INVOKABLE int KeepHeartProxy::pluginsExecute(int dllid,int key,QString  data)
{
    return executeFunction(dllid, key,data.toStdString());
}

Q_INVOKABLE int KeepHeartProxy::pluginsExecuteEx(int dllid,int key,QString  data ,double p1, double p2,double p3,double p4)
{
    return executeFunctionEx(dllid, key,data.toStdString(),p1,p2,p3,p4);
}


 int KeepHeartProxy::onPluginNotify(int pid,int code,std::string data,double p1 ,double p2 ,double p3 ,double p4)
 {
    QString str = data.c_str();
    emit pluginsNotify(pid,code,str,p1 , p2 , p3 , p4);
    return 0;
 }

 int KeepHeartProxy::onPluginNotifyEx(int pid,int code,int p1,std::string data)
 {
    QString str = data.c_str();
    emit pluginsNotifyEx(pid,code,p1,str);
    return 0;
 }
