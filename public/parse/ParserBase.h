#pragma once 

#include <Buffer/LoopBuffer.h>

class SerialParseCollection;

class ParserBase
{
public:
    friend class SerialParseCollection;
    ParserBase();
    virtual ~ParserBase();

protected:
    // 原始数据，返回解析的数据长度
    virtual int ParseData(LoopBuffer&) = 0;
    void SendData(const void* data, int len);

    SerialParseCollection*      m_serial;

};
