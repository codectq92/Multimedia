/*************************************************************************
    > File Name: CRtmpPublicFlv.h
    > Author: zhongjihao
    > Mail: zhongjihao100@163.com
    > Created Time: 2018年01月08日 星期一 15时10分53秒
 ************************************************************************/

#ifndef RTMP_PUBLIC_FLV_H
#define RTMP_PUBLIC_FLV_H

#include "libRTMP/librtmp/rtmp_sys.h"
#include "libRTMP/librtmp/log.h"

#define RD_SUCCESS        0
#define RD_FAILED         1
#define RD_INCOMPLETE     2
#define RD_NO_CONNECT     3


//将FLV格式的音视频文件使用RTMP推送至RTMP流媒体服务器
class CRtmpPublicFlv
{
private:
	RTMP *m_pRtmp;

private:
	//RTMP初始化
	void Rtmp_Init();
	//释放RTMP连接资源
	void Rtmp_Close();
	//分配RTMP空间
	RTMP* Rtmp_Alloc();
	//释放RTMP空间
	void Rtmp_Free();

public:
	CRtmpPublicFlv();
	~CRtmpPublicFlv();
	//设置RTMP中log级别
	void Rtmp_LogSetLevel(RTMP_LogLevel level);
	//设置RTMP中log输出文件
	void Rtmp_LogSetOutput(FILE* logoutfile);
	//设置RTMP连接的URL
	int Rtmp_SetupURL(const char *url);
	//设置RTMP中buffer time
	void Rtmp_SetBufferMS(int size);
	//建立RTMP网络连接NetConnection
	int Rtmp_Connect();
	//建立RTMP网络流NetStream
	int Rtmp_ConnectStream();
	//使用RTMP_SendPacket该API发布本地FLV文件到服务器
	int Rtmp_publish_using_packet(const char* sourceFlv,const char* destRtmpUrl);
	//使用RTMP_Write该API发布本地FLV文件到服务器
	int Rtmp_publish_using_write(const char* sourceFlv,const char* destRtmpUrl);
};

#endif


