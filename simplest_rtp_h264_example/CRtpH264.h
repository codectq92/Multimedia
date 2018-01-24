/*************************************************************************
    > File Name: CRtpH264.h
    > Author: zhongjihao
    > Mail: zhongjihao100@163.com
    > Created Time: 2018年01月22日 星期一 15时10分53秒
 ************************************************************************/

#ifndef RTP_H264_H
#define RTP_H264_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netdb.h>
#include <unistd.h> 
  
#define MAX_RTP_PKT_LENGTH     1400  
#define H264                   96  
  

/******************************************************************
RTP_FIXED_HEADER
0                   1                   2                   3
0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|V=2|P|X|  CC   |M|     PT      |       sequence number         |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                           timestamp                           |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|           synchronization source (SSRC) identifier            |
+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+
|            contributing source (CSRC) identifiers             |
|                             ....                              |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+

******************************************************************/
typedef struct
{
    /* byte 0 */  
    unsigned char csrc_len:4;       /* CSRC计数器，占4位，指示CSRC标识符的个数 expect 0 */  
    unsigned char extension:1;      /* 扩展标志，占1位，如果X=1，则在RTP报头后跟有一个扩展报头 expect 1, see RTP_OP below */  
    unsigned char padding:1;        /* 填充标志，占1位，如果P=1，则在该报文的尾部填充一个或多个额外的八位组，它们不是有效载荷的一部分 expect 0 */  
    unsigned char version:2;        /* RTP协议的版本号，占2位 expect 2 */  
    /* byte 1 */
    unsigned char payload:7;        /* 有效荷载类型，占7位，用于说明RTP报文中有效载荷的类型，如GSM音频、JPEM图像等,在流媒体中大部分是用来区分音频流和视频流的， 如H264类型为96，这样便于客户端进行解析 */  
    unsigned char marker:1;         /* 标记，占1位，不同的有效载荷有不同的含义，对于视频，标记一帧的结束，如传输h264时表示h264 nalu的最后一包；对于音频，标记会话的开始, expect 1 */  
    /* bytes 2, 3 */
    unsigned short seq_no;         /*序列号,占16位，用于标识发送者所发送的RTP报文的序列号，每发送一个报文，序列号增1。这个字段当下层的承载协议用UDP的时候,
                                    网络状况不好的时候可以用来检查丢包。同时出现网络抖动的情况可以用来对数据进行重新排序，序列号的初始值是随机的，同时音频包和视频包的sequence是分别记数的. */
    /* bytes 4-7 */
    unsigned  int timestamp;      /* 时戳,占32位，必须使用90 kHz 时钟频率。记录了该包中数据的第一个字节的采样时刻。接收者使用时戳来计算延迟和延迟抖动，并进行同步控制.
									 !!Can not use long type,long = 8 Byte int 64 bit  system!!*/         
    /* bytes 8-11 */
    unsigned int ssrc;            /* 同步信源(SSRC)标识符：占32位，SSRC相当于一个RTP传输session的ID,同步源就是指RTP包流的来源。该标识符是随机选择的，
									 在同一个RTP会话中不能有两个相同的SSRC值。当RTP session改变（如IP等）时，这个ID也要改变 stream number is used here. */  
}RTP_FIXED_HEADER;


/******************************************************************
H264中NALU_HEADER
+---------------+
|0|1|2|3|4|5|6|7|
+-+-+-+-+-+-+-+-+
|F|NRI|  Type   |
+---------------+
******************************************************************/ 
typedef struct
{
    //byte 0  
    unsigned char TYPE:5;  //NALU类型 
    unsigned char NRI:2;   //NAL重要性指示，标志该NAL单元的重要性，值越大，越重要，解码器在解码处理不过来的时候，可以丢掉重要性为0的NALU。
    unsigned char F:1;     //禁止位，初始为0，当网络发现NAL单元有比特错误时可设置该比特为1，以便接收方纠错或丢掉该单元.
           
}NALU_HEADER;/* 1 BYTES */  


/*
 * RTP负载为H.264定义了三种不同的基本的负载结构，接收端可能通过RTP负载的首字节来识别它们。这一个字节类似NALU头的格式，它的类型字段则指出了代表的是哪一种结构，这个字节的结构如下：

+---------------+
|0|1|2|3|4|5|6|7|
+-+-+-+-+-+-+-+-+
|F|NRI|  Type   |
+---------------+

Type定义如下：
0     没有定义
1-23  NAL单元   单个NAL单元包.
24    STAP-A   单一时间的组合包
25    STAP-B   单一时间的组合包
26    MTAP16   多个时间的组合包
27    MTAP24   多个时间的组合包
28    FU-A     分片的单元
29    FU-B     分片的单元
30-31 没有定义

首字节的类型字段和H.264的NALU头中类型字段的区别是，当Type的值为24~31表示这是一个特别格式的NAL单元，而H.264中，只取1~23是有效的值，下面分别说明这三种负载结构

一.Single NALU Packet（单一NAL单元模式）
   即一个RTP负载仅由首字节和一个NALU负载组成，对于小于1400字节的NALU便采用这种打包方案。这种情况下首字节类型字段和原始的H.264的NALU头类型字段是一样的。
   也就是说，在这种情况下RTP的负载是一个完整的NALU。

 0                   1                   2                   3
 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|F|NRI|  Type   |                                               |
+-+-+-+-+-+-+-+-+                                               |
|                                                               |
|               Bytes 2..n of a single NAL unit                 |
|                                                               |
|                               +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                               :...OPTIONAL RTP padding        |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+


二. Aggregation Packet（组合封包模式）
    在一个RTP中封装多个NALU，对于较小的NALU可以采用这种打包方案，从而提高传输效率。即可能是由多个NALU组成一个RTP包。
	分别有4种组合方式，STAP-A、STAP-B、MTAP16和MTAP24。那么这里的RTP负载首字节类型值分别是24、25、26和27。

 0                   1                   2                   3
 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|F|NRI|  Type   |                                               |
+-+-+-+-+-+-+-+-+                                               |
|                                                               |
|             one or more aggregation units                     |
|                                                               |
|                               +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                               :...OPTIONAL RTP padding        |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+

三.Fragmentation Units（分片封包模式FUs）
    一个NALU封装在多个RTP中，每个RTP负载由首字节（这里实际上是FU indicator，但是它和原首字节的结构一样，这里仍然称首字节）、FU header和NALU负载的一部分组成。
	对于大于1400字节的NALU便采用这种方案进行拆包处理。存在两种类型FU-A和FU-B，类型值分别是28和29。

FU-A类型如下图所示：

 0                   1                   2                   3
 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
| FU indicator  |   FU header   |                               |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+                               |
|                                                               |
|                         FU payload                            |
|                                                               |
|                               +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                               :...OPTIONAL RTP padding        |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+

FU-B类型如下图所示

 0                   1                   2                   3
 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
| FU indicator  |   FU header   |               DON             |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-|
|                                                               |
|                         FU payload                            |
|                                                               |
|                               +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                               :...OPTIONAL RTP padding        |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+


与FU-A相比，FU-B多了一个DON（decoding order number），DON使用的是网络字节序。FU-B只能用于隔行扫描封包模式，不能用于其他方面。

FU indicator字节结构如下所示：

+---------------+
|0|1|2|3|4|5|6|7|
+-+-+-+-+-+-+-+-+
|F|NRI|  Type   |
+---------------+

Type=28或29

FU header字节结构如下所示：

+---------------+
|0|1|2|3|4|5|6|7|
+-+-+-+-+-+-+-+-+
|S|E|R|  Type   |
+---------------+

S（Start）: 1 bit，当设置成1，该位指示分片NAL单元的开始。当随后的FU负载不是分片NAL单元的开始，该位设为0。
E（End）: 1 bit，当设置成1, 该位指示分片NAL单元的结束，此时荷载的最后字节也是分片NAL单元的最后一个字节。当随后的FU荷载不是分片NAL单元的结束,该位设为0。
R（Reserved）: 1 bit，保留位必须设置为0，且接收者必须忽略该位。

Type：与NALU头中的Type值相同

*/

/******************************************************************
FU_INDICATOR
+---------------+
|0|1|2|3|4|5|6|7|
+-+-+-+-+-+-+-+-+
|F|NRI|  Type   |
+---------------+
******************************************************************/ 
typedef struct
{  
    //byte 0  
    unsigned char TYPE:5;
    unsigned char NRI:2;
    unsigned char F:1;
}FU_INDICATOR; /* 1 BYTES */  


/******************************************************************
FU_HEADER
+---------------+
|0|1|2|3|4|5|6|7|
+-+-+-+-+-+-+-+-+
|S|E|R|  Type   |
+---------------+
******************************************************************/
typedef struct
{
    //byte 0
    unsigned char TYPE:5;
    unsigned char R:1;
    unsigned char E:1;
    unsigned char S:1;
}FU_HEADER;	/* 1 BYTES */


typedef struct  
{
	int startcodeprefix_len;      //! 4 for parameter sets and first slice in picture, 3 for everything else (suggested)
	unsigned int len;             //! Length of the NAL unit (Excluding the start code, which does not belong to the NALU)
	unsigned int max_size;        //! Nal Unit Buffer size
	int forbidden_bit;            //! should be always FALSE
	int nal_reference_idc;        //! NALU_PRIORITY_xxxx
	int nal_unit_type;            //! NALU_TYPE_xxxx
	char *buf;                    //! contains the first byte followed by the EBSP
}NALU_t;

//RTP传输H264视频碼流
class CRtpH264
{
private:
	struct sockaddr_in mServer;
	int mSocketFd;
	FILE* p_h264bitstream;             //!< the bit stream file
	unsigned short mSeq_num;
	unsigned int mTimestamp_increase;
	unsigned int mTs_current;
	NALU_t* p_Nalu;

private:
	int FindStartCode2(unsigned char *Buf);//查找3字节起始码0x000001
	int FindStartCode3(unsigned char *Buf);//查找4字节起始码0x00000001
	int SendRtpPacket(char* sendbuf, NALU_t* n);
	int GetAnnexbNALU(NALU_t* nalu);

public:
	CRtpH264();
	~CRtpH264();
	void initSocket(const char* serIP,int port);
	void AllocNALU(int buffersize);
	void FreeNALU();
	void OpenBitstreamFile(const char* fn);
	void ConstructRtpPacket(const char* file);

};

#endif


