/*************************************************************************
	> File Name: timer_thread.cpp
	> Author: 
	> Mail: 
	> Created Time: 2015年10月25日 星期日 21时05分32秒
 ************************************************************************/

#include "timer_thread.h"

namespace NSQTOOL
{

CTimerThread::CTimerThread(int32_t iThreadType, int32_t iThreadId)
    : CEventThread(iThreadType, iThreadId)
{
    pthread_mutex_init(&m_mutex, NULL);
}

CTimerThread::~CTimerThread()
{
    pthread_mutex_lock(&m_mutex);
    map<string, CTimerAddCommand *>::iterator iter = m_mapTimer.begin();

    for (; iter != m_mapTimer.end(); ++iter)
    {
        evtimer_del(iter->second->m_pEvent);
        event_free(iter->second->m_pEvent);
        delete iter->second;
    }

    m_mapTimer.clear();

    pthread_mutex_unlock(&m_mutex);
    pthread_mutex_destroy(&m_mutex);
}

void CTimerThread::OnStaticTimeOut(int iHandle, short iEvent, void *pArg)
{
    CTimerThread *pThis = dynamic_cast<CTimerThread *>(((CTimerAddCommand *)pArg)->m_pThread);
    pThis->OnTimeOut(iHandle, iEvent, pArg);
}

void CTimerThread::OnTimeOut(int iHandle, short iEvent, void *pArg)
{
    pthread_mutex_lock(&m_mutex);

    CTimerAddCommand  *pTimerAddCommand = (CTimerAddCommand *)pArg;

    char buff[64] = {0};
    snprintf(buff, sizeof(buff), "%d_%d_%d"
            , pTimerAddCommand->GetAddr().m_cSrcAddr.m_iThreadType
            , pTimerAddCommand->GetAddr().m_cSrcAddr.m_iThreadId
            , pTimerAddCommand->m_iTimerType);

	NsqLogPrintf(LOG_DEBUG, "OnTimerOut:%s\n", buff);

    if (GetIntervalNow(&(pTimerAddCommand->m_cUpdateTime)) > g_iThreadDeadLockTime)
    {
        NsqLogPrintf(LOG_ERROR, "Check Dead Lock Time Out, ThreadType = %d, ThreadId = %d, HandlerId = %d"
               , pTimerAddCommand->GetAddr().m_cSrcAddr.m_iThreadType
               , pTimerAddCommand->GetAddr().m_cSrcAddr.m_iThreadId
               , pTimerAddCommand->GetAddr().m_cSrcAddr.m_iHandlerId); 
    }

    CCommand *pCmd = new CCommand(pTimerAddCommand->m_iTimerType, -1);
    PostRemoteCmd(pCmd
            , pTimerAddCommand->GetAddr().m_cSrcAddr.m_iThreadType
            , pTimerAddCommand->GetAddr().m_cSrcAddr.m_iThreadId
            , pTimerAddCommand->GetAddr().m_cSrcAddr.m_iHandlerId
            , -1);

    if ((pTimerAddCommand->m_iPersist == 0) && (m_mapTimer.find(buff) != m_mapTimer.end()))
    {
        evtimer_del(m_mapTimer[buff]->m_pEvent);
        event_free(m_mapTimer[buff]->m_pEvent);
        delete m_mapTimer[buff];
        m_mapTimer.erase(buff);
    }

    pthread_mutex_unlock(&m_mutex);
}

void CTimerThread::TimerAdd(CCommand *pCmd)
{
    CTimerAddCommand *pTimerAddCommand = dynamic_cast<CTimerAddCommand *>(pCmd);
    char buff[64] = {0};
    snprintf(buff, sizeof(buff), "%d_%d_%d", 
            pTimerAddCommand->GetAddr().m_cSrcAddr.m_iThreadType, pTimerAddCommand->GetAddr().m_cSrcAddr.m_iThreadId, pTimerAddCommand->m_iTimerType);

    if (m_mapTimer.find(buff) != m_mapTimer.end())
    {
       gettimeofday(&m_mapTimer[buff]->m_cUpdateTime, NULL);  
       delete pTimerAddCommand;
       return ;
    }

    event *pEvent = event_new(m_pEventBase, -1, EV_PERSIST, OnStaticTimeOut,pTimerAddCommand);
    evtimer_add(pEvent, &(pTimerAddCommand->m_cTimeval));

	NsqLogPrintf(LOG_DEBUG, "TimerAdd:%s\n", buff);
    pTimerAddCommand->m_pEvent = pEvent;
    pTimerAddCommand->m_pThread = this;
    m_mapTimer[buff] = pTimerAddCommand;
    //delete pCmd; 
}

void CTimerThread::RealProcessCmd(CCommand *pCmd)
{
    pthread_mutex_lock(&m_mutex);

    switch(pCmd->GetCmdType())
    {
        case TIMER_ADD_TYPE:
        {
            TimerAdd(pCmd);
    		pthread_mutex_unlock(&m_mutex);
            return ;
        }
        case TIMER_DEL_TYPE:
        {
        }
    }

    pthread_mutex_unlock(&m_mutex);
}

};
