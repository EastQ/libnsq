#include "nsqprotocol.h"

namespace NSQTOOL
{

int32_t CNsqdResponse::Need(const char *pData, int32_t iLength)
{
    if ((iLength==0) && (m_strStream.empty()))
    {
        fprintf(stdout, "null, needLen = 4\n");
        return 4;
    }

    fprintf(stdout, "stream = %d, length = %d\n", m_strStream.length(), iLength);

    if (iLength != 0)
    {
	    m_strStream.append(pData, iLength);
    }

    fprintf(stdout, "stream = %d, length = %d\n", m_strStream.length(), iLength);

	size_t needLen=0;
	if(m_strStream.length()<4){
		needLen=4;
	}else{
		needLen = ntohl(*(int32_t*)m_strStream.c_str()) + 4;
	}

    fprintf(stdout, "needLen = %d, strStream = %d\n", needLen, m_strStream.length());
	if(needLen<=m_strStream.length()){
		Decode(m_strStream.c_str(), needLen);
        m_iCurPkgLength = needLen;
		return 0;
	}

	return needLen - m_strStream.length();
}

int32_t CNsqdResponse::Process(CNetThread::SNetContext *pContext, CNetThread *pThread)
{
    fprintf(stdout, "CNsqdResponse::Process\n");
	if (GetFrameType() == CNsqdResponse::FrameTypeResponse)
	{
		if (GetResponce() ==  "_heartbeat_")
		{
            fprintf(stdout, "_hearbeat_\n");
			CNsqdRequest cNsqdRequest;
			cNsqdRequest.Nop();
			//PutMsgToSendList(buff, cNsqdRequest.GetBuff().size(), true);
            pThread->SendData(pContext->m_iHandle, &cNsqdRequest.Encode());
		}

        fprintf(stdout, "response = %s\n", GetResponce().c_str());
	}
	else if (GetFrameType() == CNsqdResponse::FrameTypeMessage)
	{
        std::string &strMsgId = GetMsgId();
        std::string &strBody = GetBody();
        fprintf(stdout, "msg:%s\n", strBody.c_str());
		CNsqdRequest cNsqdRequest;
		cNsqdRequest.Finish(strMsgId);
        pThread->SendData(pContext->m_iHandle, &cNsqdRequest.Encode());
    }		
}

void CNsqdResponse::OnConnect(CNetThread::SNetContext *pContext, CNetThread *pThread)
{
    fprintf(stdout, "NsqdResponse:OnConnect\n");
	CNsqdRequest cNsqdRequest;
	cNsqdRequest.Megic();	
	cNsqdRequest.Subscribe("lhb", "test");
	cNsqdRequest.Ready(100);
    fprintf(stdout, "OnConnect:m_iHandle = %d\n", pContext->m_iHandle);
    pThread->SendData(pContext->m_iHandle, &cNsqdRequest.Encode());
}

void CNsqdResponse::OnError(CNetThread::SNetContext *pContext, CNetThread *pThread, short iEvent)
{
    fprintf(stdout, "OnError, iEvent = %d\n", iEvent);
}

int32_t CNsqLookupResponse::Process(CNetThread::SNetContext *pContext, CNetThread *pThread)
{
    fprintf(stdout, "CNsqLookupResponse:Process\n");
	DecodeResponseBody();
}

void CNsqLookupResponse::OnConnect(CNetThread::SNetContext *pContext, CNetThread *pThread)
{
    fprintf(stdout, "CNsqLookupResponse:OnConnect\n");
    CHttpRequest cHttpRequest;
    cHttpRequest.SetRequest("/lookup?topic=lhb", "127.0.0.1:4161");
    fprintf(stdout, "senddata = %s\n", cHttpRequest.Encode().c_str());
    pThread->SendData(pContext->m_iHandle, &cHttpRequest.Encode());
}

void CNsqLookupResponse::OnError(CNetThread::SNetContext *pContext, CNetThread *pThread, short iEvent)
{
    fprintf(stdout, "CNsqLookupResponse:OnError, iEvent = %d\n", iEvent);
}

int32_t CNsqLookupResponse::DecodeResponseBody()
{
    string &strBodyTemp = CHttpResponse::GetBody();
    const char *chBody = strBodyTemp.c_str();
    Json::Reader reader;
    Json::Value root;

    
    if (!reader.parse(chBody, root))
    {
        fprintf(stdout, "parse root failed!\n");
        return -1;
    }

    m_strStatus = root["status_code"].asString();
    m_strStatusTxt = root["status_txt"].asString();

    if (m_strStatus != "200")
    {
        fprintf(stdout, "response return failed errorinfo is:%s\n" ,m_strStatusTxt.c_str());
        return 0;
    }

    m_vecChannels.clear();
    m_vecProducers.clear();

    const Json::Value data = root["data"];
    const Json::Value channel = data["channels"];

    fprintf(stdout, "channel size = %d\n" ,  channel.size());

    for (size_t i = 0; i < channel.size(); ++i)
    {
        fprintf(stdout, "i = %d, channel = %s\n", i, channel[int(i)].asString().c_str());
        m_vecChannels.push_back(channel[int(i)].asString());	
    }

    const Json::Value producers = data["producers"];
    fprintf(stdout, "producers size = %d\n", producers.size());

    for (size_t i = 0; i < producers.size(); ++i)
    {
        SProducers item;
        item.m_strBroadcastAddres = producers[int(i)]["broadcast_address"].asString();
        item.m_strHostName = producers[int(i)]["hostname"].asString();
        item.m_iTcpPort = producers[int(i)]["tcp_port"].asInt();
        item.m_iHttpPort = producers[int(i)]["http_port"].asInt();
        item.m_strVersion = producers[int(i)]["version"].asString();

        fprintf(stdout, "i = %d, boardAddres = %s, hostName = %s, tcpPort = %d, httpPort = %d, version = %s\n", 
               i, item.m_strBroadcastAddres.c_str(), item.m_strHostName.c_str(), item.m_iTcpPort, item.m_iHttpPort, item.m_strVersion.c_str());

        m_vecProducers.push_back(item);
    }

    return 0;
}

};

