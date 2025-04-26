#include "ParserBase.h"
#include "SerialParseCollection.h"

ParserBase::ParserBase() : m_serial(NULL)
{
}

ParserBase::~ParserBase()
{
    
}

void ParserBase::SendData(const void* data, int len) 
{
    if(m_serial != NULL)
        m_serial->SendData(data, len);
}
