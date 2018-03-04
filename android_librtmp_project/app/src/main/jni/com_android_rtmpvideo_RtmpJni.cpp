//
// Created by zhongjihao on 18-2-9.
//

#define LOG_TAG "RTMP-JNI"

#include "com_android_rtmpvideo_RtmpJni.h"
#include "rtmpjni/CRtmpWrap.h"
#include <string.h>

#include <android/log.h>

#define ALOGD(...) __android_log_print(ANDROID_LOG_DEBUG,LOG_TAG,__VA_ARGS__)
#define ALOGE(...) __android_log_print(ANDROID_LOG_ERROR,LOG_TAG,__VA_ARGS__)

static CRtmpWrap* pRtmpH264 = new CRtmpWrap;
static FILE* pLogfile = NULL;

JNIEXPORT jint JNICALL Java_com_android_rtmpvideo_RtmpJni_initRtmp(JNIEnv* env,jclass jcls,jstring jurl,jstring jlogpath)
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
    ret = pRtmpH264->RTMPAV_Connect(url,pLogfile);

failed:
    env ->ReleaseStringUTFChars(jurl, url);
    env ->ReleaseStringUTFChars(jlogpath,logpath);
    ALOGD("%s: ======zhongjihao=====ret: %d",__FUNCTION__,ret);
    return ret;
}

JNIEXPORT jint JNICALL Java_com_android_rtmpvideo_RtmpJni_sendSpsAndPps(JNIEnv* env,jclass jcls,jbyteArray jsps, jint spsLen, jbyteArray jpps, jint ppsLen)
{
    jbyte* sps = env->GetByteArrayElements(jsps, NULL);
    jbyte* pps = env->GetByteArrayElements(jpps, NULL);

    unsigned  char* spsData = (unsigned char*)sps;
    unsigned  char* ppsData = (unsigned char*)pps;

    int ret = pRtmpH264->SendVideoSpsPps(ppsData,ppsLen,spsData,spsLen);
    ALOGD("%s: =====zhongjihao=====ret: %d",__FUNCTION__,ret);
    env->ReleaseByteArrayElements(jsps, sps, 0);
    env->ReleaseByteArrayElements(jpps, pps, 0);
    return ret;
}

JNIEXPORT jint JNICALL Java_com_android_rtmpvideo_RtmpJni_sendVideoFrame(JNIEnv* env,jclass jcls,jbyteArray jframe, jint len,jint time)
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

JNIEXPORT jint JNICALL Java_com_android_rtmpvideo_RtmpJni_sendAacSpec(JNIEnv* env, jclass jcls, jbyteArray jaacSpec, jint jlen)
{
    jbyte* aacSpec = env->GetByteArrayElements(jaacSpec, NULL);
    unsigned  char* aacSpecData = (unsigned char*)aacSpec;
    ALOGD("%s: ==1===zhongjihao=====aacSpeccData[0]: %d, aacSpeccData[1]: %d",__FUNCTION__,aacSpecData[0],aacSpecData[1]);

    int ret = pRtmpH264->SendAacSpec(aacSpecData,(int)jlen);
    ALOGD("%s: ==2===zhongjihao=====ret: %d",__FUNCTION__,ret);
    env->ReleaseByteArrayElements(jaacSpec, aacSpec, 0);
    return ret;
}

JNIEXPORT jint JNICALL Java_com_android_rtmpvideo_RtmpJni_sendAacData(JNIEnv* env, jclass jcls, jbyteArray jaccFrame, jint jlen, jint jtimestamp)
{
    /*
     * 直播流的时间戳不论音频还是视频，在整体时间线上应当呈现递增趋势。如果时间戳计算方法是按照音视频分开计算，那么音频时戳和视频时戳可能并不是在一条时间线上，
     * 这就有可能出现音频时戳在某一个时间点比对应的视频时戳小， 在某一个时间点又跳变到比对应的视频时戳大，导致播放端无法对齐。
     * 目前采用的时间戳为底层发送RTMP包的时间，不区分音频流还是视频流，统一使用即将发送RTMP包的系统时间作为该包的时间戳。
     */
    jbyte* accFrame = env->GetByteArrayElements(jaccFrame, NULL);
    unsigned  char* accFrameData = (unsigned char*)accFrame;
    ALOGD("%s: ==0====zhongjihao=====len: %d",__FUNCTION__,(int)jlen);
    if((int)jlen>=7)
        ALOGD("%s: ==1====zhongjihao=====aac[0]: %d,aac[1]: %d, aac[2]: %d, aac[3]: %d, aac[4]: %d, aac[5]: %d,aac[6]: %d, timestamp: %d",__FUNCTION__,
                                     accFrameData[0],accFrameData[1],accFrameData[2],
                                     accFrameData[3],accFrameData[4],accFrameData[5],accFrameData[6],(int)jtimestamp);

    int ret = pRtmpH264->SendAacData(accFrameData,(int)jlen,(int)jtimestamp);
    ALOGD("%s: ==2====zhongjihao=====ret: %d",__FUNCTION__,ret);
    env->ReleaseByteArrayElements(jaccFrame, accFrame, 0);
    return ret;
}

JNIEXPORT jint JNICALL Java_com_android_rtmpvideo_RtmpJni_stopRtmp(JNIEnv* env,jclass jcls)
{
    ALOGD("%s: ==1===zhongjihao=====RTMP断开======",__FUNCTION__);
    //断开RTMP连接并释放相关资源
    pRtmpH264->RTMPAV_Close();
    if(pLogfile)
        fclose(pLogfile);
    pLogfile = NULL;
    ALOGD("%s: ==2===zhongjihao=====RTMP断开======",__FUNCTION__);
    return 0;
}