
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "SkyConfig.h"
#include "SkyIPCServer.h"


SkyIPCServer::SkyIPCServer(FunCallback cb):monitor(cb)
{
    // 创建时不做任何事情，因为失败无返回值
}

SkyIPCServer::~SkyIPCServer()
{
}


int conn_amount;


int SkyIPCServer::ServerBuildSocket(unsigned int port)
{
    //Build socket for server ,bind and listen the port
    
    int sock_fd;
    int nOptVal = 1;
    struct sockaddr_in server_addr;

    if ((sock_fd = socket(AF_INET,SOCK_STREAM,0)) == -1)
    {
        return -1; 
    }

    if (setsockopt(sock_fd, SOL_SOCKET, SO_REUSEADDR,&nOptVal,sizeof(int)) == -1)
    {
        return -2;
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    server_addr.sin_addr.s_addr = INADDR_ANY;

    if (bind(sock_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1)
    {
        return -3;
    }

    if (listen(sock_fd,BACKLOG) == -1)
    {
        return -4;
    }

    return sock_fd;

}

int SkyIPCServer::StartServer(unsigned int port)
{

//    ServerBuildSocket(port);
    int nSocktfd, nNewSocktfd;
    int i,nRet,nMaxSock,nSockaddrLen;
    struct sockaddr_in client_addr;
    char szBuf[BUF_SIZE];
    fd_set fdRead;
    struct timeval tv;
    
    nSocktfd = ServerBuildSocket(DEFAULTPORT);
    if (nSocktfd < 0)
    {
        TRACE("ServerBuildSocket faile.\n");
        return nSocktfd;
    }

    conn_amount = 0;
    nSockaddrLen = sizeof(client_addr);
    nMaxSock = nSocktfd; 

    while(1)
    {
        //将套节字集合清空
        FD_ZERO(&fdRead);
        // 监听 nSocktfd ，用来判断是否有新的连接
        FD_SET(nSocktfd,&fdRead);

        tv.tv_sec = 30;
        tv.tv_usec = 0;

        //add active connect to fd set
        //加入感兴趣的套节字到集合,这里是一个读数据的套节字s
        for (i = 0; i < BACKLOG; i++) {
            if (fd_A[i] != 0)
            {
                FD_SET(fd_A[i],&fdRead);
            }
        }
        //检查套节字是否可读,//很多情况下就是是否有数据(注意,只是说很多情况)
        //这里select是否出错没有写
        nRet = select(nMaxSock + 1, &fdRead, NULL, NULL, &tv);
        if (nRet < 0)
        {
            TRACE("select failed.\n");
            break;
        }
        else if(nRet == 0)
        {
            TRACE("timeout.\n");
            continue;
        }

        //check whether the connection have data to recieve 
        for (i = 0; i < BACKLOG; i++)
        {
            //检查 fd_A[i] 是否在 fdRead 里面,select将更新这个集合,把其中不可读的套节字去掉
            //只保留符合条件的套节字在这个集合里面
            if (fd_A[i] && FD_ISSET(fd_A[i],&fdRead))
            {
                nRet = recv(fd_A[i],szBuf,sizeof(szBuf),0);
                if (nRet <= 0)
                {
                    TRACE("Client %d close\n",i);
                    close(fd_A[i]);
                    FD_CLR(fd_A[i],&fdRead);
                    fd_A[i] = 0;
                    conn_amount --;
                }
                else
                {
                    if (nRet <= BUF_SIZE){
                        memset(&szBuf[nRet],'\0',1);
                        if(monitor)
                            monitor(fd_A[i],szBuf);
                        // TRACE("Client[%d] send: %s.\n",i,szBuf);
                    }
                }
            }
         }//for

        //check whether a new connection comes
        // 检查是否有新的连接
        if (FD_ISSET(nSocktfd,&fdRead))
        {
            nNewSocktfd = accept(nSocktfd, (struct sockaddr *)&client_addr,(socklen_t*)&nSockaddrLen);
            if (nNewSocktfd <= 0)
            {
                TRACE("accept failed.\n");
                continue;
            }
            
            if (conn_amount < BACKLOG -1)
            {
                for(i = 0; i < BACKLOG; i++)
                {
                    if(fd_A[i] == 0)
                    {
                        fd_A[i] = nNewSocktfd;
                        conn_amount ++;
                        break;
                    }
                }
                // TRACE("new connection client[%d] %s:%d\n", conn_amount,inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));

                if (nNewSocktfd > nMaxSock)
                {
                    nMaxSock = nNewSocktfd;
                }
            }
            else
            {
                TRACE("max connections arrive,exit\n");
                send(nNewSocktfd,"The link is full",16,0);
                close(nNewSocktfd);
                continue;
            }
        }
       }//while

    //close all connections
    for (i = 0; i < BACKLOG; i++)
    {
        if (fd_A[i] != 0)
        {
            close(fd_A[i]);
        }
    }
    return(0);
    
}

int SkyIPCServer::sendData(int fd,char * data,int len)
{
    int res = send(fd,data,len,0);
    if(res < 0)
        TRACE("SkyIPCServer send error : %d \n",res);
}