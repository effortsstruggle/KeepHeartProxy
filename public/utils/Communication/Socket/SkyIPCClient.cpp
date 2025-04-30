#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <iostream>
#include <unistd.h>
#include "SkyConfig.h"
#include "SkyIPCClient.h"
static int nSockFd = -1;
static int nLen;
static char szBuf[1024];

static int ConnectServer(const char *szIp, int nPort)
{
    int nSockFd;
    int nAddrLen;
    struct sockaddr_in addr;

    if ((nSockFd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
    {
        return -1;
    }

    addr.sin_family = AF_INET;
    addr.sin_port = htons(nPort);
    if (inet_aton(szIp, &addr.sin_addr) < 0)
    {
        return -2;
    }

    nAddrLen = sizeof(addr);
    if (connect(nSockFd,(struct sockaddr *)&addr, nAddrLen) == -1)
    {
        return -3;
    }
    return nSockFd;
}

SkyIPCClient::SkyIPCClient(Fun1char cb):cbrecv(cb)
{
}

SkyIPCClient::~SkyIPCClient()
{
}

void* SkyIPCClient::threadRecv(void* args)
{

    ReConnect:
    nSockFd = -1;
    while (true)
    {
        nSockFd = ConnectServer("127.0.0.1",DEFAULTPORT);
        if (nSockFd > 0)
        {
            break;
        }
        TRACE("SkyIPCClient ConnectServer failed. ERROR:%d ,will Reconnect aften %d ms\n",nSockFd,Reconnection_Interval);
            sleep(Reconnection_Interval);
    }
    TRACE("SkyIPCClient ConnectServer success. \n" );
    char buf[512] ;
    SkyIPCClient* tt = (SkyIPCClient*)args;
    int res = 1;
    while(res >0)
    {
        memset(buf,0,sizeof(buf));
        res = recv(nSockFd, buf, sizeof(buf), 0);
        if(res <= 0)
        {
            TRACE("SkyIPCClient recv ERROR: %d",res);
            goto ReConnect;
        }
        else
        {
            tt->cbrecv(buf);
            // TRACE("SkyIPCClient recved data : %d",res);
        }
            

    }
    return nullptr;
}
int SkyIPCClient::StartClient()
{
    // 后面应该建立线程去不断的尝试连接
    
   
    pthread_create(&t, nullptr, threadRecv, this);
    return 0;
}
void SkyIPCClient::CloseClient(){
     close(nSockFd);
}

int SkyIPCClient::SendData(char * data,int len)
{
 //   TRACE("SkyIPCClient ConnectServer success.");
    int ret =send(nSockFd,data,len,0);
    if(ret <= 0){
        TRACE("SkyIPCClient send failed. return: %d\n",ret);
    }

    //这种方式发送会失败,原因？
    // if(send(nSockFd,data,len,0) <= 0){
    //     TRACE("send failed. return: \n");
    //     exit(0);
    // }
}



int clienttest(){
    nSockFd = ConnectServer("127.0.0.1",DEFAULTPORT);
    if (nSockFd < 0)
    {
        TRACE("ConnectServer failed.\n");
        exit(0);
    }

    while(fgets(szBuf, 1024, stdin) != NULL)
    {
        int ret = send(nSockFd,szBuf,strlen(szBuf),0);
        if(ret <= 0)
        {
            TRACE("SkyIPCClient send failed.return: %d\n",ret);
            exit(0);
        }
    }
    close(nSockFd);
    exit(0);
}