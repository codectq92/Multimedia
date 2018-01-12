/*************************************************************************
    > File Name: CNetByteOper.cpp
    > Author: zhongjihao
    > Mail: zhongjihao100@163.com 
    > Created Time: 2018年01月10日 星期三 15时03分54秒
 ************************************************************************/

#include "CNetByteOper.h"

/*read 1 byte*/
int CNetByteOper::ReadU8(uint32_t *u8,FILE* fp)
{
	if(fread(u8,1,1,fp) != 1)
		return 0;
	return 1;
}

/*read 2 byte*/
int CNetByteOper::ReadU16(uint32_t *u16,FILE* fp)
{
	if(fread(u16,2,1,fp) != 1)
		return 0;
	*u16 = HTON16(*u16);
	return 1;
}

/*read 3 byte*/
int CNetByteOper::ReadU24(uint32_t *u24,FILE* fp)
{
	if(fread(u24,3,1,fp) != 1)
		return 0;
	*u24 = HTON24(*u24);
	return 1;
}

/*read 4 byte*/
int CNetByteOper::ReadU32(uint32_t *u32,FILE* fp)
{
	if(fread(u32,4,1,fp) != 1)
		return 0;
	*u32 = HTON32(*u32);
	return 1;
}

/*read 1 byte,and loopback 1 byte at once*/
int CNetByteOper::PeekU8(uint32_t *u8,FILE* fp)
{
	if(fread(u8,1,1,fp) != 1)
		return 0;
	fseek(fp,-1,SEEK_CUR);
	return 1;
}

/*read 4 byte and convert to time format*/
int CNetByteOper::ReadTime(uint32_t *utime,FILE* fp)
{
	if(fread(utime,4,1,fp) != 1)
		return 0;
	//当24位时间戳不够用时，1字节的扩展时间戳放到最高位和3字节的时间戳一起存放时间
	*utime = HTONTIME(*utime);
	return 1;
}

