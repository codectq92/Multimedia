/*************************************************************************
    > File Name: simplest_librtmp_send_flv.cpp
    > Author: zhongjihao
    > Mail: zhongjihao100@163.com
    > Created Time: 2018年01月09日 星期二 16时14分56秒
 ************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include "CRtmpPublicFlv.h"


//测试将FLV格式的音视频文件使用RTMP推送至RTMP流媒体服务器
void testRtmpSendFlv(const char* logpath,const char* flv,const char* destUrl)
{
	CRtmpPublicFlv* pRtmpSendFlv = new CRtmpPublicFlv;

	pRtmpSendFlv->Rtmp_LogSetLevel(RTMP_LOGALL);

	FILE* logfile = fopen(logpath, "w+");
	if(logfile)
		pRtmpSendFlv->Rtmp_LogSetOutput(logfile);

	printf("======haoge=====RTMPDump  send flv start...\n");
    int total = pRtmpSendFlv->Rtmp_publish_using_packet(flv,destUrl);
	printf("=====haoge=====RTMPDump   send flv %d Byte done...\n",total);

	delete pRtmpSendFlv;
}

int main()
{
	//发布本地FLV视频文件到RTMP服务器的URL
	const char* publicUrl = "rtmp://10.0.142.118:1935/zhongjihao/myflv";
	//向RTMP服务器推流
	testRtmpSendFlv("log/rtmp_send_flv.log","res/cuc_ieschool.flv",publicUrl);

	return 0;
}

