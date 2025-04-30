#include "SocketHelper.h"
#include <memory>
#include <string.h>
int MsgConversion(MsgHeader* msg,char* data,char* destbuf)
{
	memset(destbuf,0,HEADERSIZE + msg->dataLen + 1);
   
    memcpy(destbuf,msg,HEADERSIZE);
	if(msg->dataLen >0 )
    	memcpy(&(destbuf[HEADERSIZE]),data,msg->dataLen);

    //显示测试数据
    MsgHeader * msgptr;
    msgptr = (MsgHeader*)destbuf;
   
}