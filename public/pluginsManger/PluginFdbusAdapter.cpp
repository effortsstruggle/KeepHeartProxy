#include "PluginFdbusAdapter.h"
#include <../proto/MsgDef.pb.h>
#include <fdbus/fdbus.h>
#include <fdbus/CFdbProtoMsgBuilder.h>
#include <fdbus/cJSON/cJSON.h>
#include <fdbus/CFdbCJsonMsgBuilder.h>

void PluginFdbusAdapter::ConnectServer(std::string const & name)
{
    FDbusClientInterface::ConnectServer(name);
}
int PluginFdbusAdapter::SetAsyncSplitNum(int num)
{
    cutNum = num;
}

void PluginFdbusAdapter::AddSubscribe(std::vector<int> &msgIDs)
{
    FDbusClientInterface::AddSubscribe(msgIDs);
}

void PluginFdbusAdapter::AddSubscribe(int minid,int maxid)
{
    FDbusClientInterface::AddSubscribe(minid, maxid);
}

int PluginFdbusAdapter::execute(int key,std::string const &data)
{

    return executeEx( key,data);
}


int  PluginFdbusAdapter::executeEx(int key,std::string const &data,double p1 ,double p2, double p3 ,double p4 )
{

    FDBMsg inMsg,outMsg;
    inMsg.set_p1(p1);
    inMsg.set_p2(p2);
    inMsg.set_p3(p3);
    inMsg.set_p4(p4);
    inMsg.set_str(data);

    if(key >cutNum) // 异步调用
    {
        CallServerSync(key,inMsg,outMsg);
        return (int)outMsg.p1();
    }
    else{
       return CallServerAsync(key,inMsg);  // 返回调用成功或失败 ，只是和服务器通信
    }

    return 1;
}


int PluginFdbusAdapter::executeAsync(int key,std::string const &data ,double p1 ,double p2 , double p3,double p4)
{
    FDBMsg inMsg;
    inMsg.set_p1(p1);
    inMsg.set_p2(p2);
    inMsg.set_p3(p3);
    inMsg.set_p4(p4);
    inMsg.set_str(data);
    return CallServerAsync(key,inMsg);  // 返回调用成功或失败 ，只是和服务器通信
}
int PluginFdbusAdapter::OnNotify(int msgid,FDBMsg const &msg){

 
    Notify(msgid,msg.p1(),msg.str());

   // printf("recv server msg %d ,%s\n",msgid,msg.str().c_str());

};
