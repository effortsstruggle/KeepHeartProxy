#pragma once 

#include <Communication/Serial/Serial.h>
#include <Buffer/LoopBuffer.h>

class ParserBase;
class SerialParseCollection : public Serial
{
public:
    SerialParseCollection();
    ~SerialParseCollection();
    
    void SetParser(ParserBase* parser);
    int SendData(const void* data, int len);

protected:
    virtual void OnData(const unsigned char* data, int len);
    bool        Parse();

    pthread_mutex_t	m_mutex;
    LoopBuffer      m_buf; 
    ParserBase*  m_parser;
};
