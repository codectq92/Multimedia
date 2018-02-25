//
// Created by zhongjihao on 18-2-9.
//

#define LOG_TAG "RTMP-JNI"

#include "com_android_rtmpvideo_RtmpH264.h"
#include "./rtmpjni/CRtmpSendH264.h"
#include <string.h>

#include <android/log.h>

#define ALOGD(...) __android_log_print(ANDROID_LOG_DEBUG,LOG_TAG,__VA_ARGS__)
#define ALOGE(...) __android_log_print(ANDROID_LOG_ERROR,LOG_TAG,__VA_ARGS__)

static CRtmpSendH264* pRtmpH264 = new CRtmpSendH264;
static FILE* pLogfile = NULL;

JNIEXPORT jint JNICALL Java_com_android_rtmpvideo_RtmpH264_initRtmp(JNIEnv* env,jclass jcls,jstring jurl,jstring jlogpath)
{
    ALOGD("%s: =======zhongjihao=====",__FUNCTION__);
    int ret = 0;
    const char* url = env->GetStringUTFChars(jurl,NULL);
    const char* logpath = env->GetStringUTFChars(jlogpath,NULL);
    pLogfile = fopen(logpath,"w+");
    if(!pLogfile)
    {
        ALOGE("%s: ======zhongjihao=====open: %s",__FUNCTION__,strerror(errno));
        goto failed;
    }
    ALOGD("%s: =====zhongjihao=====uri: %s",__FUNCTION__,url);
    //初始化并连接到服务器
    ret = pRtmpH264->RTMPH264_Connect(url,pLogfile);

failed:
    env ->ReleaseStringUTFChars(jurl, url);
    env ->ReleaseStringUTFChars(jlogpath,logpath);
    ALOGD("%s: ======zhongjihao=====ret: %d",__FUNCTION__,ret);
    return ret;
}

JNIEXPORT jint JNICALL Java_com_android_rtmpvideo_RtmpH264_sendSpsAndPps(JNIEnv* env,jclass jcls,jbyteArray jsps, jint spsLen, jbyteArray jpps, jint ppsLen)
{
    jbyte* sps = env->GetByteArrayElements(jsps, NULL);
    jbyte* pps = env->GetByteArrayElements(jpps, NULL);

    unsigned  char* spsData = (unsigned char*)sps;
    unsigned  char* ppsData = (unsigned char*)pps;

    int ret = pRtmpH264->SendVideoSpsPps(ppsData,ppsLen,spsData,spsLen);
    ALOGD("%s: =====zhongjihao=====ret: %d",__FUNCTION__,ret);
    env->ReleaseByteArrayElements(jsps, sps, 0);
    env->ReleaseByteArrayElements(jpps, pps, 0);
    return 0;
}

JNIEXPORT jint JNICALL Java_com_android_rtmpvideo_RtmpH264_sendVideoFrame(JNIEnv* env,jclass jcls,jbyteArray jframe, jint len,jint time)
{
    ALOGD("%s: ==1===zhongjihao=====RTMP推送开始====大小: %d, 时间戳: %d",__FUNCTION__,len,(unsigned int)time);
    jbyte* frame = env->GetByteArrayElements(jframe, NULL);
    unsigned char* data = (unsigned char*)frame;
    /*去掉StartCode帧界定符*/
    if (data[2] == 0x00) {/*00 00 00 01*/
        data += 4;
        len -= 4;
    } else if (data[2] == 0x01) {/*00 00 01*/
        data += 3;
        len -= 3;
    }

    //提取NALU Header中type字段,Nalu头一个字节，type是其后5bit
    int type = data[0] & 0x1f;
    int bKeyframe  = (type == 0x05) ? TRUE : FALSE;

    int ret = pRtmpH264->SendH264Packet(data,len,bKeyframe,(unsigned int)time);
    ALOGD("%s: ==2===zhongjihao=====RTMP推送完成====大小: %d, 时间戳: %d,  ret: %d",__FUNCTION__,len,(unsigned int)time,ret);
    env->ReleaseByteArrayElements(jframe, frame, 0);
    return ret;
}

JNIEXPORT jint JNICALL Java_com_android_rtmpvideo_RtmpH264_stopRtmp(JNIEnv* env,jclass jcls)
{
    ALOGD("%s: ==1===zhongjihao=====RTMP断开======",__FUNCTION__);
    //断开RTMP连接并释放相关资源
    pRtmpH264->RTMPH264_Close();
    if(pLogfile)
        fclose(pLogfile);
    pLogfile = NULL;
    ALOGD("%s: ==2===zhongjihao=====RTMP断开======",__FUNCTION__);
    return 0;
}