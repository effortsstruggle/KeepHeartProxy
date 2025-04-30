#ifndef __SKYIPCSERVER_H_
#define __SKYIPCSERVER_H_
#include <iostream>
#include <vector>
#include "SkyConfig.h"


class SkyIPCServer
{
private:
    /* data */
public:
    SkyIPCServer(FunCallback);
    ~SkyIPCServer();

    //类创建后，必须先设置回调函数
  //  int RegisterMonitor(Fun1WRet* );
    int StartServer(unsigned int port =  DEFAULTPORT);
    int sendData(int fd,char * data,int len);

private:
    int ServerBuildSocket(unsigned int port);
    FunCallback monitor;
    int fd_A[BACKLOG];
};



#endif // !__SKYIPCSERVER_H_