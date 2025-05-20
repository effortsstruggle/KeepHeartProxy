#include <QDebug>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <KeepHeartProxy.h>
#include <Singleton/singleton.h>
#include <Sonic/sonic/sonic.h>
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

        
        QString stReadData = R"({
            "data1": "pw"
        })";

        QString stReadData_2 = R"({
            "data1": "abc"
        })";


        QString stReadData_3 = R"({
            "data1": "kkk"
        })";

        QString stWriteData = R"({
            "data1": { "key":"pw", "value":"wangqin" }
        })";

        QString stWriteData_2 = R"({
            "data1": { "key":"abc", "value": { "test" : "hahahaha" }}
        })";


        QString stWriteData_3 = R"({
            "data1": { "key":"kkk", "value": [1,2,3] } 
        })";

        Singleton_KeepHeartProxy::getInstance()->pluginsExecuteAsync( this->m_s32PluginConfigId , CFG_INIT , "" );
 
        Singleton_KeepHeartProxy::getInstance()->pluginsExecuteAsync( this->m_s32PluginConfigId , CFG_WRITE , stWriteData );
        Singleton_KeepHeartProxy::getInstance()->pluginsExecuteAsync( this->m_s32PluginConfigId , CFG_WRITE , stWriteData_2 );
        Singleton_KeepHeartProxy::getInstance()->pluginsExecuteAsync( this->m_s32PluginConfigId , CFG_WRITE , stWriteData_3 );
        Singleton_KeepHeartProxy::getInstance()->pluginsExecuteAsync( this->m_s32PluginConfigId , CFG_READ , stReadData );
        Singleton_KeepHeartProxy::getInstance()->pluginsExecuteAsync( this->m_s32PluginConfigId , CFG_READ , stReadData_2 );
        Singleton_KeepHeartProxy::getInstance()->pluginsExecuteAsync( this->m_s32PluginConfigId , CFG_READ , stReadData_3 );
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
            {


                // 将字符串解析为 QJsonDocument
                QJsonDocument doc = QJsonDocument::fromJson(data.toUtf8());

                if (!doc.isNull() && doc.isObject()) {
                    QJsonObject jsonObj = doc.object();
                    
                    // 访问简单值
                    int s32Ret = jsonObj["result"].toInt();

                    if( 0 == s32Ret ){
                        QString successJson = jsonObj["sucess_notify"].toString();

                        QJsonDocument retDoc = QJsonDocument::fromJson(successJson.toUtf8());
                         if (!retDoc.isNull() && retDoc.isObject()) {
                            QJsonObject retJsonObj = retDoc.object();
                            int type = retJsonObj["type"].toInt();
                            if ( 1 == type  ) {
                                qDebug() << "successJson.isString()" ;
                                qDebug() << retJsonObj["value"].toString();
                            } else if ( 2 == type  )  {
                                qDebug() << "successJson.isInt()" ;
                                qDebug() << retJsonObj["value"].toDouble();
                            } else if ( 3 == type  )  {
                                qDebug() << "successJson.isDouble()" ;
                                qDebug() << retJsonObj["value"].toDouble();
                            } else if ( 4 == type  )  {
                                qDebug() << "successJson.isObject()" ;
                                qDebug() << retJsonObj["value"].toObject();
                            } else if ( 5 == type  )  {
                                qDebug() << "successJson.isArray()" ; 
                                QJsonArray oArrays = retJsonObj["value"].toArray();
                                qDebug() << "oArrays:";
                                for (const QJsonValue &oArray : oArrays) {
                                    qDebug() << "- " << oArray.toDouble();
                                }
                            }
                         }
                    } else {
                        int errorCode = jsonObj["error_notify"].toInt();
                    }

                } else {
                    qDebug() << "Invalid JSON";
                }


            }
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