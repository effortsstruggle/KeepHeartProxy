#include "SkyProxy.h"
#include <iostream>
#include "dvrApi.h"

#include <assert.h>
#include <fcntl.h>
#include <getopt.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sstream>
#include <time.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/file.h>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QString>
#include "log.h"                 //carbitUtil header
#include "lollipop_socket_ipc.h" //carbitUtil header
#include "properties.h"          //carbitUtil header
#include "dvrApi.h"
#include "dvrFiles.h"
#include "dvrTypes.h"
#include <QDebug>
#include <QDir>
#include <QStringList>
#include <QDebug>
#include <map>
#include "LoadPlugin.h"
#include <QTextCodec>
#include "bllLog.h"
#include <dirent.h>
#include <QThread>
using namespace std;
using namespace std::placeholders; 
static char szBuf512[512];
#define  LOGDD(...) __android_log_print(ANDROID_LOG_DEBUG, "SKY_PROXY", __VA_ARGS__);
 

 SkyProxy::SkyProxy()
 {
    LoadPlugnins();
   
   // QTextCodec::setCodecForLocale(QTextCodec::codecForName("utf-8"));
    addMonitor( CallBackFuncType (std::bind(&SkyProxy::onPluginNotify, this,_1,_2,_3,_4,_5,_6,_7))); 
    addMonitorEx( CallBackFuncIIIS (std::bind(&SkyProxy::onPluginNotifyEx, this,_1,_2,_3,_4)));
    

 }
SkyProxy::~SkyProxy()
{

}
static SkyProxy*  proxy_instance  = NULL;
SkyProxy * SkyProxy::getInstance()
{
    QTextCodec::setCodecForLocale(QTextCodec::codecForName("utf-8"));
    if(!proxy_instance)
        proxy_instance = new   SkyProxy;
    return  proxy_instance;
}

Q_INVOKABLE int SkyProxy::getPluginsID(QString pname)
{
    return getPluginByName(pname.toStdString());

}


Q_INVOKABLE int SkyProxy::pluginsExecute(int dllid,int key,QString  data)
{
   // float val = data.toFloat();

    return executeFunction(dllid, key,data.toStdString());
  
}

Q_INVOKABLE int SkyProxy::pluginsExecuteEx(int dllid,int key,QString  data ,double p1, double p2,double p3,double p4)
{
    return executeFunctionEx(dllid, key,data.toStdString(),p1,p2,p3,p4);
}


 int SkyProxy::onPluginNotify(int pid,int code,std::string data,double p1 ,double p2 ,double p3 ,double p4)
 {
 //   qDebug() << "SkyProxy::onPluginNotify : " <<  this  ;
 //   qDebug() << "QThread::currentThreadId() =  "<< QThread::currentThreadId();
    QString str = data.c_str();
    emit skyPluginsNotify(pid,code,str,p1 , p2 , p3 , p4);
   
    return 0;
 }

 int SkyProxy::onPluginNotifyEx(int pid,int code,int p1,std::string data)
 {
  //   qDebug() << "SkyProxy::onPluginNotifyEx : " <<pid<<  data.c_str();
    QString str = data.c_str();
    emit skyPluginsNotifyEx(pid,code,p1,str);
   // qDebug() << "SkyProxy::onPluginNotifyEx : " <<pid<<  str;
    return 0;
 }
