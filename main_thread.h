/*************************************************************************
	> File Name: main_thread.h
	> Author: 
	> Mail: 
	> Created Time: 2015年10月25日 星期日 22时41分15秒
 ************************************************************************/

#ifndef _MAIN_THREAD_H
#define _MAIN_THREAD_H
#include "thread.h"
#include "handler.h"
#include "tcp_handler.h"
#include "nsqlookup_protocol.h"

namespace NSQTOOL
{
    struct CHandlerContext
    {
        CTcpHandler *m_pHandler; 
        int m_iProtocolId;
        string m_strNsqdHost;
        uint16_t m_iNsqdPort;
        string m_strTopic;
        string m_strChannel;
    };

    typedef void (*BIZCALLBACK)(int, const string&, const string&);

    struct CNsqLookupContext
    {
        string m_strLookupHost;
        uint16_t m_iLookupPort;
        string m_strTopic;
        string m_strChannel;
        BIZCALLBACK m_funcCallBack;
    };


    class CMainThread:public CThread
    {
    public:
        enum MAINTHREADTIMER
        {
            LOOKUP_TIMER = 1,
        };

        CMainThread(int iThreadType, int iThreadId)
           : CThread(iThreadType, iThreadId)
        {
        
        }

        void RealProcessCmd(CCommand &cCmd); 

        static void InitSuperServer();
        static void StartSuperServer();

        static void SetConnectNum(int iProducerNum = 5); //每个生产者连接个数

        static void SetProducer(const string &strLookupHost,
                                uint16_t iLookupPort, 
                                const string &strTopic, 
                                BIZCALLBACK pFunc
                                );

        static void SetConsumer(const string &strLookupHost, 
                                uint16_t iLookupPort,
                                const string &strTopic, 
                                const string &strChannel,
                                BIZCALLBACK pFunc
                                );

        static CNsqLookupContext &LookupConnectCallBack(int iProtocolId);
        static void LookupReadCallBack(int iProtocolId, const vector<string> &vecChannels, 
                                        const vector<CNsqLookupResponse::SProducers> &vecProducers);
        static void NewNsqdConnect(const string &strTopic, const string &strChannel, 
                            const string &strHost, uint16_t iPort);

        static CHandlerContext &NsqdConnectCallBack(int iProtocolId, CTcpHandler *pHandler);
        static void NsqdReadCallBack(int iProtocolId, const string &strMsgId, const string &strMsgBody);
        static void NsqdErrorCallBack(int iProtocolId);
        static int ProducerMsg(const string &strTopic, const string &strMsg);

        
    private:
        static map<string, vector<CHandlerContext> > m_mapTopic2Handler;
        static map<int, string> m_mapHandler2Topic;
        static map<int, CNsqLookupContext> m_mapLookupContext;
        static int m_iConnectNum;
        static int m_iProtocolId;
        static pthread_mutex_t m_mutex;
    };
};

#endif
