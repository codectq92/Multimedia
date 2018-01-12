/*************************************************************************
    > File Name: CRtmpRecvFlv.h
    > Author: zhongjihao
    > Mail: zhongjihao100@163.com
    > Created Time: 2018年01月08日 星期一 15时10分53秒
 ************************************************************************/

#ifndef RTMP_RECV_FLV_H
#define RTMP_RECV_FLV_H

#include "libRTMP/rtmpdump/librtmp/rtmp_sys.h"
#include "libRTMP/rtmpdump/librtmp/log.h"


#define RD_SUCCESS        0
#define RD_FAILED         1
#define RD_INCOMPLETE     2
#define RD_NO_CONNECT     3


//通过RTMP流媒体协议接收FLV数据
class CRtmpRecvFlv
{
private:
	RTMP *m_pRtmp;
	//是否是直播流
	bool m_bLiveStream;
	//接收FLV流媒体文件
	FILE* m_recvFile;

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
	CRtmpRecvFlv();
	~CRtmpRecvFlv();
	//设置是否是直播流
	void Rtmp_SetIsLiveStream(bool isLiveStream);
	//设置RTMP中log级别
	void Rtmp_LogSetLevel(RTMP_LogLevel level);
	//设置RTMP中log输出文件
	void Rtmp_LogSetOutput(FILE* logoutfile);
	//FLV输出文件
	void Rtmp_FlvSetOutput(FILE* flvoutfile);
	//设置RTMP连接的URL
	int Rtmp_SetupURL(const char *url);
	//设置RTMP中buffer time
	void Rtmp_SetBufferMS(int size);
	//建立RTMP网络连接NetConnection
	int Rtmp_Connect();
	//建立RTMP网络流NetStream
	int Rtmp_ConnectStream();
	//接收RTMP流媒体服务器数据
	int Rtmp_Read();
};

#endif


