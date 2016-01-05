#ifndef _PROTOCOL_H_
#define _PROTOCOL_H_
#include <stdint.h>
#include<string>

namespace NSQTOOL
{
	class CProtocol
	{
	public:
        CProtocol()
        {
            m_strStream = "";
        }

        virtual ~CProtocol()
        {
            m_strStream.clear();
        }

		virtual int32_t Need(const char *pData, int32_t iLength) = 0;
        int32_t NextPkg();
        virtual void Decode() = 0;

    protected:
        std::string m_strStream;
        size_t m_iCurPkgLength;
	};
};
#endif
