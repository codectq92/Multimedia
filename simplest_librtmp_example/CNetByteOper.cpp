/*************************************************************************
    > File Name: CNetByteOper.cpp
    > Author: zhongjihao
    > Mail: zhongjihao100@163.com 
    > Created Time: 2018年01月10日 星期三 15时03分54秒
 ************************************************************************/

#include "libRTMP/rtmpdump/librtmp/amf.h"
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

//1字节主机字节序转换为网络字节序
char* CNetByteOper::put_byte(char* output, uint8_t nVal)
{
	output[0] = nVal;
	return output+1;
}

//2字节主机字节序转换为网络字节序
char* CNetByteOper::put_be16(char* output, uint16_t nVal)
{
	output[1] = nVal & 0xff;
	output[0] = nVal >> 8;
	return output+2;
}

//3字节主机字节序转换为网络字节序
char* CNetByteOper::put_be24(char* output, uint32_t nVal)
{
	output[2] = nVal & 0xff;
	output[1] = (nVal >> 8) & 0xff;
	output[0] = (nVal >> 16) & 0xff;
	return output+3;
}

//4字节主机字节序转换为网络字节序
char* CNetByteOper::put_be32(char* output, uint32_t nVal)
{
	output[3] = nVal & 0xff;
	output[2] = (nVal >> 8) & 0xff;
	output[1] = (nVal >> 16) & 0xff;
	output[0] = nVal >> 24;
	return output+4;
}

//8字节主机字节序转换为网络字节序
char* CNetByteOper::put_be64(char* output, uint64_t nVal)
{
	output = put_be32(output, nVal >> 32);
	output = put_be32(output, nVal & 0x00000000ffffffff);
	return output;
}

//字符串转换为网络字节序
char* CNetByteOper::put_amf_string(char* c, const char* str)
{
	uint16_t len = strlen(str);
	//先用2字节存储字符串的长度
	c = put_be16(c, len);
	//接着拷贝字符串内容
	memcpy(c,str,len);
	return c+len;
}

//double类型转换为网络字节序
char* CNetByteOper::put_amf_double(char* c, double d)
{
	//1字节存放类型Number，0代表Number类型，占8字节，即Double类型
	*c++ = AMF_NUMBER;  /* type: Number */
	{
		unsigned char *ci, *co;
		ci = (unsigned char *)&d;
		co = (unsigned char *)c;
		//网络字节序即Big endian，高位存储在低地址中，低位存储在高地址中
		co[0] = ci[7];
		co[1] = ci[6];
		co[2] = ci[5];
		co[3] = ci[4];
		co[4] = ci[3];
		co[5] = ci[2];
		co[6] = ci[1];
		co[7] = ci[0];
	}
    return c+8;
}

UINT CNetByteOper::Ue(BYTE* pBuff, UINT nLen, UINT& nStartBit)
{
	//计算0bit的个数
	UINT nZeroNum = 0;
	while (nStartBit < nLen * 8)
	{
		if (pBuff[nStartBit / 8] & (0x80 >> (nStartBit % 8))) //&:按位与，%取余
		{
			break;
		}
		nZeroNum++;
		nStartBit++;
	}
	nStartBit++;

	//计算结果
	DWORD dwRet = 0;
	for (UINT i=0; i<nZeroNum; i++)
	{
		dwRet <<= 1;
		if (pBuff[nStartBit / 8] & (0x80 >> (nStartBit % 8)))
		{
			dwRet += 1;
		}
		nStartBit++;
	}
	return (1 << nZeroNum) - 1 + dwRet;
}

int CNetByteOper::Se(BYTE* pBuff, UINT nLen, UINT& nStartBit)
{
	int UeVal = Ue(pBuff,nLen,nStartBit);
	double k = UeVal;
	int nValue = ceil(k/2);//ceil函数：ceil函数的作用是求不小于给定实数的最小整数。ceil(2)=ceil(1.2)=cei(1.5)=2.00
	if (UeVal % 2 == 0)
		nValue=-nValue;
	return nValue;
}

DWORD CNetByteOper::u(UINT BitCount,BYTE* buf,UINT& nStartBit)
{
	DWORD dwRet = 0;
	for (UINT i=0; i<BitCount; i++)
	{
		dwRet <<= 1;
		if (buf[nStartBit / 8] & (0x80 >> (nStartBit % 8)))
		{
			dwRet += 1;
		}
		nStartBit++;
	}
	return dwRet;
}

