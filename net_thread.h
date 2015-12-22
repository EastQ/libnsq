#ifndef _NET_THREAD_H_
#define _NET_THREAD_H_
#include "thread.h"
#include "event2/event.h"
#include "event2/bufferevent.h"
#include "event2/buffer.h"
#include "event2/listener.h"
#include <string>
#include <set>
#include <map>
#include <stdint.h>
#include <errno.h>
#include "listener.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

using namespace std;

namespace NSQTOOL
{
    class CTcpHandler;
    class CListenHandler;

	class CNetThread:public CThread
	{
	public:
		struct SNetContext
		{
            int m_iProtocolType;
            int m_iProtocolId;
            int m_iFd;
            string m_strHost;
            uint16_t m_iPort;
            CListenHandler *m_pListenHandler;
		};

		CNetThread(int32_t iThreadType, int32_t iThreadId);

        ~CNetThread() {}

		static void OnStaticRead(struct bufferevent *pBufevt, void *arg);
		static void OnStaticError(struct bufferevent *pBufevt, 
                                short iTemp, void *arg);
		void RealRun();
        void DestoryHandler(uint64_t iHandlerId);
        int SendData(struct bufferevent *pBufevt, 
                    const std::string *pString, 
                    bool bIsCopy = false);
    protected:
		void RealProcessCmd(CCommand &cCmd);
    private:
		void OnRead(struct bufferevent *pBufevt, void *arg);
		void OnError(struct bufferevent *pBufevt, short iTemp, void *arg);
	private:
		event_base *m_pEventBase;
	};

	class CListenThread:public CThread
	{
	public:
		struct SListenInfo
		{
            string m_strHost;
            uint16_t m_iPort;
			int m_iProtocolType;	
            int m_iProtocolId;
			evconnlistener *m_pListener;
            CListenHandler *m_pListenHandler;
		};


		CListenThread(int32_t iThreadType, int32_t iThreadId);
		void RealRun();
		static void OnStaticRead(struct evconnlistener *pListener, 
                            evutil_socket_t iAcceptHandle, 
                            struct sockaddr *pAddr, int socklen, void *pArg);		
		void RealProcessCmd(CCommand &cCmd);
        void DestoryHandler(uint64_t iHandlerId);
		void OnRead(struct evconnlistener *pListener, evutil_socket_t iAcceptHandle, 
                            struct sockaddr *pAddr, int socklen, void *pArg);		
	private:
		event_base *m_pEventBase;
	};
};
#endif
