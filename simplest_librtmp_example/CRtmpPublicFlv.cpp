/*************************************************************************
    > File Name: CRtmpPublicFlv.cpp
    > Author: zhongjihao
    > Mail: zhongjihao100@163.com
    > Created Time: 2018年01月08日 星期一 15时10分53秒
 ************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "CRtmpPublicFlv.h"
#include "CNetByteOper.h"


CRtmpPublicFlv::CRtmpPublicFlv()
{
	m_pRtmp = Rtmp_Alloc();
	Rtmp_Init();
    //set connection timeout,default 30s
	m_pRtmp->Link.timeout = 10;
	m_pRtmp->Link.lFlags |= RTMP_LF_LIVE;
}

CRtmpPublicFlv::~CRtmpPublicFlv()
{
	Rtmp_Close();
	Rtmp_Free();
}

void CRtmpPublicFlv::Rtmp_Init()
{
	RTMP_Init(m_pRtmp);
}

RTMP* CRtmpPublicFlv::Rtmp_Alloc()
{
	return RTMP_Alloc();
}

void CRtmpPublicFlv::Rtmp_Free()
{
	if(m_pRtmp)
		RTMP_Free(m_pRtmp);
	m_pRtmp = NULL;
}

void CRtmpPublicFlv::Rtmp_Close()
{
	if(m_pRtmp)
		RTMP_Close(m_pRtmp);
}

void CRtmpPublicFlv::Rtmp_SetBufferMS(int bufferTime)
{
	RTMP_Log(RTMP_LOGDEBUG, "%s: ====haoge=====Setting buffer time to: %dms", __FUNCTION__,bufferTime);
	RTMP_SetBufferMS(m_pRtmp, bufferTime);
}

void CRtmpPublicFlv::Rtmp_LogSetLevel(RTMP_LogLevel level)
{
	RTMP_LogSetLevel(level);
}

void CRtmpPublicFlv::Rtmp_LogSetOutput(FILE* logoutfile)
{
	RTMP_LogSetOutput(logoutfile);
}

int CRtmpPublicFlv::Rtmp_SetupURL(const char *url)
{
	if(!RTMP_SetupURL(m_pRtmp,const_cast<char*>(url)))
	{
		RTMP_Log(RTMP_LOGERROR,"%s: ====haoge=====SetupURL Err",__FUNCTION__);
		Rtmp_Free();
		return RD_NO_CONNECT;
	}
	return RD_SUCCESS;
}

int CRtmpPublicFlv::Rtmp_Connect()
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

int CRtmpPublicFlv::Rtmp_ConnectStream()
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

int CRtmpPublicFlv::Rtmp_publish_using_packet(const char* sourceFlv,const char* destRtmpUrl)
{
	RTMPPacket *packet = NULL;
	int totalByte = 0;
	uint32_t start_time = 0;
	uint32_t now_time = 0;
	//the timestamp of the previous frame
	long pre_frame_time = 0; 
	long lasttime = 0;
	int bNextIsKey = 1;
	uint32_t preTagsize = 0;
	
	//packet attributes
	uint32_t type = 0;          //FLV规范中Tag的类型，占1字节，0x09表示视频、0x08表示音频、0x12表示Script
	uint32_t datalength = 0;    //表示Tag 数据部分的大小，占3字节		
	uint32_t timestamp = 0;     //Tag 中3字节时间戳 和 1字节时间戳扩展字节
	uint32_t streamid = 0;		//Tag 中流ID，占3字节	

	FILE* fp = NULL;
	fp = fopen(sourceFlv,"r");
	if (!fp)
	{
		RTMP_LogPrintf("%s: =====haoge=====Open File Error.\n",__FUNCTION__);
		return RD_FAILED;
	}

	if(Rtmp_SetupURL(destRtmpUrl) != RD_SUCCESS)
		return RD_FAILED;
	
	//if unable,the AMF command would be 'play' instead of 'publish'
	//发布流的时候必须要使用。如果不使用则代表接收流
	RTMP_EnableWrite(m_pRtmp);

	//建立RTMP连接，创建一个RTMP协议规范中的NetConnection
	if(Rtmp_Connect() != RD_SUCCESS)
		return RD_FAILED;

	//创建一个RTMP协议规范中的NetStream
	if(Rtmp_ConnectStream() != RD_SUCCESS)
		return RD_FAILED;

	packet = (RTMPPacket*)malloc(sizeof(RTMPPacket));
	RTMPPacket_Alloc(packet,1024*64);
	RTMPPacket_Reset(packet);

	//给RTMPPacket包头赋值
	packet->m_hasAbsTimestamp = 0;
	//public发布消息的Chunk stream ID
	packet->m_nChannel = 0x04;
	packet->m_nInfoField2 = m_pRtmp->m_stream_id;

	RTMP_LogPrintf("%s: ====haoge===public stream id: %d, Start to send data ...\n",__FUNCTION__,m_pRtmp->m_stream_id);
	
	//jump over FLV Header
	fseek(fp,9,SEEK_SET);	
	//jump over previousTagSizen
	fseek(fp,4,SEEK_CUR);	
	start_time = RTMP_GetTime();

	while(1)
	{
		//发布流过程中的延时，保证按正常播放速度发送数据
		if( ((now_time = RTMP_GetTime()) - start_time) < pre_frame_time )
		{	
			//wait for 1 sec if the send process is too fast
			//this mechanism is not very good,need some improvement
			if(pre_frame_time > lasttime)
			{
				RTMP_LogPrintf("%s: ======haoge=====TimeStamp:%8lu ms\n",__FUNCTION__,pre_frame_time);
				lasttime = pre_frame_time;
			}
			sleep(1);
			continue;
		}

		//not quite the same as FLV spec
		if(!CNetByteOper::ReadU8(&type,fp))            //读取Tag header中Type类型	
			break;
		if(!CNetByteOper::ReadU24(&datalength,fp))     //读取Tag header中Tag Data的大小
			break;
		if(!CNetByteOper::ReadTime(&timestamp,fp))     //读取Tag的时间戳
			break;
		if(!CNetByteOper::ReadU24(&streamid,fp))       //读取stream id
			break;

		//读取完第一个Tag头后，判断当前Tag是否是音频Tag还是视频Tag，如果都不是，则跳过该Tag块，继续读取下一个Tag
		if (type != 0x08 && type != 0x09)
		{
			//jump over non_audio and non_video frame，
			//jump over next previousTagSizen at the same time
			fseek(fp,datalength+4,SEEK_CUR);
			continue;
		}

		//音频帧或视频帧,读取音频数据或视频数据放到RTMPPacket的Body中
		if(fread(packet->m_body,1,datalength,fp) != datalength)
			break;
        //继续给RTMPPacket包头赋值
		packet->m_headerType = RTMP_PACKET_SIZE_LARGE;
		packet->m_nTimeStamp = timestamp;
		packet->m_packetType = type;
		packet->m_nBodySize  = datalength;
		pre_frame_time = timestamp;

		//检查RTMP socket连接是否成功
		if (!RTMP_IsConnected(m_pRtmp))
		{
			RTMP_Log(RTMP_LOGERROR,"%s: ===haoge===rtmp is not connect\n",__FUNCTION__);
			break;
		}
		//发送一个构造好的RTMP数据RTMPPacket
		if (!RTMP_SendPacket(m_pRtmp,packet,0))
		{
			RTMP_Log(RTMP_LOGERROR,"%s: ====haoge====Send Error\n",__FUNCTION__);
			break;
		}
		totalByte += datalength;

		//读取表示该Tag大小的字段
		if(!CNetByteOper::ReadU32(&preTagsize,fp))
			break;

		//判断下一个Tag 类型
		if(!CNetByteOper::PeekU8(&type,fp))
			break;
		//视频Tag
		if(type == 0x09)
		{
			//跳过该Tag头11字节
			if(fseek(fp,11,SEEK_CUR) != 0)
				break;
			//获取视频Tag Data开始的第1个字节，即视频数据的参数信息。第1个字节的前4位的数值表示帧类型，第1个字节的后4位的数值表示视频编码类型
			if(!CNetByteOper::PeekU8(&type,fp)){
				break;
			}
			//keyframe (for AVC，a seekable frame) 即采用H264编码的keyframe帧
			if(type == 0x17)
				bNextIsKey = 1;
			else
				bNextIsKey = 0;
			//将文件指针重新定位到下一个Tag头的开始位置
			fseek(fp,-11,SEEK_CUR);
		}
	}

	RTMP_LogPrintf("%s: ====haoge=======Send total %d Byte Data Over\n",__FUNCTION__,totalByte);

	if(fp)
		fclose(fp);

	if(packet != NULL)
	{
		RTMPPacket_Free(packet);	
		free(packet);
		packet = NULL;
	}

	return totalByte;
}

int CRtmpPublicFlv::Rtmp_publish_using_write(const char* sourceFlv,const char* destRtmpUrl)
{
	uint32_t start_time = 0;
	uint32_t now_time = 0;
	long pre_frame_time = 0;
	uint32_t lasttime = 0;
	int bNextIsKey = 0;
	char* pFileBuf = NULL;
	int totalByte = 0;

	//read from tag header
	uint32_t type = 0;          //FLV规范中Tag的类型，占1字节，0x09表示视频、0x08表示音频、0x12表示Script
	uint32_t datalength = 0;    //表示Tag 数据部分的大小，占3字
	uint32_t timestamp = 0;     //Tag 中3字节时间戳 和 1字节时间戳扩展字节

	FILE* fp = NULL;
	fp = fopen(sourceFlv,"r");
	if (!fp)
	{
		RTMP_LogPrintf("%s: =====haoge=====Open File Error.\n",__FUNCTION__);
		return RD_FAILED;
	}

	if(Rtmp_SetupURL(destRtmpUrl) != RD_SUCCESS)
		return RD_FAILED;

	//if unable,the AMF command would be 'play' instead of 'publish'
	//发布流的时候必须要使用。如果不使用则代表接收流
	RTMP_EnableWrite(m_pRtmp);

	//1 hour
	Rtmp_SetBufferMS(3600*1000);

	//建立RTMP连接，创建一个RTMP协议规范中的NetConnection
	if(Rtmp_Connect() != RD_SUCCESS)
		return RD_FAILED;

	//创建一个RTMP协议规范中的NetStream
	if(Rtmp_ConnectStream() != RD_SUCCESS)
		return RD_FAILED;

	printf("%s: ====haoge====Start to send data ...\n",__FUNCTION__);

    //jump over FLV Header
	fseek(fp,9,SEEK_SET);
	//jump over previousTagSizen
	fseek(fp,4,SEEK_CUR);
	start_time = RTMP_GetTime();

	while(1)
	{
		if((((now_time = RTMP_GetTime()) -start_time) < (pre_frame_time)) )
		{
			//wait for 1 sec if the send process is too fast
			//this mechanism is not very good,need some improvement
			if(pre_frame_time>lasttime)
			{
				RTMP_LogPrintf("%s: =====haoge======TimeStamp:%8lu ms\n",__FUNCTION__,pre_frame_time);
				lasttime = pre_frame_time;
			}

			sleep(1);
			continue;
		}

		//jump over type
		fseek(fp,1,SEEK_CUR);
		if(!CNetByteOper::ReadU24(&datalength,fp))
			break;
		if(!CNetByteOper::ReadTime(&timestamp,fp))
			break;
		//jump back
		fseek(fp,-8,SEEK_CUR);

		pFileBuf = (char*)malloc(11+datalength+4);
		memset(pFileBuf,0,11+datalength+4);
		if(fread(pFileBuf,1,11 + datalength + 4,fp) != (11 + datalength + 4))
			break;

		pre_frame_time = timestamp;

		if (!RTMP_IsConnected(m_pRtmp))
		{
			RTMP_Log(RTMP_LOGERROR,"%s: ====haoge======rtmp is not connect\n",__FUNCTION__);
			break;
		}
		int sendByte = RTMP_Write(m_pRtmp,pFileBuf,11+datalength+4);
		totalByte = totalByte + (11+4+datalength);
		RTMP_Log(RTMP_LOGERROR,"%s: ====haoge======send %d Byte, TagDataSize: %d Byte , totalByte: %d Byte\n",__FUNCTION__,sendByte,15+datalength,totalByte);
		if (!sendByte/*RTMP_Write(m_pRtmp,pFileBuf,11+datalength+4)*/)
		{
			RTMP_Log(RTMP_LOGERROR,"%s: ======haoge====Rtmp Write Error\n",__FUNCTION__);
			break;
		}

		free(pFileBuf);
		pFileBuf = NULL;

		if(!CNetByteOper::PeekU8(&type,fp))
			break;

		if(type == 0x09)
		{
			if(fseek(fp,11,SEEK_CUR) != 0)
				break;
			if(!CNetByteOper::PeekU8(&type,fp)){
				break;
			}
			if(type == 0x17)
				bNextIsKey = 1;
			else
				bNextIsKey = 0;
			fseek(fp,-11,SEEK_CUR);
		}
	}

	RTMP_LogPrintf("%s: ====haoge=====Send %d Byte Data Over\n",__FUNCTION__,totalByte);

	if(fp)
		fclose(fp);

	if(pFileBuf)
	{
		free(pFileBuf);
		pFileBuf = NULL;
	}

	return totalByte;
}


