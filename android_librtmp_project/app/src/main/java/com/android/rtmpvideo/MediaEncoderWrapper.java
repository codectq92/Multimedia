package com.android.rtmpvideo;

/**
 * Created by zhongjihao on 18-1-31.
 */

import android.os.Environment;
import android.util.Log;

public class MediaEncoderWrapper {
    private final static String TAG = "MediaEncoderWrapper";
    public static final String rtmpUrl = "rtmp://192.168.1.101:1935/zhongjihao/myh264";
    public static boolean DEBUG = true;
    private volatile boolean isExit = false;
    private static MediaEncoderWrapper mediaPro;

    //   private AudioRunnable audioThread;
    private AvcEncoderRunnable videoThread;


    private MediaEncoderWrapper() {
        isExit = false;
        videoThread = new AvcEncoderRunnable(VideoGather.IMAGE_WIDTH, VideoGather.IMAGE_HEIGHT);
        videoThread.start();
    }

    public static int startAVThread() {
        if (mediaPro == null) {
            synchronized (MediaEncoderWrapper.class) {
                if (mediaPro == null) {
                    mediaPro = new MediaEncoderWrapper();
                }
            }
        }

        String logPath = Environment
                .getExternalStorageDirectory()
                + "/" + "zhongjihao/rtmp.log";
        if (DEBUG) Log.d(TAG, "====zhongjihao====连接RTMP服务器=====");
        return RtmpH264.initRtmp(rtmpUrl, logPath);
    }

    public static void stopAVThread() {
        if (mediaPro != null) {
            mediaPro.isExit = true;
            mediaPro.exit();
            mediaPro = null;
        }
        Log.d(TAG, "====zhongjihao====断开RTMP服务器=====");
        RtmpH264.stopRtmp();
    }

    public static void addVideoFrameData(byte[] data) {
        if (mediaPro != null) {
            if(!mediaPro.isExit)
                mediaPro.addVideoData(data);
        }
    }


    private void addVideoData(byte[] data) {
        if (videoThread != null) {
            videoThread.add(data);
        }
    }

    private void exit() {
        if (videoThread != null) {
            videoThread.exit();
            try {
                if (DEBUG) Log.d(TAG, "====zhongjihao===等待Video视频编码线程结束开始");
                videoThread.join();
                videoThread = null;
                if (DEBUG) Log.d(TAG, "====zhongjihao===等待Video视频编码线程结束完成");
            } catch (InterruptedException e) {
            }
        }
    }

}
