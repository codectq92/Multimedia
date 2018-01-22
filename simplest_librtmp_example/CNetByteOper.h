/*************************************************************************
    > File Name: CNetByteOper.h
    > Author: zhongjihao
    > Mail: zhongjihao100@163.com 
    > Created Time: 2018年01月10日 星期三 15时03分54秒
 ************************************************************************/

#ifndef CNET_BYTE_OPER_H
#define CNET_BYTE_OPER_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <math.h>
#include <unistd.h>

#define HTON16(x)  ( (x>>8 & 0xff) | (x<<8 & 0xff00) )
#define HTON24(x)  ( (x>>16 & 0xff) | (x<<16 & 0xff0000) | (x & 0xff00) )
#define HTON32(x)  ( (x>>24 & 0xff) | (x>>8 & 0xff00) | (x<<8 & 0xff0000) | (x<<24 & 0xff000000) )
#define HTONTIME(x) ( (x>>16 & 0xff) | (x<<16 & 0xff0000) | (x & 0xff00) | (x & 0xff000000) )

typedef unsigned int  UINT;
typedef unsigned char BYTE;
typedef unsigned long DWORD;

//网络字节操作类
class CNetByteOper
{
public:
	/*read 1 byte*/
	static int ReadU8(uint32_t *u8,FILE* fp);
	/*read 2 byte*/
	static int ReadU16(uint32_t *u16,FILE* fp);
	/*read 3 byte*/
	static int ReadU24(uint32_t *u24,FILE* fp);
	/*read 4 byte*/
	static int ReadU32(uint32_t *u32,FILE* fp);
	/*read 1 byte,and loopback 1 byte at once*/
	static int PeekU8(uint32_t *u8,FILE* fp);
	/*read 4 byte and convert to time format*/
	static int ReadTime(uint32_t *utime,FILE* fp);

	//主机字节序向网络字节序转换
	static char* put_byte(char* output, uint8_t nVal);
	static char* put_be16(char* output, uint16_t nVal);
	static char* put_be24(char* output, uint32_t nVal);
	static char* put_be32(char* output, uint32_t nVal);
	static char* put_be64(char* output, uint64_t nVal);
	static char* put_amf_string(char* c, const char* str);
	static char* put_amf_double(char* c, double d);

	static UINT Ue(BYTE* pBuff, UINT nLen, UINT& nStartBit);
	static int Se(BYTE* pBuff, UINT nLen, UINT& nStartBit);
	static DWORD u(UINT BitCount,BYTE* buf,UINT& nStartBit);
};



#endif

