/*************************************************************************
    > File Name: simplest_rtp_send_h264.cpp
    > Author: zhongjihao
    > Mail: zhongjihao100@163.com 
    > Created Time: 2018年01月23日 星期二 19时29分44秒
 ************************************************************************/

#include "CRtpH264.h"

#define DEST_IP                "127.0.0.1" /* 显示端 IP 地址 */ 
#define DEST_PORT              16000

int main(int argc, char* argv[])  
{
	CRtpH264* pRtpH264 = new CRtpH264;
	pRtpH264->initSocket(DEST_IP,DEST_PORT);
   
	pRtpH264->ConstructRtpPacket("./res/test.h264");

	delete pRtpH264;

    return 0;  
}  

