/*************************************************************************
    > File Name: CRtmpSendH264.h
    > Author: zhongjihao
    > Mail: zhongjihao100@163.com 
    > Created Time: 2018年01月15日 星期一 16时14分30秒
 ************************************************************************/

#ifndef CRTMP_SEND_H264_H
#define CRTMP_SEND_H264_H

#include "libRTMP/rtmpdump/librtmp/rtmp_sys.h"
#include "libRTMP/rtmpdump/librtmp/log.h"
#include "libRTMP/rtmpdump/librtmp/rtmp.h"
#include "CNetByteOper.h"

//定义包头长度，RTMP_MAX_HEADER_SIZE=18
#define RTMP_HEAD_SIZE   (sizeof(RTMPPacket) + RTMP_MAX_HEADER_SIZE)

//存储Nal单元数据的buffer大小
#define BUFFER_SIZE 32768

//搜寻Nal单元时的一些标志
#define GOT_A_NAL_CROSS_BUFFER         BUFFER_SIZE+1
#define GOT_A_NAL_INCLUDE_A_BUFFER     BUFFER_SIZE+2
#define NO_MORE_BUFFER_TO_READ         BUFFER_SIZE+3


/**
 * _NaluUnit
 * 内部结构体。该结构体主要用于存储和传递Nal单元的类型、大小和数据
 */ 
typedef struct _NaluUnit  
{
	int type;
	int size;
	unsigned char *data;
}NaluUnit;

/**
 * _RTMPMetadata
 * 内部结构体。该结构体主要用于存储和传递元数据信息
 */ 
typedef struct _RTMPMetadata  
{
	// video, must be h264 type
	unsigned int    nWidth;
	unsigned int    nHeight;
	unsigned int    nFrameRate;
	unsigned int    nSpsLen;
	unsigned char*  Sps;
	unsigned int    nPpsLen;
	unsigned char*  Pps;
}RTMPMetadata,*LPRTMPMetadata;

enum
{
	//JPEG (currently unused)
	VIDEO_CODECID_JPEG = 1,
    //Sorenson H.263
	VIDEO_CODECID_H263 = 2,
	//Screen video
	VIDEO_CODECID_SCREEN_VIDEO = 3,
	//On2 VP6
	VIDEO_CODECID_ON2_VP6 = 4,
	//On2 VP6 with alpha channel
	VIDEO_CODECID_ON2_VP6_ALPHA = 5,
	//Screen video version 2
	VIDEO_CODECID_SCREEN_VIDEO2 = 6,
	//AVC，即H264编码
	VIDEO_CODECID_H264 = 7,
};

//本类用于将内存中的H.264数据推送至RTMP流媒体服务器
class CRtmpSendH264
{
private:
	unsigned int m_nFileBufSize;     //从文件读取数据存放到内存的缓冲大小
	unsigned int nalhead_pos;        //记录每次寻找NALU起始码的开始位置
	RTMP* m_pRtmp;                   //RTMP协议对象
	RTMPMetadata metaData;           //h264视频数据封装
	unsigned char* m_pFileBuf;       //从文件读取数据存放的缓冲
	unsigned char* m_pFileBuf_tmp;   //存放寻找到的NALU
	unsigned char* m_pFileBuf_tmp_old; //used for realloc

private:
	/**
	 * H264的NAL起始码防竞争机制
	 * @param buf SPS数据内容
	 * @无返回值
	*/
	void de_emulation_prevention(BYTE* buf,unsigned int* buf_size);

	/**
	 * 解码SPS,获取视频图像宽、高信息
	 * @param buf SPS数据内容
	 * @param nLen SPS数据的长度
	 * @param width 图像宽度
	 * @param height 图像高度
	 * @成功则返回 1 , 失败则返回 0
	 */
	int h264_decode_sps(BYTE* buf,unsigned int nLen,int& width,int& height,int& fps);

	/**
	 * 发送RTMP数据包
	 * @param nPacketType 数据类型
	 * @param data 存储数据内容
	 * @param size 数据大小
	 * @param nTimestamp 当前包的时间戳
	 * @成功则返回 1 , 失败则返回 0
	*/
	int SendPacket(unsigned int nPacketType,unsigned char* data,unsigned int size,unsigned int nTimestamp);

	/**
	 * 发送H264数据帧
	 * @param data 存储数据帧内容
	 * @param size 数据帧的大小
	 * @param bIsKeyFrame 记录该帧是否为关键帧
	 * @param nTimeStamp 当前帧的时间戳
	 * @成功则返回 1 , 失败则返回 0
	*/
	int SendH264Packet(unsigned char* data,unsigned int size,int bIsKeyFrame,unsigned int nTimeStamp);

	/**
	 * 发送视频的sps和pps信息
	 * @param pps 存储视频的pps信息
	 * @param pps_len 视频的pps信息长度
	 * @param sps 存储视频的sps信息
	 * @param sps_len 视频的sps信息长度
	 * @成功则返回 1 , 失败则返回 0
	*/
	int SendVideoSpsPps(unsigned char* pps,int pps_len,unsigned char* sps,int sps_len);

    /**
	 * 从内存中读取出第一个Nal单元
	 * @param nalu 存储nalu数据
	 *
	 * @param read_buffer 回调函数，当数据不足的时候，系统会自动调用该函数获取输入数据.
	 * 2个参数功能：
	 * uint8_t *buf：外部数据送至该地址
	 * int buf_size：外部数据大小
	 * 返回值：成功读取的内存大小
	 * @成功则返回 1 , 失败则返回 0
	*/
	int ReadFirstNaluFromBuf(NaluUnit& nalu,int (*read_buffer)(uint8_t *buf, int buf_size));

	/**
	 * 从内存中读取出一个Nal单元
	 * @param nalu 存储nalu数据
	 * @param read_buffer 回调函数，当数据不足的时候，系统会自动调用该函数获取输入数据.
	 * 2个参数功能：
	 * uint8_t* buf：外部数据送至该地址
	 * int buf_size：外部数据大小
	 * 返回值：成功读取的内存大小
	 * @成功则返回 1 , 失败则返回 0
	*/
	int ReadOneNaluFromBuf(NaluUnit& nalu,int (*read_buffer)(uint8_t* buf, int buf_size));

public:
	CRtmpSendH264();
	~CRtmpSendH264();
	/**
	 * 初始化并连接到RTMP服务器
	 * @param level log级别
	 * @param url 服务器上对应webapp的地址
	 * @param logfile log文件
	 * @成功则返回 1 , 失败则返回 0
	*/
	int RTMPH264_Connect(RTMP_LogLevel level,const char* url,FILE* logfile);

	/**
	 * 将内存中的一段H.264编码的视频数据利用RTMP协议发送到服务器
	 * @param read_buffer 回调函数，当数据不足的时候，系统会自动调用该函数获取输入数据.

	 * 2个参数功能:
	 * uint8_t *buf：外部数据送至该地址
	 * int buf_size：外部数据大小
	 * 返回值：成功读取的内存大小
	 * @成功则返回 1 , 失败则返回 0
	 */
	int RTMPH264_Send(int (*read_buffer)(unsigned char* buf, int buf_size));

	/**
	 * 断开连接，释放相关的资源
	*/
	void RTMPH264_Close();
};

#endif


