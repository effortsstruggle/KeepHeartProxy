#include "LoopBuffer.h"
#include <unistd.h>
#include <stdio.h>
#include <memory.h>

#define min(a,b) ((a)<(b)?(a):(b))

LoopBuffer::LoopBuffer(int nMaxSize)
{
	m_nSize = nMaxSize;
	m_nPos = 0;
	m_nLen = 0;

	m_pBuffer = new unsigned char[m_nSize];
}

LoopBuffer::~LoopBuffer()
{
	if(m_pBuffer != NULL)
		delete[] m_pBuffer;
}

int LoopBuffer::GetData(unsigned char* pBuf, int nLength)
{
	if(m_pBuffer == NULL)
		return -1;

	int nRecved = 0;
    int nTimes = 0;
	while(nRecved<nLength && nTimes<=1)
	{
		int nSubRecved = min(m_nLen, nLength-nRecved);
		if(nSubRecved > 0)
		{
			if(m_nPos>0 && m_nPos+nSubRecved>m_nSize)
				nSubRecved = m_nSize-m_nPos;

			memcpy(pBuf+nRecved, m_pBuffer+m_nPos, nSubRecved);

			m_nPos += nSubRecved;
			if(m_nPos >= m_nSize)
				m_nPos -= m_nSize;

			m_nLen -= nSubRecved;
	
			nRecved += nSubRecved;
		}


        nTimes++;
	}

	return nRecved;
}

int LoopBuffer::GetData2(unsigned char* pBuf, int nLength)
{
	if(m_pBuffer == NULL)
		return -1;

	int nPos = m_nPos;
	int nLen = m_nLen;
	int nRecved = 0;
    int nTimes = 0;
	while(nRecved<nLength && nTimes<=1)
	{
		int nSubRecved = min(nLen, nLength-nRecved);
		if(nSubRecved > 0)
		{
			if(nPos>0 && nPos+nSubRecved>m_nSize)
				nSubRecved = m_nSize-nPos;

			memcpy(pBuf+nRecved, m_pBuffer+nPos, nSubRecved);

			nPos += nSubRecved;
			if(nPos >= m_nSize)
				nPos -= m_nSize;

			nLen -= nSubRecved;
			nRecved += nSubRecved;
		}

        nTimes++;
	}

	return nRecved;
}

bool LoopBuffer::AddData(const unsigned char* pData, int nDataLength)
{

	if(m_nSize-m_nLen < nDataLength)
	{
		return false;
	}

	while(nDataLength > 0)
	{
		int nOffset = m_nPos+m_nLen;
		if(nOffset >= m_nSize)
			nOffset -= m_nSize;

		int nSaved = min(m_nSize-m_nLen, nDataLength);

		if(nOffset+nSaved > m_nSize)
			nSaved = m_nSize-nOffset;

		memcpy(m_pBuffer+nOffset, pData, nSaved);

		m_nLen += nSaved;
		
		pData += nSaved;
		nDataLength -= nSaved;
	}

    return true;
}

unsigned char LoopBuffer::operator [] (int index)
{
    index += m_nPos;

    while(index >= m_nSize)
    {
        index -= m_nSize;
    }

    return m_pBuffer[index];
}

void LoopBuffer::Pop(int count)
{
    if(count > m_nLen)
        count = m_nLen;

    m_nPos += count;
    while(m_nPos >= m_nSize)
    {
        m_nPos -= m_nSize;
    }

    m_nLen -= count; 
}

