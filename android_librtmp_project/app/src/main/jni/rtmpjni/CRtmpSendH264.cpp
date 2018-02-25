/*************************************************************************
    > File Name: CRtmpSendH264.cpp
    > Author: zhongjihao
    > Mail: zhongjihao100@163.com 
    > Created Time: 2018年02月09日 星期五 16时14分30秒
 ************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "CRtmpSendH264.h"

CRtmpSendH264::CRtmpSendH264()
{

}

CRtmpSendH264::~CRtmpSendH264()
{

}

/**
 * 初始化并连接到RTMP服务器
 * @param url 服务器上对应webapp的地址
 * @param logfile log文件
 * @成功则返回 1 , 失败则返回 0
 */
int CRtmpSendH264::RTMPH264_Connect(const char* url,FILE* logfile)
{
	m_pRtmp = RTMP_Alloc();
	RTMP_Init(m_pRtmp);

	RTMP_LogSetLevel(RTMP_LOGALL);
	RTMP_LogSetOutput(logfile);

	/*设置URL*/
	if (RTMP_SetupURL(m_pRtmp,(char*)url) == FALSE)
	{
		RTMP_Free(m_pRtmp);
		m_pRtmp = NULL;
		return false;
	}

	/*设置可写,即发布流,这个函数必须在连接前使用,否则无效*/
	RTMP_EnableWrite(m_pRtmp);

	/*连接服务器*/
	if (RTMP_Connect(m_pRtmp, NULL) == FALSE) 
	{
		RTMP_Free(m_pRtmp);
		m_pRtmp = NULL;
		return false;
	} 

	/*连接流*/
	if (RTMP_ConnectStream(m_pRtmp,0) == FALSE)
	{
		RTMP_Close(m_pRtmp);
		RTMP_Free(m_pRtmp);
		m_pRtmp = NULL;
		return false;
	}
	return true;
}

/**
 * 发送RTMP数据包
 * @param nPacketType 数据类型
 * @param data 存储数据内容
 * @param size 数据大小
 * @param nTimestamp 当前包的时间戳
 * @成功则返回 1 , 失败则返回 0
 */
int CRtmpSendH264::SendPacket(unsigned int nPacketType,unsigned char* data,unsigned int size,unsigned int nTimestamp)
{
	RTMPPacket* packet;
	/*分配包内存和初始化,len为包体长度*/
	packet = (RTMPPacket *)malloc(RTMP_HEAD_SIZE+size);
	memset(packet,0,RTMP_HEAD_SIZE);
	/*包体内存*/
	packet->m_body = (char *)packet + RTMP_HEAD_SIZE;
	packet->m_nBodySize = size;
	memcpy(packet->m_body,data,size);
	packet->m_hasAbsTimestamp = 0;
	packet->m_packetType = nPacketType; /*此处为类型有两种一种是音频,一种是视频*/
	packet->m_nInfoField2 = m_pRtmp->m_stream_id;
	packet->m_nChannel = 0x04;

	packet->m_headerType = RTMP_PACKET_SIZE_LARGE;
	if(RTMP_PACKET_TYPE_AUDIO == nPacketType && size != 4)
	{
		packet->m_headerType = RTMP_PACKET_SIZE_MEDIUM;
	}

	packet->m_nTimeStamp = nTimestamp;
	/*发送*/
	int nRet = 0;
	if(RTMP_IsConnected(m_pRtmp))
	{
		nRet = RTMP_SendPacket(m_pRtmp,packet,TRUE); /*TRUE为放进发送队列,FALSE是不放进发送队列,直接发送*/
	}

	/*释放内存*/
	free(packet);

	return nRet;
}

/**
 * 发送H264数据帧
 * @param data 存储数据帧内容
 * @param size 数据帧的大小
 * @param bIsKeyFrame 记录该帧是否为关键帧
 * @param nTimeStamp 当前帧的时间戳
 * @成功则返回 1 , 失败则返回 0
 */
int CRtmpSendH264::SendH264Packet(unsigned char* data,unsigned int size,int bIsKeyFrame,unsigned int nTimeStamp)
{
	if(data == NULL && size < 11)
		return false;

    unsigned char* body = (unsigned char*)malloc(size+9);
	memset(body,0,size+9);

	int i = 0;
	if(bIsKeyFrame)
	{
		body[i++] = 0x17;// 1:Iframe  7:AVC   
		body[i++] = 0x01;// AVC NALU   
		body[i++] = 0x00;  
		body[i++] = 0x00;  
		body[i++] = 0x00;

		// NALU size   
		body[i++] = size>>24 & 0xff;  
		body[i++] = size>>16 & 0xff;  
		body[i++] = size>>8 & 0xff;  
		body[i++] = size & 0xff;
		// NALU data
		memcpy(&body[i],data,size);
	}
	else
	{
		body[i++] = 0x27;// 2:Pframe  7:AVC
		body[i++] = 0x01;// AVC NALU
		body[i++] = 0x00;
		body[i++] = 0x00;  
		body[i++] = 0x00;

		// NALU size
		body[i++] = size>>24 & 0xff;
		body[i++] = size>>16 & 0xff;
		body[i++] = size>>8 & 0xff;
		body[i++] = size & 0xff;
		// NALU data
		memcpy(&body[i],data,size);
	}

	int bRet = SendPacket(RTMP_PACKET_TYPE_VIDEO,body,i+size,nTimeStamp);
	free(body);

	return bRet;
}

/**
 * 发送视频的sps和pps信息
 * @param pps 存储视频的pps信息
 * @param pps_len 视频的pps信息长度
 * @param sps 存储视频的sps信息
 * @param sps_len 视频的sps信息长度
 * @param time 序列参数集时间
 * @成功则返回 1 , 失败则返回 0
 */
int CRtmpSendH264::SendVideoSpsPps(unsigned char* pps,int pps_len,unsigned char* sps,int sps_len)
{
	RTMPPacket * packet = NULL;//rtmp包结构
	unsigned char * body = NULL;
	int i;
	packet = (RTMPPacket *)malloc(RTMP_HEAD_SIZE+1024);
	//RTMPPacket_Reset(packet);//重置packet状态
	memset(packet,0,RTMP_HEAD_SIZE+1024);
	packet->m_body = (char *)packet + RTMP_HEAD_SIZE;
	body = (unsigned char *)packet->m_body;
	i = 0;
	body[i++] = 0x17;
	body[i++] = 0x00;

	body[i++] = 0x00;
	body[i++] = 0x00;
	body[i++] = 0x00;

	/*AVCDecoderConfigurationRecord*/
	body[i++] = 0x01;
	body[i++] = sps[1];
	body[i++] = sps[2];
	body[i++] = sps[3];
	body[i++] = 0xff;

	/*sps*/
	body[i++] = 0xe1;
	body[i++] = (sps_len >> 8) & 0xff;
	body[i++] = sps_len & 0xff;
	//拷贝sps数据
	memcpy(&body[i],sps,sps_len);
	i +=  sps_len;

	/*pps*/
	body[i++] = 0x01;
	body[i++] = (pps_len >> 8) & 0xff;
	body[i++] = pps_len & 0xff;

	//拷贝pps数据
	memcpy(&body[i],pps,pps_len);
	i +=  pps_len;

    /*
	 * typedef struct RTMPPacket{
	 *     uint8_t  m_headerType;
	 *     uint8_t  m_packetType;
	 *     uint8_t  m_hasAbsTimestamp;
	 *     int      m_nChannel;
	 *     uint32_t m_nTimeStamp;
	 *     int32_t  m_nInfoField2;
	 *     uint32_t m_nBodySize;
	 *     uint32_t m_nBytesRead;
	 *     RTMPChunk *m_chunk;
	 *     char    *m_body;
	 * }RTMPPacket;
	 *   
	 *   packet->m_headerType： 可以定义如下
	 *   #define RTMP_PACKET_SIZE_LARGE    0
	 *   #define RTMP_PACKET_SIZE_MEDIUM   1
	 *   #define RTMP_PACKET_SIZE_SMALL    2
	 *   #define RTMP_PACKET_SIZE_MINIMUM  3
	 *   一般定位为 RTMP_PACKET_SIZE_MEDIUM
	 *
	 *   packet->m_packetType： 音频、视频包类型
	 *   #define RTMP_PACKET_TYPE_AUDIO    0x08
	 *   #define RTMP_PACKET_TYPE_VIDEO    0x09
	 *   #define RTMP_PACKET_TYPE_INFO     0x12
	 *   还有其他更多类型，但一般都是 音频或视频
	 *
	 *   packet->m_hasAbsTimestamp： 是否使用绝对时间戳，一般定义为0
	 *
	 *   packet->m_nChannel：0x04代表source channel (invoke)
	 *
	 *   packet->m_nTimeStamp：时间戳
	 *   一般视频时间戳可以从0开始计算，每帧时间戳 + 1000/fps (25fps每帧递增25；30fps递增33)
	 *   音频时间戳也可以从0开始计算，48K采样每帧递增21；44.1K采样每帧递增23
	 *
	 *   packet->m_nInfoField2 = rtmp->m_stream_id
	 *
	 *   packet->m_nBodySize：RTMPPacket包体长度
	 *
	 *   packet->m_nBytesRead：不用管
	 *   packet->m_chunk： 不用管
	 *
	 *   packet->m_body：RTMPPacket包体数据，其长度为packet->m_nBodySize。
	*/

	packet->m_packetType = RTMP_PACKET_TYPE_VIDEO;
	packet->m_nBodySize = i;
	packet->m_nChannel = 0x04;
	packet->m_nTimeStamp = 0;
	packet->m_hasAbsTimestamp = 0;
	packet->m_headerType = RTMP_PACKET_SIZE_MEDIUM;
	packet->m_nInfoField2 = m_pRtmp->m_stream_id;
	/*调用发送接口*/
	int nRet = RTMP_SendPacket(m_pRtmp,packet,TRUE);
	free(packet);    //释放内存
	return nRet;
}

/**
 * 断开连接，释放相关的资源
 */
void CRtmpSendH264::RTMPH264_Close()
{
	if(m_pRtmp)  
	{
		RTMP_Close(m_pRtmp);
		RTMP_Free(m_pRtmp);
		m_pRtmp = NULL;
	}
}


