/*************************************************************************
    > File Name: simplest_librtmp_send_h264.cpp
    > Author: zhongjihao
    > Mail: zhongjihao100@163.com
    > Created Time: 2018年01月20日 星期二 16时14分56秒
 ************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include "CRtmpSendH264.h"

static FILE* fp_send1;

//读文件的回调函数  
//we use this callback function to read data from buffer
static int read_buffer1(unsigned char *buf, int buf_size )
{
    if(!feof(fp_send1))
	{
		int true_size = fread(buf,1,buf_size,fp_send1);
        return true_size;
    }
	else
		return -1;
}

int main()
{
	//发布本地H264视频到RTMP服务器的URL
	const char* publicUrl = "rtmp://10.0.142.118:1935/zhongjihao/myh264";
	CRtmpSendH264* pRtmpH264 = new CRtmpSendH264;
	FILE* logfile = fopen("log/rtmp_send_h264.log","w+");
    fp_send1 = fopen("res/cuc_ieschool.h264", "r");

	//初始化并连接到服务器
	pRtmpH264->RTMPH264_Connect(RTMP_LOGALL,publicUrl,logfile);

	printf("======haoge=====RTMPDump send h264 nalu start...\n");
	//向RTMP服务器推流
	pRtmpH264->RTMPH264_Send(read_buffer1);

	printf("======haoge=====RTMPDump send h264 nalu done...\n");
    //断开RTMP连接并释放相关资源
	pRtmpH264->RTMPH264_Close();

	fclose(logfile);
	fclose(fp_send1);
	delete pRtmpH264;

	return 0;
}

