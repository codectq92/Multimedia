package com.android.rtmpvideo;

import android.util.Log;

/**
 * Created by apadmin on 18-2-7.
 */

public class RtmpJni {
    static {
        Log.d("RtmpJni", "====zhongjihao====add lib RTMP  start===");
        System.loadLibrary("rtmpJni");
        Log.d("RtmpJni", "====zhongjihao=====add lib RTMP end===");
    }

    public static final native int initRtmp(String url, String logpath);

    public static final native int sendSpsAndPps(byte[] sps, int spsLen, byte[] pps, int ppsLen);

    public static final native int sendVideoFrame(byte[] frame, int len, int timestamp);

    public static final native int sendAacSpec(byte[] data, int len);

    public static final native int sendAacData(byte[] data, int len, int timestamp);

    public static final native int stopRtmp();
}
