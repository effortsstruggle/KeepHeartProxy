#ifndef __SkyIPCClient_H_
#define __SkyIPCClient_H_

#include "SkyConfig.h"
class SkyIPCClient
{
private:
    /* data */
public:
    // 接受消息的回调函数
    SkyIPCClient(Fun1char );
    ~SkyIPCClient();

    int StartClient();
    int SendData(char * ,int len);
    void CloseClient();

private:
    static void *  threadRecv(void* args);
    Fun1char cbrecv;
    pthread_t t;
};

// 独立完整的测试程序
int clienttest();
#endif // !__SkyIPCClient_H_