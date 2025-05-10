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

        QString stData = "{\"data1\":\"pw\"}";
        Singleton_KeepHeartProxy::getInstance()->pluginsExecuteAsync( this->m_s32PluginConfigId , CFG_INIT , stData );
        Singleton_KeepHeartProxy::getInstance()->pluginsExecuteAsync( this->m_s32PluginConfigId , CFG_READ , stData );
        // Singleton_KeepHeartProxy::getInstance()->pluginsExecuteAsync( this->m_s32PluginConfigId , CFG_WRITE , stData );
        // Singleton_KeepHeartProxy::getInstance()->pluginsExecuteAsync( this->m_s32PluginConfigId , CFG_READ_JSON , stData );
        // Singleton_KeepHeartProxy::getInstance()->pluginsExecuteAsync( this->m_s32PluginConfigId , CFG_READ_JSON_ALL , stData );
        // Singleton_KeepHeartProxy::getInstance()->pluginsExecuteAsync( this->m_s32PluginConfigId , CFG_CLOSE , stData );
        // Singleton_KeepHeartProxy::getInstance()->pluginsExecuteAsync( this->m_s32PluginConfigId , CFG_RESET , stData );
        
    }

    ~TestKeepHeartProxy()
    {
        std::cout << "~TestKeepHeartProxy(1)" << std::endl;
    }


    void emitTestSignal()
    {
        emit testsignal(1,1,"hahaahahahhaha");
    }

public slots:
    void onPluginNotify(int pid, int code, QString data)
    {
        qDebug() << " code : " << code << " data : " << data  ;

        if( pid == this->m_s32PluginConfigId )
        {
            switch ( code )
            {

            case CFG_INIT:
            break;
            
            case CFG_CLOSE:
            break;
            
            case CFG_READ:
            break;

            case CFG_WRITE:
            break;

            case CFG_RESET:
            break;

            case CFG_READ_JSON:
            break;

            case CFG_READ_JSON_ALL:
            break;

            default:
            break;

            }
        }

    }

    void onPluginNotifyAsyn(int pid, int code, QString data)
    {

        qDebug() << " code : " << code << " data : " << data  ;

        if( pid == this->m_s32PluginConfigId )
        {
            switch ( code )
            {

            case CFG_INIT:
            break;
            
            case CFG_CLOSE:
            break;
            
            case CFG_READ:
            break;

            case CFG_WRITE:
            break;

            case CFG_RESET:
            break;

            case CFG_READ_JSON:
            break;

            case CFG_READ_JSON_ALL:
            break;

            default:
            break;

            }
        }

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

    usleep( 5 * 1000 );
    std::cout << "-------------------end---------------------------" << std::endl;
    return 0;
}

 #include "testmain.moc"