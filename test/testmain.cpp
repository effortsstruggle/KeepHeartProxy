#include <stdio.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <stdio.h>
#include <unistd.h>
#include <getopt.h>
#include <errno.h>
#include <KeepHeartProxy.h>
#include <QDebug>
#include "CmdDef.h"


class TestKeepHeartProxy :public QObject
{
    Q_OBJECT
public:
    TestKeepHeartProxy()
    {
        this->m_s32PluginConfigId = Singleton_KeepHeartProxy::getInstance()->getPluginsID("PluginConfig");


        QObject::connect( Singleton_KeepHeartProxy::getInstance() ,  &KeepHeartProxy::pluginNotify , this, &TestKeepHeartProxy::onPluginNotify , Qt::DirectConnection);
        QObject::connect( Singleton_KeepHeartProxy::getInstance() ,  &KeepHeartProxy::pluginNotifyAsyn , this, &TestKeepHeartProxy::onPluginNotifyAsyn , Qt::DirectConnection);
        QObject::connect( this , &TestKeepHeartProxy::testsignal , 
            this, [=](int pid , int code , QString data){
            qDebug() << " -- pid -- : " << pid << " -- code -- : " << code  << " -- data -- : " << data ;
        });
    }

    ~TestKeepHeartProxy()
    {

    }


    void emitTestSignal()
    {
        emit testsignal(1,1,"hahaahahahhaha");
    }

public slots:
    void onPluginNotify(int pid, int code, QString data)
    {
  
    }

    void onPluginNotifyAsyn(int pid, int code, QString data)
    {
    
    }
    
signals:
    void testsignal(int pid,int code,QString data);

private:
    int m_s32PluginConfigId ; //PluginConfig id
    int m_s32PluginDBId ; //PluginDB id
};
using Singleton_TestKeepHeartProxy = Singleton<TestKeepHeartProxy>;


int main(int argc, char *argv[])
{
    Singleton_TestKeepHeartProxy::getInstance()->emitTestSignal();


    std::cout << "-------------------end---------------------------" << std::endl;
    return 0;
}

 #include "testmain.moc"