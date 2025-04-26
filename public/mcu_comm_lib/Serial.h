#pragma once

#undef min
#undef max

#include <thread>
using namespace std;

class Serial
{
public:
	Serial();
	~Serial();

    bool Open(const char* path, int nSpeed,int nBits, char nEvent,int nStop);	
    void Close();

    void StartListen();
    void StopListen();

	int Read(unsigned char* buf, int size);
    int Write(unsigned char* buf, int size);

protected:
    void ListenThread();

    bool SetOpt(int nSpeed,int nBits, char nEvent,int nStop);
    virtual void OnData(const unsigned char* data, int len) = 0;

    int             m_fd;
    thread*         m_pThread;
    bool            m_bListen;
};
