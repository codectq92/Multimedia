package com.android.rtmpvideo;

import android.util.Log;

/**
 * Created by apadmin on 18-2-7.
 */

public class RtmpH264 {
    static {
        Log.d("RtmpH264", "====zhongjihao====add lib RTMP  start===");
        System.loadLibrary("rtmpJni");
        Log.d("RtmpH264", "====zhongjihao=====add lib RTMP end===");
    }

    public static final native int initRtmp(String url, String logpath);

    public static final native int sendSpsAndPps(byte[] sps, int spsLen, byte[] pps, int ppsLen);

    public static final native int sendVideoFrame(byte[] frame, int len, int time);

    public static final native int stopRtmp();
}
