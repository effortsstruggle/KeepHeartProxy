#include <QDebug>
#include <KeepHeartProxy.h>
#include <Singleton/singleton.h>
#include <unistd.h>
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


        QString stData = "{\"data1\":\"pw\"}";
        Singleton_KeepHeartProxy::getInstance()->pluginsExecuteAsync( this->m_s32PluginConfigId , CFG_INIT , stData );
        Singleton_KeepHeartProxy::getInstance()->pluginsExecuteAsync( this->m_s32PluginConfigId , CFG_READ , stData );
        // Singleton_KeepHeartProxy::getInstance()->pluginsExecuteAsync( this->m_s32PluginConfigId , CFG_WRITE , stData );
        Singleton_KeepHeartProxy::getInstance()->pluginsExecuteAsync( this->m_s32PluginConfigId , CFG_READ_JSON , stData );
        Singleton_KeepHeartProxy::getInstance()->pluginsExecuteAsync( this->m_s32PluginConfigId , CFG_READ_JSON_ALL , stData );
        Singleton_KeepHeartProxy::getInstance()->pluginsExecuteAsync( this->m_s32PluginConfigId , CFG_RESET , stData );
        // Singleton_KeepHeartProxy::getInstance()->pluginsExecuteAsync( this->m_s32PluginConfigId , CFG_CLOSE , stData );

        
    }

    ~TestKeepHeartProxy()
    {
    }

public slots:
    void onPluginNotify(int pid, int code, QString data)
    {
        qDebug() << "onPluginNotify ::  code : " << code << " data : " << data  ;

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

        qDebug() << " onPluginNotifyAsyn :: code : " << code << " data : " << data  ;

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
    Singleton_TestKeepHeartProxy::getInstance() ; // ->emitTestSignal();

    usleep( 5 * 1000 );
    std::cout << "-------------------end---------------------------" << std::endl;
    return 0;
}

 #include "testmain.moc"