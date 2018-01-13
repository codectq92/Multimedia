/*************************************************************************
    > File Name: simplest_librtmp_send_flv.cpp
    > Author: zhongjihao
    > Mail: zhongjihao100@163.com
    > Created Time: 2018年01月09日 星期二 16时14分56秒
 ************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include "CRtmpPublicFlv.h"


//使用Rtmp_publish_using_packet函数将FLV格式的音视频文件通过RTMP协议推送至RTMP流媒体服务器
void testRtmpSendPacketFlv(const char* logpath,const char* flv,const char* destUrl)
{
	CRtmpPublicFlv* pRtmpSendFlv = new CRtmpPublicFlv;

	pRtmpSendFlv->Rtmp_LogSetLevel(RTMP_LOGALL);

	FILE* logfile = fopen(logpath, "w+");
	if(logfile)
		pRtmpSendFlv->Rtmp_LogSetOutput(logfile);

	printf("======haoge=====RTMPDump  send flv packet  start...\n");
    int total = pRtmpSendFlv->Rtmp_publish_using_packet(flv,destUrl);
	printf("=====haoge=====RTMPDump   send flv packet %d Byte done...\n",total);

	delete pRtmpSendFlv;
}


//使用Rtmp_publish_using_write函数将FLV格式的音视频文件通过RTMP协议推送至RTMP流媒体服务器
void testRtmpWriteByteFlv(const char* logpath,const char* flv,const char* destUrl)
{
	CRtmpPublicFlv* pRtmpSendFlv = new CRtmpPublicFlv;

	pRtmpSendFlv->Rtmp_LogSetLevel(RTMP_LOGALL);

	FILE* logfile = fopen(logpath, "w+");
	if(logfile)
		pRtmpSendFlv->Rtmp_LogSetOutput(logfile);

	printf("======haoge=====RTMPDump  write flv byte start...\n");
    int total = pRtmpSendFlv->Rtmp_publish_using_write(flv,destUrl);
	printf("=====haoge=====RTMPDump   write flv %d Byte done...\n",total);

	delete pRtmpSendFlv;
}

int main()
{
	//发布本地FLV视频文件到RTMP服务器的URL
	const char* publicUrl = "rtmp://10.0.142.118:1935/zhongjihao/myflv";
	//向RTMP服务器推流
	//testRtmpSendPacketFlv("log/rtmp_send_flv.log","res/cuc_ieschool.flv",publicUrl);

	testRtmpWriteByteFlv("log/rtmp_send_flv.log","res/cuc_ieschool.flv",publicUrl);
	return 0;
}

