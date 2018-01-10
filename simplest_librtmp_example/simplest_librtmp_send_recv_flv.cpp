/*************************************************************************
    > File Name: simplest_librtmp_send_recv_flv.cpp
    > Author: zhongjihao
    > Mail: zhongjihao100@163.com
    > Created Time: 2018年01月09日 星期二 16时14分56秒
 ************************************************************************/

#include <stdio.h>
#include "CRtmpDumpFlv.h"

int main()
{
	CRtmpDumpFlv* pRtmpFlv = new CRtmpDumpFlv;

	pRtmpFlv->Rtmp_SetIsLiveStream(true);
	pRtmpFlv->Rtmp_LogSetLevel(RTMP_LOGALL);

	FILE* logfile = fopen("rtmp_flv.log", "w+");
	if(logfile)
		pRtmpFlv->Rtmp_LogSetOutput(logfile);

	FILE* recvFlv = fopen("rtmp_recv.flv", "w+");
	if(recvFlv)
		pRtmpFlv->Rtmp_FlvSetOutput(recvFlv);

	// HKS's live URL
	const char* url = "rtmp://10.0.142.118:1935/live...vhost...demo.srs.com/livestream_sd";
	if(pRtmpFlv->Rtmp_SetupURL(url) != RD_SUCCESS)
		return -1;

	pRtmpFlv->Rtmp_SetBufferMS(3600*1000);

	if(pRtmpFlv->Rtmp_Connect() != RD_SUCCESS)
		return -1;

	if(pRtmpFlv->Rtmp_ConnectStream() != RD_SUCCESS)
		return -1;

	printf("======haoge=====RTMPDump  DownLoad start...\n");
    int total = pRtmpFlv->Rtmp_Read();
	printf("=====haoge=====RTMPDump  %d Byte DownLoad done...\n",total);

	delete pRtmpFlv;

	return 0;
}

