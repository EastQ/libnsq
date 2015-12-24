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

void CTimerThread::OnStaticTimeOut(int iHandle, short iEvent, void *pArg)
{
    CTimerThread *pThis = ((STimerInfo *)pArg)->m_pThis;
    pThis->OnTimeOut(iHandle, iEvent, pArg);
}

void CTimerThread::OnTimeOut(int iHandle, short iEvent, void *pArg)
{
    pthread_mutex_lock(&m_mutex);
    STimerInfo *pTimer = (STimerInfo *)pArg;
    char buff[64] = {0};
    snprintf(buff, sizeof(buff), "%d_%d_%d", 
            pTimer->m_iDstType, pTimer->m_iDstTid, pTimer->m_iCmdType);

    CCommand cmd(pTimer->m_iCmdType);
    CCommand::CCmdAddr cAddr;
    cAddr.m_iDstType = pTimer->m_iDstType;
    cAddr.m_iDstTid = pTimer->m_iDstTid;
    cmd.SetAddr(cAddr);
    CThreadMgrSingleton::GetInstance()->PostCmd(cAddr.m_iDstType, cmd, cAddr.m_iDstTid);

    if ((pTimer->m_iPersist == 0) && (m_mapTimer.find(buff) != m_mapTimer.end()))
    {
        evtimer_del(m_mapTimer[buff]->m_pEvent);
        event_free(m_mapTimer[buff]->m_pEvent);
        delete m_mapTimer[buff];
        m_mapTimer.erase(buff);
    }

    pthread_mutex_unlock(&m_mutex);
}

void CTimerThread::RealProcessCmd(CCommand &cCmd)
{
    pthread_mutex_lock(&m_mutex);

    switch(cCmd.GetCmdType())
    {
        case TIMER_ADD_TYPE:
        {
            STimerInfo *pTimer = (STimerInfo *)cCmd.GetLData();
            event *pEvent = event_new(m_pEventBase, -1, EV_PERSIST, OnStaticTimeOut,pTimer);
            evtimer_add(pEvent, &(pTimer->m_cTimeval));
            char buff[64] = {0};
            snprintf(buff, sizeof(buff), "%d_%d_%d", 
                    pTimer->m_iDstType, pTimer->m_iDstTid, pTimer->m_iCmdType);
            pTimer->m_pEvent = pEvent;
            pTimer->m_pThis = this;
            m_mapTimer[buff] = pTimer;
        }
        break;
        case TIMER_DEL_TYPE:
        {
            STimerInfo *pTimer = (STimerInfo *)cCmd.GetLData();
            char buff[64] = {0};
            snprintf(buff, sizeof(buff), "%d_%d_%d", 
                    pTimer->m_iDstType, pTimer->m_iDstTid, pTimer->m_iCmdType);

            if (m_mapTimer.find(buff) != m_mapTimer.end())
            {
                event_free(m_mapTimer[buff]->m_pEvent);
                delete m_mapTimer[buff];
                m_mapTimer.erase(buff);
            }

            delete pTimer;
        }
    }

    pthread_mutex_unlock(&m_mutex);
}

};
