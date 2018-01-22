/*************************************************************************
    > File Name: simplest_librtmp_recv_flv.cpp
    > Author: zhongjihao
    > Mail: zhongjihao100@163.com
    > Created Time: 2018年01月09日 星期二 16时14分56秒
 ************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include "CRtmpRecvFlv.h"

//测试接收RTMP流媒体并在本地保存成FLV格式的文件
void testRtmpRecvFlv(const char* logpath,const char* outFlv,const char* rtmpUrl)
{
	CRtmpRecvFlv* pRtmpRecvFlv = new CRtmpRecvFlv;

	pRtmpRecvFlv->Rtmp_SetIsLiveStream(true);
	pRtmpRecvFlv->Rtmp_LogSetLevel(RTMP_LOGALL);

	FILE* logfile = fopen(logpath, "w+");
	if(logfile)
		pRtmpRecvFlv->Rtmp_LogSetOutput(logfile);

	FILE* recvFlv = fopen(outFlv, "w+");
	if(recvFlv)
		pRtmpRecvFlv->Rtmp_FlvSetOutput(recvFlv);

	pRtmpRecvFlv->Rtmp_SetupURL(rtmpUrl);

	pRtmpRecvFlv->Rtmp_SetBufferMS(3600*1000);

	pRtmpRecvFlv->Rtmp_Connect();
	
	pRtmpRecvFlv->Rtmp_ConnectStream();

	printf("======haoge=====RTMPDump  DownLoad start...\n");
    int total = pRtmpRecvFlv->Rtmp_Read();
	printf("=====haoge=====RTMPDump  %d Byte DownLoad done...\n",total);

	delete pRtmpRecvFlv;
}

int main()
{
	//RTMP服务器上流媒体资源URL
	const char* publicUrl = "rtmp://10.0.142.118:1935/zhongjihao/myflv";
	//从RTMP服务器拉流
    testRtmpRecvFlv("log/rtmp_recvflv.log","out/rtmp_recv.flv",publicUrl);

	return 0;
}

