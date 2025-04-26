#include <unistd.h>
#include "McuSerial.h"
#include "McuParserBase.h"
// #include "McuData.h"

// #include <android/log.h>


// #define  LOGD2(...) __android_log_print(ANDROID_LOG_DEBUG, "SKY_MCU", __VA_ARGS__);

McuSerial::McuSerial():m_buf(1000), m_parser(NULL)
{
    pthread_mutex_init(&m_mutex, NULL);
}

McuSerial::~McuSerial()
{
    pthread_mutex_destroy(&m_mutex);
}

void McuSerial::SetParser(McuParserBase* parser)
{
    parser->m_serial = this;
    m_parser = parser;
}

void McuSerial::OnData(const unsigned char* data, int len)
{
 
  //  LOGD2("AddData  len = %d",len);
    m_buf.AddData(data, len);


    while(Parse())
    {
        //do nothing here...
    }
}

bool McuSerial::Parse()
{
   
    if(m_parser == NULL)
    {
        m_buf.Pop(m_buf.GetSize());
      //  LOGD2("m_parser empty ")
        return false;
    }
    
    
    int res = m_parser->ParseData(m_buf);
 //   LOGD2("ParseData res: %d,m_buf.GetSize():%d",res,m_buf.GetSize());
   if(res < 0)
        return false;

    
    return true;
}

int McuSerial::SendData(const void* data, int len)
{
    int ret = 0;

    pthread_mutex_lock(&m_mutex);
    ret = Write((unsigned char*)data, len);
    pthread_mutex_unlock(&m_mutex);    

    return ret; 
}
