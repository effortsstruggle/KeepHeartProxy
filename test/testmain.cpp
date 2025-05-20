#include <QDebug>
#include <KeepHeartProxy.h>
#include <Singleton/singleton.h>
#include <unistd.h>
#include "CmdDef.h"
#include <Sonic/sonic/sonic.h>


class TestKeepHeartProxy :public QObject
{
    Q_OBJECT
public:
    TestKeepHeartProxy()
    {
        this->m_s32PluginConfigId = Singleton_KeepHeartProxy::getInstance()->getPluginsID("PluginConfig");

        QObject::connect( Singleton_KeepHeartProxy::getInstance() ,  &KeepHeartProxy::pluginNotify , this, &TestKeepHeartProxy::onPluginNotify , Qt::DirectConnection);
        QObject::connect( Singleton_KeepHeartProxy::getInstance() ,  &KeepHeartProxy::pluginNotifyAsyn , this, &TestKeepHeartProxy::onPluginNotifyAsyn , Qt::DirectConnection);

        
        QString stReadData = R"({
            "data1": "pw"
        })";
        QString stWriteData = R"({
            "data1": { "key":"pw", "value":"wangqin" }
        })";

        QString stWriteObjData = R"({
            "data1": { "key":"abc", "value": { "test" : "hahahaha" }}
        })";

        QString stWriteObjData_2 = R"({
            "data1": { "key":"abc_2", "value": { "test_2" : "haha_2" }}
        })";

        QString stWriteArrData = R"({
            "data1": { "key":"kkk", "value": [1,2,3] } 
        })";

        Singleton_KeepHeartProxy::getInstance()->pluginsExecuteAsync( this->m_s32PluginConfigId , CFG_INIT , "" );
        Singleton_KeepHeartProxy::getInstance()->pluginsExecuteAsync( this->m_s32PluginConfigId , CFG_READ , stReadData );
        Singleton_KeepHeartProxy::getInstance()->pluginsExecuteAsync( this->m_s32PluginConfigId , CFG_WRITE , stWriteData );
        Singleton_KeepHeartProxy::getInstance()->pluginsExecuteAsync( this->m_s32PluginConfigId , CFG_WRITE , stWriteObjData );
        Singleton_KeepHeartProxy::getInstance()->pluginsExecuteAsync( this->m_s32PluginConfigId , CFG_WRITE , stWriteObjData_2 );
        Singleton_KeepHeartProxy::getInstance()->pluginsExecuteAsync( this->m_s32PluginConfigId , CFG_WRITE , stWriteArrData );
        // Singleton_KeepHeartProxy::getInstance()->pluginsExecuteAsync( this->m_s32PluginConfigId , CFG_READ_JSON , stData );
        Singleton_KeepHeartProxy::getInstance()->pluginsExecuteAsync( this->m_s32PluginConfigId , CFG_READ_JSON_ALL , "" );
        // // Singleton_KeepHeartProxy::getInstance()->pluginsExecuteAsync( this->m_s32PluginConfigId , CFG_RESET , stData );
        // // Singleton_KeepHeartProxy::getInstance()->pluginsExecuteAsync( this->m_s32PluginConfigId , CFG_CLOSE , stData );

        
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

    usleep( 100 * 1000 );
    std::cout << "-------------------end---------------------------" << std::endl;
    return 0;
}

 #include "testmain.moc"