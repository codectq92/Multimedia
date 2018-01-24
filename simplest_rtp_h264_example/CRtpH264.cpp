/*************************************************************************
    > File Name: CRtpH264.cpp
    > Author: zhongjihao
    > Mail: zhongjihao100@163.com
    > Created Time: 2018年01月23日 星期二 10时10分53秒
 ************************************************************************/

#include "CRtpH264.h"  


CRtpH264::CRtpH264() : mSocketFd(0),mSeq_num(0),mTimestamp_increase(0),mTs_current(0),p_h264bitstream(NULL),p_Nalu(NULL)
{

}

CRtpH264::~CRtpH264()
{
	close(mSocketFd);
	FreeNALU();
	if(p_h264bitstream)
		fclose(p_h264bitstream);
    p_h264bitstream = NULL;
	printf("%s: ====haoge====\n",__FUNCTION__);
}

void CRtpH264::initSocket(const char* serIP,int port)
{
    mServer.sin_family = AF_INET;
    mServer.sin_port = htons(port);
    mServer.sin_addr.s_addr = inet_addr(serIP);
    mSocketFd = socket(AF_INET, SOCK_DGRAM, 0);
}

void CRtpH264::AllocNALU(int buffersize)  
{
	if((p_Nalu = (NALU_t*)calloc(1,sizeof(NALU_t))) == NULL)  
	{
		printf("%s: ===haoge===Alloc NALU: null\n",__FUNCTION__);  
		exit(0);
	}

	p_Nalu->max_size = buffersize;
	if((p_Nalu->buf = (char*)calloc(buffersize, sizeof(char))) == NULL)  
	{
		free(p_Nalu); 
		printf("%s: ===haoge====Alloc NALU: n->buf null\n",__FUNCTION__);
		exit(0);
	}
	return;
}

void CRtpH264::FreeNALU()  
{  
	if(p_Nalu)
	{
		if(p_Nalu->buf)
		{
			free(p_Nalu->buf);
			p_Nalu->buf = NULL;
		}
		free(p_Nalu);
		p_Nalu = NULL;
	}
}

void CRtpH264::OpenBitstreamFile(const char *fn)  
{
	if(NULL == (p_h264bitstream = fopen(fn, "r")))
	{
		printf("%s: ====haoge====open file error\n",__FUNCTION__);
		exit(0);
	}
}


int CRtpH264::FindStartCode2(unsigned char* Buf)  
{  
	if(Buf[0] != 0 || Buf[1] != 0 || Buf[2] != 1) 
		return 0; //判断是否为0x000001,如果是返回 1
	else
		return 1;
}

int CRtpH264::FindStartCode3(unsigned char* Buf)  
{
	if(Buf[0] != 0 || Buf[1] != 0 || Buf[2] != 0 || Buf[3] != 1)
		return 0;//判断是否为0x00000001,如果是返回 1
	else
		return 1;
}

//这个函数主要功能为得到一个完整的NALU并保存在NALU_t的buf中，获取他的长度，填充F,IDC,TYPE位. 并且返回NALU的长度
int CRtpH264::GetAnnexbNALU(NALU_t* nalu)  
{
	int mInfo2 = 0;
	int mInfo3 = 0;
	int pos = 0;
	int rewind;
	int StartCodeFound;
	unsigned char* Buf;

//	memset(nalu->buf,0,nalu->max_size);

	if((Buf = (unsigned char*)calloc(nalu->max_size,sizeof(char))) == NULL)
		printf("%s: =====haoge====GetAnnexbNALU: Could not allocate Buf memory\n",__FUNCTION__);

	printf("%s: =====haoge===nalu->max_size = %d\n",__FUNCTION__,nalu->max_size);
	memset(Buf,0,nalu->max_size);
	nalu->startcodeprefix_len = 3;//初始化码流序列的开始字符为3个字节
	if(3 != fread(Buf, 1, 3, p_h264bitstream))//从码流中读3个字节
	{
		free(Buf);
		return 0;
	}

	//先找3字节的startCode 0x000001
	mInfo2 = FindStartCode2(Buf);
	if(mInfo2 != 1)
	{  
		//如果不是，再读一个字节  
		if(1 != fread(Buf+3, 1, 1, p_h264bitstream))//读一个字节  
		{
			free(Buf);
			return 0;
		}
		//再找4字节的StartCode 0x00000001
		mInfo3 = FindStartCode3(Buf);
		if(mInfo3 != 1)//如果不是，返回-1
		{
			free(Buf);
			return -1;
		}
		else
		{
			//如果是0x00000001,得到开始前缀为4个字节
			pos = 4;
			nalu->startcodeprefix_len = 4;
		}
	}
	else  
	{
		//如果是0x000001,得到开始前缀为3个字节  
		nalu->startcodeprefix_len = 3;
		pos = 3;
	}

	StartCodeFound = 0;
	mInfo2 = 0;
	mInfo3 = 0;
    //找到StartCode后，查找相邻的下一个StartCode的位置
	while(!StartCodeFound)
	{
		if(feof(p_h264bitstream))//判断是否到了文件尾
		{
			//所谓文件末尾即最后一个字符的下一个位置，所以达到文件末尾时，此时pos-1代表Buf中的字节总数
			nalu->len = (pos-1) - nalu->startcodeprefix_len;//NALU的大小，包括一个字节NALU头和EBSP数据
			printf("%s: ====haoge===1===nalu->len = %d\n",__FUNCTION__,nalu->len);
			memcpy(nalu->buf, &Buf[nalu->startcodeprefix_len], nalu->len); //拷贝一个完整NALU，不拷贝起始前缀0x000001或0x00000001
			nalu->forbidden_bit 	= nalu->buf[0] & 0x80;     // 1 bit
			nalu->nal_reference_idc = nalu->buf[0] & 0x60;     // 2 bit
			nalu->nal_unit_type		= nalu->buf[0] & 0x1f;     // 5 bit
			free(Buf);
			return pos - 1;
		}

		//pos代表Buf中存储的字节个数
		Buf[pos++] = fgetc(p_h264bitstream);//读一个字节到BUF中
		mInfo3 = FindStartCode3(&Buf[pos-4]);//判断是否为0x00000001
		if(mInfo3 != 1)
		{
			mInfo2 = FindStartCode2(&Buf[pos-3]);//判断是否为0x000001
		}
		StartCodeFound = (mInfo2 == 1 || mInfo3 == 1);
	}

	// Here, we have found another start code (and read length of startcode bytes more than we should  
	// have.  Hence, go back in the file  
	rewind = (mInfo3 == 1) ? -4 : -3;
	
	if(0 != fseek(p_h264bitstream, rewind, SEEK_CUR))//把文件指针指向前一个NALU的末尾
	{
		free(Buf);
		printf("%s :=====haoge====Cannot fseek in the bit stream file\n",__FUNCTION__);
	}
	
	// Here the Start code, the complete NALU, and the next start code is in the Buf.    
	// The size of Buf is pos, pos+rewind are the number of bytes excluding the next  
	// start code, and (pos+rewind)-startcodeprefix_len is the size of the NALU excluding the start code  
	
	nalu->len = (pos+rewind) - nalu->startcodeprefix_len; //NALU的大小，包括一个字节NALU头和EBSP数据 
	printf("%s: ===haoge=====2====nalu->len =  %d\n",__FUNCTION__,nalu->len);
	memcpy(nalu->buf,&Buf[nalu->startcodeprefix_len],nalu->len);//拷贝一个完整NALU，不拷贝起始前缀0x000001或0x00000001  
	nalu->forbidden_bit		 = nalu->buf[0] & 0x80; 	// 1  bit  提取NALU头中的forbidden_bit (禁止位)
	nalu->nal_reference_idc  = nalu->buf[0] & 0x60;     // 2  bit  提取NALU头中的nal_reference_bit (优先级)
	nalu->nal_unit_type		 = nalu->buf[0] & 0x1f;     // 5  bit  提取NALU头中的nal_unit_type (NAL类型)
	free(Buf);

    //返回两个起始码之间间隔的字节数，即包含有前缀的NALU的长度
	return (pos+rewind);
}  

int CRtpH264::SendRtpPacket(char* sendbuf, NALU_t* n)
{
	int mBytes = 0;
	RTP_FIXED_HEADER* pRtp_hdr  = NULL;
	NALU_HEADER*      pNalu_hdr = NULL;
	FU_INDICATOR*     pFu_ind   = NULL;
	FU_HEADER*        pFu_hdr   = NULL;
	char* nalu_payload = NULL;
	//rtp固定包头，为12字节,该句将sendbuf[0]的地址赋给pRtp_hdr，以后对pRtp_hdr的写入操作将直接写入sendbuf
    pRtp_hdr = (RTP_FIXED_HEADER*)&sendbuf[0];
	//设置RTP HEADER
    pRtp_hdr->csrc_len	 = 0;
	pRtp_hdr->extension = 0;
    pRtp_hdr->padding = 0;
	pRtp_hdr->payload = H264;  		//负载类型号
    pRtp_hdr->version = 2;  		//版本号，此版本固定为2
    pRtp_hdr->marker = 0;   		//标志位，由具体协议规定其值
    pRtp_hdr->ssrc = htonl(10); 	//随机指定为10，并且在本RTP会话中全局唯一  bytes 8-11
	//当一个NALU小于1400字节的时候，采用一个单RTP包发送
    if(n->len <= 1400)
	{
		//设置rtp M 位；
		pRtp_hdr->marker = 1;  
        pRtp_hdr->seq_no = htons(mSeq_num++); //序列号，每发送一个RTP包增1  bytes 2, 3
        //设置NALU HEADER,并将这个HEADER填入sendbuf[12]
        pNalu_hdr 		= (NALU_HEADER*)&sendbuf[12]; //将sendbuf[12]的地址赋给pNalu_hdr，之后对pNalu_hdr的写入就将写入sendbuf中；
        pNalu_hdr->F 	= n->forbidden_bit >> 7;
        pNalu_hdr->NRI 	= n->nal_reference_idc >> 5;//有效数据在n->nal_reference_idc的第6，7位，需要右移5位才能将其值赋给nalu_hdr->NRI
        pNalu_hdr->TYPE	= n->nal_unit_type;

		nalu_payload	= &sendbuf[13];//同理将sendbuf[13]赋给nalu_payload  
        memcpy(nalu_payload, n->buf + 1, n->len - 1);//去掉nalu头的nalu剩余内容写入sendbuf[13]开始的字符串

		mTs_current = mTs_current + mTimestamp_increase;
        pRtp_hdr->timestamp = htonl(mTs_current);
		printf("%s: ======haoge===timestamp = %u, ts_current = %u\n",__FUNCTION__,pRtp_hdr->timestamp,mTs_current);
        mBytes = n->len + 12; //获得sendbuf的长度,为nalu的长度（包含NALU头但除去起始前缀）加上rtp_header的固定长度12字节

		sendto(mSocketFd,sendbuf,mBytes,0,(struct sockaddr *)&mServer, sizeof(mServer));
		usleep(50000);
	}
	/*
	 * 同一个NALU分包的FU indicator头是完全一致的，FU header只有S以及E位有区别，分别标记开始和结束，它们的RTP分包的序列号应该是依次递增的，
	 * 并且它们的时间戳必须一致，而负载数据为NALU包去掉1个字节的NALU头后对剩余数据的拆分
	 */
	else if(n->len > 1400)
	{
		//得到该nalu需要用多少长度为1400字节的RTP包来发送
        int k = 0;
		int l = 0;
		int t = 0;          //用于指示当前发送的是第几个分片RTP包
        k = n->len / 1400;	//需要k个1400字节的RTP包
        l = n->len % 1400;	//最后一个RTP包的需要装载的字节数

		mTs_current = mTs_current + mTimestamp_increase;
		printf("%s: ======haoge=====ts_current = %d\n",__FUNCTION__,mTs_current);
        pRtp_hdr->timestamp = htonl(mTs_current);
		while(t <= k)
		{
			pRtp_hdr->seq_no = htons(mSeq_num++);   //序列号，每发送一个RTP包增1
			//发送一个需要分片的NALU的第一个分片，置FU HEADER的S位
            if(!t)
			{
				//设置rtp M 位；
                pRtp_hdr->marker = 0; 
                //设置FU INDICATOR,并将这个HEADER填入sendbuf[12]  
                pFu_ind 	 = (FU_INDICATOR*)&sendbuf[12]; //将sendbuf[12]的地址赋给pFu_ind，之后对pFu_ind的写入就将写入sendbuf中；
                pFu_ind->F 	 = n->forbidden_bit >> 7;
                pFu_ind->NRI = n->nal_reference_idc >> 5;
                pFu_ind->TYPE = 28; //FU-A类型
                //设置FU HEADER,并将这个HEADER填入sendbuf[13]
                pFu_hdr 	 = (FU_HEADER*)&sendbuf[13];
                pFu_hdr->E	 = 0;
                pFu_hdr->R	 = 0;
                pFu_hdr->S	 = 1;
                pFu_hdr->TYPE = n->nal_unit_type;
                nalu_payload = &sendbuf[14];               //同理将sendbuf[14]赋给nalu_payload
                memcpy(nalu_payload, n->buf + 1, 1400);    //去掉NALU头
                mBytes = 1400 + 12 + 2;                    //获得sendbuf的长度,为nalu的长度（除去起始前缀和NALU头）加上rtp_header，fu_ind，fu_hdr的固定长度14字节


				sendto(mSocketFd, sendbuf,mBytes,0,(struct sockaddr *)&mServer, sizeof(mServer));
                t++;
			}
			//发送一个需要分片的NALU的非第一个分片，清零FU HEADER的S位，如果该分片是该NALU的最后一个分片，置FU HEADER的E位
			else if(t < k && 0 != t)
			{
				//设置rtp M 位；
                pRtp_hdr->marker = 0;
                //设置FU INDICATOR,并将这个HEADER填入sendbuf[12]
                pFu_ind 		= (FU_INDICATOR*)&sendbuf[12]; //将sendbuf[12]的地址赋给pFu_ind，之后对pFu_ind的写入就将写入sendbuf中；  
                pFu_ind->F 		= n->forbidden_bit >> 7;
                pFu_ind->NRI 	= n->nal_reference_idc >> 5;
                pFu_ind->TYPE 	= 28;
                //设置FU HEADER,并将这个HEADER填入sendbuf[13]
                pFu_hdr 		= (FU_HEADER*)&sendbuf[13];
                pFu_hdr->R 		= 0;
                pFu_hdr->S 		= 0;
                pFu_hdr->E 		= 0;
                pFu_hdr->TYPE 	= n->nal_unit_type;
                nalu_payload 	= &sendbuf[14];                     //同理将sendbuf[14]的地址赋给nalu_payload
                memcpy(nalu_payload, n->buf + t * 1400 + 1, 1400);  //去掉起始前缀的nalu剩余内容写入sendbuf[14]开始的字符串
                mBytes = 1400 + 12 + 2;                             //获得sendbuf的长度,为nalu的长度（除去原NALU头）加上rtp_header，fu_ind，fu_hdr的固定长度14字节


			    sendto(mSocketFd, sendbuf, mBytes, 0, (struct sockaddr *)&mServer, sizeof(mServer));
                t++; 
			}
			//发送的是最后一个分片，注意最后一个分片的长度可能超过1400字节 (当l>1386时)
            else if(k == t)
			{
				//设置rtp M 位；当前传输的是最后一个分片时该位置1
                pRtp_hdr->marker = 1;

				//设置FU INDICATOR,并将这个HEADER填入sendbuf[12]
                pFu_ind			= (FU_INDICATOR*)&sendbuf[12]; //将sendbuf[12]的地址赋给pFu_ind，之后对pFu_ind的写入就将写入sendbuf中;
                pFu_ind->F		= n->forbidden_bit >> 7;
                pFu_ind->NRI	= n->nal_reference_idc >> 5;
                pFu_ind->TYPE	= 28;

				//设置FU HEADER,并将这个HEADER填入sendbuf[13]
                pFu_hdr 		= (FU_HEADER*)&sendbuf[13];
                pFu_hdr->R      = 0;
                pFu_hdr->S 		= 0;
                pFu_hdr->TYPE	= n->nal_unit_type;
                pFu_hdr->E		= 1;
                nalu_payload	= &sendbuf[14];//同理将sendbuf[14]的地址赋给nalu_payload
				if((n != NULL) && (n->buf != NULL) && (l > 1))
				{
					memcpy(nalu_payload, n->buf + t * 1400 + 1, l - 1); //将nalu最后剩余的l-1(去掉了一个字节的NALU头)字节内容写入sendbuf[14]开始的字符串.
					mBytes = l - 1 + 12 + 2;                             //获得sendbuf的长度,为剩余nalu的长度l-1加上rtp_header，FU_INDICATOR,FU_HEADER三个包头共14字节
			        sendto(mSocketFd, sendbuf, mBytes, 0, (struct sockaddr *)&mServer, sizeof(mServer));
				}
				else
					printf("%s: ======haoge=====n->buf == NULL !\n",__FUNCTION__);
				t++;
			}
		}
		usleep(50000);
	}
}



void CRtpH264::ConstructRtpPacket(const char* file)
{
    char  sendbuf[1500];
    float framerate = 25;
    mSeq_num = 0;
    mTimestamp_increase = 0;
	mTs_current = 0;

	OpenBitstreamFile(file);
	//h264的采样率为90000HZ，因此时间戳的单位为1(秒)/90000，因此如果当前视频帧率为25fps，那时间戳间隔或者说增量应该为3600，
	//每帧是1/25秒，那么这1/25秒有多少个时间戳单元呢，除以1/90000即可。而如果帧率为30fps，则增量为3000，以此类推
    mTimestamp_increase = (unsigned int)(90000.0 / framerate); //+0.5);
    AllocNALU(8000000);//为结构体nalu_t及其成员buf分配空间。返回值为指向nalu_t存储空间的指针

	while(!feof(p_h264bitstream))
	{
		GetAnnexbNALU(p_Nalu);//每执行一次，文件的指针指向本次找到的NALU的末尾，下一个位置即为下个NALU的起始码
		//（1）一个NALU就是一个RTP包的情况： RTP_FIXED_HEADER（12字节）  + NALU_HEADER（1字节） + EBPS
        //（2）一个NALU分成多个RTP包的情况： RTP_FIXED_HEADER （12字节） + FU_INDICATOR （1字节）+  FU_HEADER（1字节） + EBPS(1400字节)
        memset(sendbuf, 0, 1500);//清空sendbuf；此时会将上次的时间戳清空，因此需要mTs_current来保存上次的时间戳值
		SendRtpPacket(sendbuf,p_Nalu);
	}
}


