/*************************************************************************
    > File Name: CRtmpRecvFlv.cpp
    > Author: zhongjihao
    > Mail: zhongjihao100@163.com
    > Created Time: 2018年01月08日 星期一 15时10分53秒
 ************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include "CRtmpRecvFlv.h"


CRtmpRecvFlv::CRtmpRecvFlv()
{
	m_pRtmp = Rtmp_Alloc();
	Rtmp_Init();
    //set connection timeout,default 30s
	m_pRtmp->Link.timeout = 10;
}

CRtmpRecvFlv::~CRtmpRecvFlv()
{
	Rtmp_Close();
	Rtmp_Free();
}

void CRtmpRecvFlv::Rtmp_Init()
{
	RTMP_Init(m_pRtmp);
}

RTMP* CRtmpRecvFlv::Rtmp_Alloc()
{
	return RTMP_Alloc();
}

void CRtmpRecvFlv::Rtmp_Free()
{
	if(m_pRtmp)
		RTMP_Free(m_pRtmp);
	m_pRtmp = NULL;
}

void CRtmpRecvFlv::Rtmp_Close()
{
	if(m_pRtmp)
		RTMP_Close(m_pRtmp);
}

void CRtmpRecvFlv::Rtmp_SetBufferMS(int bufferTime)
{
	RTMP_Log(RTMP_LOGDEBUG, "%s: ====haoge=====Setting buffer time to: %dms", __FUNCTION__,bufferTime);
	RTMP_SetBufferMS(m_pRtmp, bufferTime);
}

void CRtmpRecvFlv::Rtmp_LogSetLevel(RTMP_LogLevel level)
{
	RTMP_LogSetLevel(level);
}

void CRtmpRecvFlv::Rtmp_SetIsLiveStream(bool isLiveStream)
{
	m_bLiveStream = isLiveStream;
	if(m_bLiveStream)
	{
		m_pRtmp->Link.lFlags |= RTMP_LF_LIVE;
	}
}

void CRtmpRecvFlv::Rtmp_LogSetOutput(FILE* logoutfile)
{
	RTMP_LogSetOutput(logoutfile);
}

void CRtmpRecvFlv::Rtmp_FlvSetOutput(FILE* flvoutfile)
{
	m_recvFile = flvoutfile;
}

int CRtmpRecvFlv::Rtmp_SetupURL(const char *url)
{
	if(!RTMP_SetupURL(m_pRtmp,const_cast<char*>(url)))
	{
		RTMP_Log(RTMP_LOGERROR,"%s: ====haoge=====SetupURL Err",__FUNCTION__);
		Rtmp_Free();
		return RD_NO_CONNECT;
	}
	return RD_SUCCESS;
}

int CRtmpRecvFlv::Rtmp_Connect()
{
	RTMP_Log(RTMP_LOGDEBUG,"%s: ====haoge====Connecting ...",__FUNCTION__);
	if(!RTMP_Connect(m_pRtmp, NULL))
	{
		RTMP_Log(RTMP_LOGERROR,"%s: ====haoge======Connect Err",__FUNCTION__);
		Rtmp_Free();
		return RD_NO_CONNECT;
	}

	RTMP_Log(RTMP_LOGDEBUG, "%s: =====haoge=====Connected...",__FUNCTION__);
    return RD_SUCCESS;
}

int CRtmpRecvFlv::Rtmp_ConnectStream()
{
	RTMP_Log(RTMP_LOGDEBUG,"%s: ====haoge====ConnectStream ...",__FUNCTION__);
	if(!RTMP_ConnectStream(m_pRtmp, 0))
	{
		RTMP_Log(RTMP_LOGERROR,"%s: ====haoge====ConnectStream Err",__FUNCTION__);
		Rtmp_Close();
		Rtmp_Free();
		return RD_FAILED;
	}

	RTMP_Log(RTMP_LOGDEBUG, "%s: =====haoge=====ConnectStream Done...",__FUNCTION__);
	return RD_SUCCESS;
}

int CRtmpRecvFlv::Rtmp_Read()
{
	if(!m_recvFile)
	{
		RTMP_Log(RTMP_LOGERROR,"%s: ======haoge===Open File Error.",__FUNCTION__);
		return RD_FAILED;
	}
	int bufferSize = 64 * 1024;
	int nRead = 0;
	int countbufsize = 0;
	char* buffer = (char *) malloc(bufferSize);
	do
	{
		nRead = RTMP_Read(m_pRtmp, buffer, bufferSize);
		if(nRead > 0)
		{
			if(fwrite(buffer, sizeof(unsigned char), nRead, m_recvFile) != (size_t) nRead)
			{
				RTMP_Log(RTMP_LOGERROR, "%s: =====haoge====Failed writing, exiting!", __FUNCTION__);
				free(buffer);
				fclose(m_recvFile);
				return RD_FAILED;
			}
			countbufsize += nRead;
			printf("%s: ====haoge====RTMPDump Receive: %5d Byte, Total: %5.2fKB\n",__FUNCTION__,nRead,countbufsize*1.0/1024);
			RTMP_Log(RTMP_LOGDEBUG,"%s: ======haoge====Receive: %5dByte, Total: %5.2fkB\n",__FUNCTION__,nRead,countbufsize*1.0/1024);
		}
	}while (nRead > 0 && RTMP_IsConnected(m_pRtmp) && !RTMP_IsTimedout(m_pRtmp));

	free(buffer);
	fclose(m_recvFile);

	return countbufsize;
}


