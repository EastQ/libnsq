#ifndef _COMMAND_H_
#define _COMMAND_H_
#include <stdint.h>
#include <pthread.h>
#include <string>
#include "event2/event.h" 
#include "common.h"
#include <time.h>

namespace NSQTOOL
{
    struct CAddr
    {
        int32_t m_iThreadType;
        int32_t m_iThreadId;
        int32_t m_iHandlerId;

        CAddr()
        {
           m_iThreadType = -1;  
           m_iThreadId = -1;
           m_iHandlerId = -1;
        }
    };

    struct CCmdAddr
    {
        CAddr m_cSrcAddr;
        CAddr m_cDstAddr;
    };

    class CThread;

	class CCommand
	{
	public:
		CCommand(int32_t iCmdType, int32_t iCmdId);
		//不负责释放
		virtual ~CCommand();
		void SetCmdType(int32_t iCmdType);
        void SetCmdId(int32_t iCmdId);
		void SetLData(void *pData);
		void SetRData(void *pData);
		void *GetLData();
		void *GetRData();
		int32_t GetCmdType();
        int32_t GetCmdId();
		CCmdAddr &GetAddr();
		void SetAddr(CCmdAddr &cCmdAddr);
        int CheckTimeout();

	private:
		int32_t m_iCmdType;
        int32_t m_iCmdId;
		void *m_pLData;
		void *m_pRData;
		CCmdAddr m_cAddr;
        struct timeval m_cTimeBegin;
	}; 

    class CTcpConnectCommand:public CCommand
    {
    public: 
       CTcpConnectCommand(const string &strHost, uint16_t iPort, int32_t iCmdId)
           : CCommand(TCP_CONNECT_TYPE, iCmdId)
           , m_strHost(strHost), m_iPort(iPort) 
       {

       }

    public:
       string m_strHost;
       uint16_t m_iPort;
    };

    class CTcpAddCommand:public CCommand
    {
    public:
        CTcpAddCommand(int32_t iFd, int32_t iCmdId)
            :CCommand(TCP_ADD_TYPE, iCmdId), m_iFd(iFd)
        {
        
        }

    public:
        int32_t m_iFd;
    };

    class CTcpSendCommand:public CCommand
    {
    public:
        CTcpSendCommand(const char *pData, int32_t iLength)
            : CCommand(TCP_SEND_TYPE, -1)
            , m_pSendData(pData), m_iLength(iLength)
        {
             
        }

    public:
        const char *m_pSendData;
        int32_t m_iLength;
    };

    class CTcpReadCommand:public CCommand
    {
    public:         
        CTcpReadCommand():CCommand(TCP_READ_TYPE, -1)
        {
        
        }
    };

    class CTcpDelCommand:public CCommand
    {
    public:
        CTcpDelCommand(short iErrorType)
            : CCommand(TCP_DEL_TYPE, -1)
            , m_iErrorType(iErrorType)
        {
        
        }

    public:
        short m_iErrorType;
    };

    class CTcpListenAddCommand:public CCommand
    {
    public: 
       CTcpListenAddCommand(const string &strHost, uint16_t iPort,
               int32_t iCmdId):CCommand(TCP_LISTEN_ADD_TYPE, iCmdId)
               , m_strHost(strHost), m_iPort(iPort)
        {
         
        }

    public:
       string m_strHost;
       uint16_t m_iPort;
    };

    class CTcpListenDelCommand:public CCommand
    {
    public:
        CTcpListenDelCommand(const string &strHost, uint16_t iPort)
            :CCommand(TCP_LISTEN_DEL_TYPE, -1)
            , m_strHost(strHost), m_iPort(iPort)
        {
             
        }

    public:
       string m_strHost;
       uint16_t m_iPort;
    };

    class CTcpListenAcceptCommand:public CCommand
    {
    public:
        CTcpListenAcceptCommand(int iAcceptFd)
            : CCommand(TCP_LISTEN_ACCEPT_TYPE, -1)
            , m_iAcceptFd(iAcceptFd)
        {
        
        }

    public:
        int m_iAcceptFd;

    };

    class CTimerAddCommand:public CCommand
    {
    public: 
        CTimerAddCommand(struct timeval &cTimeval, 
                int iPersist, int iTimerType)
            : CCommand(TIMER_ADD_TYPE, -1)
            , m_cTimeval(cTimeval)
            , m_iPersist(iPersist)
            , m_iTimerType(iTimerType)
        {
            gettimeofday(&m_cUpdateTime, NULL);  
        }

    public:
        struct timeval m_cTimeval;
        struct timeval m_cUpdateTime;
        int m_iPersist;
        int m_iTimerType;
        event *m_pEvent;
        CThread *m_pThread;
    };


};
#endif
