/*************************************************************************
    > File Name: CRtmpSendH264.cpp
    > Author: zhongjihao
    > Mail: zhongjihao100@163.com 
    > Created Time: 2018年01月15日 星期一 16时14分30秒
 ************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include "CRtmpSendH264.h"

CRtmpSendH264::CRtmpSendH264()
{

}

CRtmpSendH264::~CRtmpSendH264()
{

}

/**
* H264的NAL起始码防竞争机制
* @param buf SPS数据内容
* @无返回值
*/
void CRtmpSendH264::de_emulation_prevention(BYTE* buf,unsigned int* buf_size)
{
	int i = 0,j = 0;
	BYTE* tmp_ptr = NULL;
	unsigned int tmp_buf_size = 0;
	int val = 0;

	tmp_ptr = buf;
	tmp_buf_size = *buf_size;
	for(i = 0; i < (tmp_buf_size-2); i++)
	{
		//check for 0x000003
		val = (tmp_ptr[i]^0x00) + (tmp_ptr[i+1]^0x00) + (tmp_ptr[i+2]^0x03);
		if(val == 0)
		{
			//kick out 0x03
			//剔除刚找到的0x03,即将0x03后面的数据往前移动一个字节
			for(j=i+2;j < tmp_buf_size-1;j++)
				tmp_ptr[j] = tmp_ptr[j+1];

			//and so we should devrease bufsize
			(*buf_size)--;
		}
	}
	return;
}

/**
* 解码SPS,获取视频图像宽、高信息和帧率
* @param buf SPS数据内容
* @param nLen SPS数据的长度
* @param width 图像宽度
* @param height 图像高度
* @成功则返回 1 , 失败则返回 0
*/
int CRtmpSendH264::h264_decode_sps(BYTE* buf,unsigned int nLen,int& width,int& height,int& fps)
{
	UINT StartBit = 0; 
	fps = 0;
	de_emulation_prevention(buf,&nLen);

	//提取该NALU header字段
	int forbidden_zero_bit = CNetByteOper::u(1,buf,StartBit);
	int nal_ref_idc = CNetByteOper::u(2,buf,StartBit);
	int nal_unit_type = CNetByteOper::u(5,buf,StartBit);
	//该nalu为序列参数集SPS帧
	if(nal_unit_type == 7)
	{
		int profile_idc = CNetByteOper::u(8,buf,StartBit);
		int constraint_set0_flag = CNetByteOper::u(1,buf,StartBit);//(buf[1] & 0x80)>>7;
		int constraint_set1_flag = CNetByteOper::u(1,buf,StartBit);//(buf[1] & 0x40)>>6;
		int constraint_set2_flag = CNetByteOper::u(1,buf,StartBit);//(buf[1] & 0x20)>>5;
		int constraint_set3_flag = CNetByteOper::u(1,buf,StartBit);//(buf[1] & 0x10)>>4;
		int reserved_zero_4bits = CNetByteOper::u(4,buf,StartBit);
		int level_idc = CNetByteOper::u(8,buf,StartBit);

		int seq_parameter_set_id = CNetByteOper::Ue(buf,nLen,StartBit);
		if( profile_idc == 100 || profile_idc == 110 || profile_idc == 122 || profile_idc == 144 )
		{
			int chroma_format_idc = CNetByteOper::Ue(buf,nLen,StartBit);
			if( chroma_format_idc == 3 )
				int residual_colour_transform_flag = CNetByteOper::u(1,buf,StartBit);
			int bit_depth_luma_minus8 = CNetByteOper::Ue(buf,nLen,StartBit);
			int bit_depth_chroma_minus8 = CNetByteOper::Ue(buf,nLen,StartBit);
			int qpprime_y_zero_transform_bypass_flag = CNetByteOper::u(1,buf,StartBit);
			int seq_scaling_matrix_present_flag = CNetByteOper::u(1,buf,StartBit);

			int seq_scaling_list_present_flag[8];
			if( seq_scaling_matrix_present_flag )
			{
				for( int i = 0; i < 8; i++ ) {
					seq_scaling_list_present_flag[i] = CNetByteOper::u(1,buf,StartBit);
				}
			}
		}

		int log2_max_frame_num_minus4 = CNetByteOper::Ue(buf,nLen,StartBit);
		int pic_order_cnt_type = CNetByteOper::Ue(buf,nLen,StartBit);
		if( pic_order_cnt_type == 0 )
			int log2_max_pic_order_cnt_lsb_minus4 = CNetByteOper::Ue(buf,nLen,StartBit);
		else if( pic_order_cnt_type == 1 )
		{
			int delta_pic_order_always_zero_flag = CNetByteOper::u(1,buf,StartBit);
			int offset_for_non_ref_pic = CNetByteOper::Se(buf,nLen,StartBit);
			int offset_for_top_to_bottom_field = CNetByteOper::Se(buf,nLen,StartBit);
			int num_ref_frames_in_pic_order_cnt_cycle = CNetByteOper::Ue(buf,nLen,StartBit);

			int *offset_for_ref_frame = new int[num_ref_frames_in_pic_order_cnt_cycle];
			for( int i = 0; i < num_ref_frames_in_pic_order_cnt_cycle; i++ )
				offset_for_ref_frame[i] = CNetByteOper::Se(buf,nLen,StartBit);
			delete [] offset_for_ref_frame;
		}

		int num_ref_frames = CNetByteOper::Ue(buf,nLen,StartBit);
		int gaps_in_frame_num_value_allowed_flag = CNetByteOper::u(1,buf,StartBit);
		int pic_width_in_mbs_minus1 = CNetByteOper::Ue(buf,nLen,StartBit);
		int pic_height_in_map_units_minus1 = CNetByteOper::Ue(buf,nLen,StartBit);
		
		width = (pic_width_in_mbs_minus1+1)*16;
		height = (pic_height_in_map_units_minus1+1)*16;

		int frame_mbs_only_flag = CNetByteOper::u(1,buf,StartBit);
		if(!frame_mbs_only_flag)
			int mb_adaptive_frame_field_flag = CNetByteOper::u(1,buf,StartBit);

		int direct_8x8_inference_flag = CNetByteOper::u(1,buf,StartBit);
		int frame_cropping_flag = CNetByteOper::u(1,buf,StartBit);
		if(frame_cropping_flag)
		{
			int frame_crop_left_offset = CNetByteOper::Ue(buf,nLen,StartBit);
			int frame_crop_right_offset = CNetByteOper::Ue(buf,nLen,StartBit);
			int frame_crop_top_offset = CNetByteOper::Ue(buf,nLen,StartBit);
			int frame_crop_bottom_offset = CNetByteOper::Ue(buf,nLen,StartBit);
		}
		int vui_parameter_present_flag = CNetByteOper::u(1,buf,StartBit);

		if(vui_parameter_present_flag)
		{
			int aspect_ratio_info_present_flag = CNetByteOper::u(1,buf,StartBit);              
			if(aspect_ratio_info_present_flag)
			{
				int aspect_ratio_idc = CNetByteOper::u(8,buf,StartBit);   
				if(aspect_ratio_idc == 255)
				{
					int sar_width = CNetByteOper::u(16,buf,StartBit);                                  
					int sar_height = CNetByteOper::u(16,buf,StartBit);                                      
				}
			}
			int overscan_info_present_flag = CNetByteOper::u(1,buf,StartBit); 
			if(overscan_info_present_flag)
				int overscan_appropriate_flagu = CNetByteOper::u(1,buf,StartBit);                   
			int video_signal_type_present_flag = CNetByteOper::u(1,buf,StartBit);
            if(video_signal_type_present_flag)
			{
				int video_format = CNetByteOper::u(3,buf,StartBit);                         
				int video_full_range_flag = CNetByteOper::u(1,buf,StartBit);                       
				int colour_description_present_flag = CNetByteOper::u(1,buf,StartBit);
				if(colour_description_present_flag)
				{
					int colour_primaries = CNetByteOper::u(8,buf,StartBit);              
					int transfer_characteristics = CNetByteOper::u(8,buf,StartBit);                     
					int matrix_coefficients = CNetByteOper::u(8,buf,StartBit);                  		
				}
			}

            int chroma_loc_info_present_flag = CNetByteOper::u(1,buf,StartBit);  
			if(chroma_loc_info_present_flag)
			{
				int chroma_sample_loc_type_top_field = CNetByteOper::Ue(buf,nLen,StartBit);             
				int chroma_sample_loc_type_bottom_field = CNetByteOper::Ue(buf,nLen,StartBit);       
			}

			/*
			 * timing_info_present_flag等于1表示num_units_in_tick，time_scale和fixed_frame_rate_flag在比特流中存在。
			 * timing_info_present_flag等于0表示num_units_in_tick，time_scale和fixed_frame_rate_flag在比特流中不存在。
			 * 因此，当timing_info_present_flag等于0时，无法得到码率，可据此设置一个默认帧率。
			*/
			int timing_info_present_flag = CNetByteOper::u(1,buf,StartBit);      
			if(timing_info_present_flag)
			{
				int num_units_in_tick = CNetByteOper::u(32,buf,StartBit);                              
				int time_scale = CNetByteOper::u(32,buf,StartBit);    
			//	fps = time_scale/(2*num_units_in_tick);
			    fps = time_scale/num_units_in_tick;  
                int fixed_frame_rate_flag = CNetByteOper::u(1,buf,StartBit);
				//fixed_frame_rate_flag=1时fps才要除以2
                if(fixed_frame_rate_flag)
                {
					fps = fps/2;
                }
			}
		}
		return true;
	}
	else
		return false;
}

/**
 * 初始化并连接到RTMP服务器
 * @param level log级别
 * @param url 服务器上对应webapp的地址
 * @param logfile log文件
 * @成功则返回 1 , 失败则返回 0
 */
int CRtmpSendH264::RTMPH264_Connect(RTMP_LogLevel level,const char* url,FILE* logfile)
{
	nalhead_pos = 0;
	m_nFileBufSize = BUFFER_SIZE;
	m_pFileBuf = (unsigned char*)malloc(BUFFER_SIZE);
	m_pFileBuf_tmp = (unsigned char*)malloc(BUFFER_SIZE);

	m_pRtmp = RTMP_Alloc();
	RTMP_Init(m_pRtmp);

	RTMP_LogSetLevel(level);
	RTMP_LogSetOutput(logfile);

	/*设置URL*/
	if (RTMP_SetupURL(m_pRtmp,(char*)url) == FALSE)
	{
		RTMP_Free(m_pRtmp);
		return false;
	}

	/*设置可写,即发布流,这个函数必须在连接前使用,否则无效*/
	RTMP_EnableWrite(m_pRtmp);

	/*连接服务器*/
	if (RTMP_Connect(m_pRtmp, NULL) == FALSE) 
	{
		RTMP_Free(m_pRtmp);
		return false;
	} 

	/*连接流*/
	if (RTMP_ConnectStream(m_pRtmp,0) == FALSE)
	{
		RTMP_Close(m_pRtmp);
		RTMP_Free(m_pRtmp);
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
 * 将内存中的一段H.264编码的视频数据利用RTMP协议发送到服务器
 * @param read_buffer 回调函数，当数据不足的时候，系统会自动调用该函数获取输入数据.
 *
 * 2个参数功能:
 * uint8_t *buf：外部数据送至该地址
 * int buf_size：外部数据大小
 * 返回值：成功读取的内存大小
 * @成功则返回 1 , 失败则返回 0
 */
int CRtmpSendH264::RTMPH264_Send(int (*read_buffer)(unsigned char* buf, int buf_size))
{
	int ret;
	uint32_t now,last_update;

	memset(&metaData,0,sizeof(RTMPMetadata));
	memset(m_pFileBuf,0,BUFFER_SIZE);
	//从文件中读取ret个字节数保存到m_pFileBuf中
	if((ret = read_buffer(m_pFileBuf,m_nFileBufSize)) < 0)
	{
		return FALSE;
	}
 
	NaluUnit naluUnit;
	/*
	 * 先读取h264文件的SPS和PPS。SPS是序列参数集，PPS是图像参数集。在SPS序列参数集中可以解析出图像的宽，高和帧率等信息。
	 * 而在h264文件中，最开始的两帧数据就是SPS和PPS，这个h264文件只存在一个SPS帧和一个PPS帧。
	 * RTMP 传输的时候，它需要在每次发送H264的I帧之前，发送SPS序列参数集帧和PPS图像参数集帧。
	 * 在这里的处理方式是，先提取出SPS和PPS帧，然后保存起来，然后每次发送I帧之前都发送一次SPS和PPS帧
	 */
	//读取SPS帧
	ReadFirstNaluFromBuf(naluUnit,read_buffer);
	metaData.nSpsLen = naluUnit.size;  
	metaData.Sps = (unsigned char*)malloc(naluUnit.size);
	memcpy(metaData.Sps,naluUnit.data,naluUnit.size);

	//读取PPS帧   
	ReadOneNaluFromBuf(naluUnit,read_buffer);  
	metaData.nPpsLen = naluUnit.size; 
	metaData.Pps = (unsigned char*)malloc(naluUnit.size);
	memcpy(metaData.Pps,naluUnit.data,naluUnit.size);
	
	// 解码SPS,获取视频图像宽、高信息   
	int width = 0,height = 0, fps = 0;  
	h264_decode_sps(metaData.Sps,metaData.nSpsLen,width,height,fps);  
	metaData.nWidth = width;  
	metaData.nHeight = height;  
	if(fps)
		metaData.nFrameRate = fps; 
	else
		metaData.nFrameRate = 25;
	RTMP_Log(RTMP_LOGDEBUG, "%s: ====haoge====vedio width: %d, height: %d, frameRate: %d", __FUNCTION__,width,height,fps);
	
	//发送PPS,SPS
	//ret = SendVideoSpsPps(metaData.Pps,metaData.nPpsLen,metaData.Sps,metaData.nSpsLen);
	//if(ret != 1)
	//	return FALSE;

	unsigned int tick = 0;  
	unsigned int tick_gap = 1000/metaData.nFrameRate; 
	ReadOneNaluFromBuf(naluUnit,read_buffer);
	//该NALU帧为IDR图像的片
	int bKeyframe  = (naluUnit.type == 0x05) ? TRUE : FALSE;
	while(SendH264Packet(naluUnit.data,naluUnit.size,bKeyframe,tick))
	{
got_sps_pps:
		RTMP_Log(RTMP_LOGDEBUG,"%s: ======haoge=======NALU size: %8d,  tick: %d,  tick_gap: %d, 发送间隔: %08d",__FUNCTION__,naluUnit.size,tick,tick_gap,tick_gap - now + last_update);
		last_update = RTMP_GetTime();
		if(!ReadOneNaluFromBuf(naluUnit,read_buffer))
			goto end;
		//如果刚取得的NALU帧为SPS即序列参数集或PPS即图像参数集,则继续取下一个NALU
		if(naluUnit.type == 0x07 || naluUnit.type == 0x08)
			goto got_sps_pps;
		bKeyframe  = (naluUnit.type == 0x05) ? TRUE : FALSE;
		tick += tick_gap;
		now = RTMP_GetTime();
		msleep(tick_gap);
	//	msleep(tick_gap - now + last_update);
	}
end:
	free(metaData.Sps);
	free(metaData.Pps);
	return TRUE;
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
		//关键帧，则在发送该帧之前先发送SPS和PPS
		SendVideoSpsPps(metaData.Pps,metaData.nPpsLen,metaData.Sps,metaData.nSpsLen);
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
int CRtmpSendH264::ReadFirstNaluFromBuf(NaluUnit& nalu,int (*read_buffer)(uint8_t *buf, int buf_size))
{
	int naltail_pos = nalhead_pos;
	memset(m_pFileBuf_tmp,0,BUFFER_SIZE);
	while(nalhead_pos < m_nFileBufSize)  
	{ 
		//search for nal header
		if(m_pFileBuf[nalhead_pos++] == 0x00 && m_pFileBuf[nalhead_pos++] == 0x00) 
		{
			if(m_pFileBuf[nalhead_pos++] == 0x01)
				goto gotnal_head;
			else
			{
				//cuz we have done an i++ before,so we need to roll back now
				nalhead_pos--;		
				if(m_pFileBuf[nalhead_pos++] == 0x00 && m_pFileBuf[nalhead_pos++] == 0x01)
					goto gotnal_head;
				else
					continue;
			}
		}
		else
			continue;

		//search for nal tail which is also the head of next nal
gotnal_head:
		//normal case:the whole nal is in this m_pFileBuf
		naltail_pos = nalhead_pos;  
		while (naltail_pos < m_nFileBufSize)  
		{  
			if(m_pFileBuf[naltail_pos++] == 0x00 && m_pFileBuf[naltail_pos++] == 0x00)
			{  
				if(m_pFileBuf[naltail_pos++] == 0x01)
				{
					//naltail_pos-3表示找到的next nalu的起始码的开始位置,nalhead_pos表示先前第一个nalu的开始位置,两者之差即为先前第一个nalu的大小
					nalu.size = (naltail_pos-3) - nalhead_pos;
					break;
				}
				else
				{
					naltail_pos--;
					if(m_pFileBuf[naltail_pos++] == 0x00 && m_pFileBuf[naltail_pos++] == 0x01)
					{
						//naltail_pos-4表示找到的next nalu的起始码的开始位置,nalhead_pos表示先前第一个nalu的开始位置,两者之差即为先前第一个nalu的大小
						nalu.size = (naltail_pos-4) - nalhead_pos;
						break;
					}
				}
			}  
		}

		//nalu header占一个字节，其中type字段占header的后5 bit
		nalu.type = m_pFileBuf[nalhead_pos] & 0x1f; 
		memcpy(m_pFileBuf_tmp,m_pFileBuf+nalhead_pos,nalu.size);
		nalu.data = m_pFileBuf_tmp;
		//此时naltail_pos为m_pFileBuf中第二个nalu的开始位置，重新记录寻找下一个nalu的StartCode的开始位置
		nalhead_pos = naltail_pos;
		return TRUE;
	}
}

/**
 * 从内存中读取出一个Nal单元
 * @param nalu 存储nalu数据
 * @param read_buffer 回调函数，当数据不足的时候，系统会自动调用该函数获取输入数据.
 *
 * 2个参数功能：
 * uint8_t* buf：外部数据送至该地址
 * int buf_size：外部数据大小
 * 返回值：成功读取的内存大小
 * @成功则返回 1 , 失败则返回 0
 */
int CRtmpSendH264::ReadOneNaluFromBuf(NaluUnit& nalu,int (*read_buffer)(uint8_t* buf, int buf_size))
{
	int naltail_pos = nalhead_pos;
	int ret;
	int nalustart;//nalu的开始标识符是几个00
	memset(m_pFileBuf_tmp,0,BUFFER_SIZE);
	nalu.size = 0;
	while(1)
	{
		if(nalhead_pos == NO_MORE_BUFFER_TO_READ)
			return FALSE;
		while(naltail_pos < m_nFileBufSize)  
		{
			//search for nal tail
			if(m_pFileBuf[naltail_pos++] == 0x00 && m_pFileBuf[naltail_pos++] == 0x00) 
			{
				if(m_pFileBuf[naltail_pos++] == 0x01)
				{
					nalustart = 3;
					goto gotnal;
				}
				else 
				{
					//cuz we have done an i++ before,so we need to roll back now
					naltail_pos--;		
					if(m_pFileBuf[naltail_pos++] == 0x00 && m_pFileBuf[naltail_pos++] == 0x01)
					{
						nalustart = 4;
						goto gotnal;
					}
					else
						continue;
				}
			}
			else
				continue;

			//找到了起始码
			gotnal:
 				/**
				 *special case1:parts of the nal lies in a m_pFileBuf and we have to read from buffer 
				 *again to get the rest part of this nal
				 */
			    //新读取的m_pFileBuf中找到了相应的StartCode
				if(nalhead_pos == GOT_A_NAL_CROSS_BUFFER || nalhead_pos == GOT_A_NAL_INCLUDE_A_BUFFER)
				{
					//前一个m_pFileBuf剩下的nalu大小加上在新的m_pFileBuf中找到的nalu数据大小一起作为一个完整的Nalu
					nalu.size = nalu.size + naltail_pos - nalustart;
					if(nalu.size > BUFFER_SIZE)
					{
						//前一个m_pFileBuf剩下的nalu数据保存在m_pFileBuf_tmp中;
						m_pFileBuf_tmp_old = m_pFileBuf_tmp;	// save pointer in case realloc fails
						if((m_pFileBuf_tmp = (unsigned char*)realloc(m_pFileBuf_tmp,nalu.size)) ==  NULL)
						{
							//realloc分配空间失败，此时原来的堆区空间m_pFileBuf_tmp需要释放
							free(m_pFileBuf_tmp_old); // free original block
							return FALSE;
						}
					}
					//将在m_pFileBuf中找到的nalu拷贝到新分配的空间m_pFileBuf_tmp中
					memcpy(m_pFileBuf_tmp + nalu.size + nalustart-naltail_pos,m_pFileBuf,naltail_pos-nalustart);
					nalu.data = m_pFileBuf_tmp;
					nalhead_pos = naltail_pos;
					return TRUE;
				}
				//normal case:the whole nal is in this m_pFileBuf
				else 
				{
					//在第一次读取的m_pFileBuf中找到了PPS帧
					nalu.type = m_pFileBuf[nalhead_pos] & 0x1f; 
					nalu.size = naltail_pos - nalhead_pos - nalustart;
					if(nalu.type == 0x06)
					{
						//该nalu是补充增强信息单元(SEI),则跳过该SEI帧，继续下一轮循环
						nalhead_pos = naltail_pos;
						continue;
					}
					memcpy(m_pFileBuf_tmp,m_pFileBuf+nalhead_pos,nalu.size);
					nalu.data = m_pFileBuf_tmp;
					nalhead_pos = naltail_pos;
					return TRUE; 
				} 					
		}

		//在第一次读取的m_pFileBuf中没有找到之后的startCode，则需要继续读取数据继续查找
		if(naltail_pos >= m_nFileBufSize && nalhead_pos != GOT_A_NAL_CROSS_BUFFER && nalhead_pos != GOT_A_NAL_INCLUDE_A_BUFFER)
		{
			//从nalhead_pos开始的直到m_pFileBuf结尾都是该nalu数据，同时将这些nalu数据先保存到nalu对象中，还没有找到下一个StartCode，所以需要继续从文件读取数据以便找到该Nalu的结束位置
			nalu.size = BUFFER_SIZE - nalhead_pos;
			nalu.type = m_pFileBuf[nalhead_pos] & 0x1f; 
			memcpy(m_pFileBuf_tmp,m_pFileBuf+nalhead_pos,nalu.size);
			if((ret = read_buffer(m_pFileBuf,m_nFileBufSize)) < BUFFER_SIZE)
			{
				//文件数据全部读取完毕,则m_pFileBuf中nalhead_pos开始的数据和刚从文件中读取的数据一起都是同一个NAlu
				memcpy(m_pFileBuf_tmp+nalu.size,m_pFileBuf,ret);
				nalu.size = nalu.size + ret;
				nalu.data = m_pFileBuf_tmp;
				nalhead_pos = NO_MORE_BUFFER_TO_READ;
				return FALSE;
			}
			//从新读取的m_pFileBuf中重新寻找还未找到的StartCode
			naltail_pos = 0;
			nalhead_pos = GOT_A_NAL_CROSS_BUFFER;
			continue;
		}

		//新读取的m_pFileBuf中还是没有寻找到相应的StartCode
		if(nalhead_pos == GOT_A_NAL_CROSS_BUFFER || nalhead_pos == GOT_A_NAL_INCLUDE_A_BUFFER)
		{
			//之前剩下的nalu大小加上新读取的整个m_pFileBuf一起作为一个完整的Nalu
			nalu.size = BUFFER_SIZE + nalu.size;

			m_pFileBuf_tmp_old = m_pFileBuf_tmp;	// save pointer in case realloc fails
			if((m_pFileBuf_tmp = (unsigned char*)realloc(m_pFileBuf_tmp,nalu.size)) ==  NULL)
			{
				free(m_pFileBuf_tmp_old);  // free original block
				return FALSE;
			}

			//将新读取的整个m_pFileBuf拷贝到新分配的空间m_pFileBuf_tmp中
			memcpy(m_pFileBuf_tmp+nalu.size-BUFFER_SIZE,m_pFileBuf,BUFFER_SIZE);

			//重新从文件中读取数据,进行寻找相应的StartCode
			if((ret = read_buffer(m_pFileBuf,m_nFileBufSize)) < BUFFER_SIZE)
			{
				m_pFileBuf_tmp_old = m_pFileBuf_tmp;	// save pointer in case realloc fails
				if((m_pFileBuf_tmp = (unsigned char*)realloc(m_pFileBuf_tmp,nalu.size + ret)) ==  NULL)
				{
					free(m_pFileBuf_tmp_old);  // free original block
					return FALSE;
				}

				memcpy(m_pFileBuf_tmp+nalu.size,m_pFileBuf,ret);
				nalu.size = nalu.size + ret;
				nalu.data = m_pFileBuf_tmp;
				nalhead_pos = NO_MORE_BUFFER_TO_READ;
				return FALSE;
			}
			naltail_pos = 0;
			nalhead_pos = GOT_A_NAL_INCLUDE_A_BUFFER;
			continue;
		}
	}
	return FALSE;  
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

	if(m_pFileBuf != NULL)
	{
		free(m_pFileBuf);
	}
	if(m_pFileBuf_tmp != NULL)
	{
		free(m_pFileBuf_tmp);
	}
}


