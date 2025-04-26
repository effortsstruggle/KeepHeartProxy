#include <stdio.h>
#include <stdlib.h>
#include <termio.h>
#include <fcntl.h>
#include <memory.h>
#include <unistd.h>
#include "Serial.h"

Serial::Serial()
{
    m_fd = 0;
    m_bListen = false;
    m_pThread = NULL;
}

Serial::~Serial()
{
    Close();
}

// "/dev/ttyS1", 115200, 8, 'N', 1
bool Serial::Open(const char* path, int nSpeed,int nBits, char nEvent,int nStop)
{
    if(m_fd != 0)
    {
        close(m_fd);
    }

    m_fd = open(path, O_RDWR|O_NOCTTY|O_NONBLOCK);
    if(m_fd == -1)
    {
        printf("Can't open serial port %s \n",path);
        return false;
    }

    fcntl(m_fd,F_SETFL,0);

    if(!SetOpt(nSpeed, nBits, nEvent, nStop)) //配置串口
    {
        close(m_fd);
        m_fd = 0;

        return false;
    }
    
    return true;
}

void Serial::Close()
{
    StopListen();

    if(m_fd > 0)
        close(m_fd);

    m_fd = 0;
}

int Serial::Read(unsigned char* buf, int size)
{   
    return ::read(m_fd, buf, size);
}

int Serial::Write(unsigned char* buf,int size)
{
    return ::write(m_fd, buf, size);
}

//nSpeed：波特率，nBits：字符大小，nEvent：奇偶校验，nStop：停止位
bool Serial::SetOpt(int nSpeed,int nBits, char nEvent,int nStop)
{
    struct termios newtio,oldtio;

    if(tcgetattr(m_fd,&oldtio) != 0)//保存测试现有串口参数设置，在这里如果串口号等出错，会有相关的出错信息
    {
        printf("setupserial 1\n");
        return false;
    }

    memset(&newtio, 0, sizeof(newtio));//清零
    //设置字符大小
    newtio.c_cflag|=CLOCAL|CREAD;//本地连接和接收使能
    newtio.c_cflag&=~CSIZE;//去除数据位位掩码
    switch(nBits)
    {
    case 7:
            newtio.c_cflag|=CS7;//7个数据位
            break;
    case 8:
            newtio.c_cflag|=CS8;//8个数据位
            break;
    }

    //设置奇偶校验位
    switch(nEvent)
    {
    case 'O' ://奇数
            newtio.c_cflag|=PARENB;//校验位使能
            newtio.c_cflag|=PARODD;//使用奇校验
            newtio.c_iflag|=(INPCK|ISTRIP);//奇偶校验使能以及除去奇偶校验位
            break;
    case 'E' ://偶数
            newtio.c_iflag|=(INPCK|ISTRIP);
            newtio.c_cflag|=PARENB;
            newtio.c_cflag&=~PARODD;
            break;
    case 'N' :
            newtio.c_cflag&=~PARENB;
            break;
    }

    //设置波特率
    switch(nSpeed)
    {
        case 2400:
            cfsetispeed(&newtio,B2400);//输入波特率
            cfsetospeed(&newtio,B2400);//输出波特率
            break;
        case 4800:
            cfsetispeed(&newtio,B4800);
            cfsetospeed(&newtio,B4800);
            break;
        case 9600:
            cfsetispeed(&newtio,B9600);
            cfsetospeed(&newtio,B9600);
            break;
        case 38400:
            cfsetispeed(&newtio,B38400);
            cfsetospeed(&newtio,B38400);
            break;       
        case 115200:
            cfsetispeed(&newtio,B115200);
            cfsetospeed(&newtio,B115200);
            break;
        case 460800:
            cfsetispeed(&newtio,B460800);
            cfsetospeed(&newtio,B460800);
            break;
        default:
            cfsetispeed(&newtio,B9600);
            cfsetospeed(&newtio,B9600);
            break;
    }

//设置停止位
    if(nStop==1)
        newtio.c_cflag&=~CSTOPB;//CSTOPB不设置为2，设置1
    else if(nStop==2)
        newtio.c_cflag|=CSTOPB;

//设置等待时间和最小接收字符
    newtio.c_cc[VTIME]=0;//最小等待时间
    newtio.c_cc[VMIN]=0;//最小接收字符
    tcflush(m_fd,TCIFLUSH);//处理接收字符，刷新收到的数据但是不读

//激活新配置
    if((tcsetattr(m_fd,TCSANOW,&newtio))!=0)
    {
        printf("Failed to set com.\n");
        return false;
    }

    return true;
}

void Serial::StartListen()
{
    if(m_pThread == NULL)
    {
        m_pThread = new std::thread(&Serial::ListenThread, this);
    }
}

void Serial::StopListen()
{
    m_bListen = false;
    
    if(m_pThread != NULL)
    {
        m_pThread->join();
        delete m_pThread;
    }

    m_pThread = NULL;    
}

void Serial::ListenThread()
{
    m_bListen = true;
    unsigned char buf[256];

    while(m_bListen)
    {
        struct timeval timeout;
        timeout.tv_sec = 0;
        timeout.tv_usec = 100*1000;

        fd_set readfd;
        FD_ZERO(&readfd);
        FD_SET(m_fd, &readfd);

        int ret = select(m_fd+1, &readfd, 0, 0, &timeout);   
        if(ret <= 0)    
        {            
            if(ret < 0) //error
            {
                printf("select error. \n");
                break;
            }

            continue;   //timeout
        }

        if(FD_ISSET(m_fd, &readfd)) 
        {
            int nRead = Read(buf, 256);

            if(nRead > 0)
                OnData(buf, nRead);
        }

        usleep(1000);
    }
}