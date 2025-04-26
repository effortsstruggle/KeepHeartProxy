#pragma once 

#include "Serial.h"
#include "LoopBuffer.h"

class McuParserBase;

class McuSerial : public Serial
{
public:
    McuSerial();
    ~McuSerial();
    
    void SetParser(McuParserBase* parser);
    int SendData(const void* data, int len);

protected:
    virtual void OnData(const unsigned char* data, int len);
    bool        Parse();

    pthread_mutex_t	m_mutex;
    LoopBuffer      m_buf; 
    McuParserBase*  m_parser;
};
