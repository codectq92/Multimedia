package com.android.rtmpvideo;

/**
 * Created by zhongjihao on 18-1-31.
 */

import android.os.Environment;
import android.util.Log;

import java.util.Date;

public class MediaEncoderWrapper implements AudioGather.Callback,VideoGather.Callback{
    private final static String TAG = "MediaEncoderWrapper";
    public static final String rtmpUrl = "rtmp://192.168.1.101:1935/zhongjihao/myh264";
    public static boolean DEBUG = true;
    private volatile boolean isExit = false;
    private static MediaEncoderWrapper mediaPro;

    private AacEncoderRunnable aacEncThread;
    private AvcEncoderRunnable avcEncThread;

    //pts时间基数
    public long presentationTimeUs = 0;


    private MediaEncoderWrapper() {
        isExit = false;
        presentationTimeUs = new Date().getTime() * 1000;
    }

    public static MediaEncoderWrapper getMediaEncWrapInstance(){
        if (mediaPro == null) {
            synchronized (MediaEncoderWrapper.class) {
                if (mediaPro == null) {
                    mediaPro = new MediaEncoderWrapper();
                }
            }
        }
        return mediaPro;
    }

    public static int startAVThread() {
        String logPath = Environment
                .getExternalStorageDirectory()
                + "/" + "zhongjihao/rtmp.log";
        if (DEBUG) Log.d(TAG, "====zhongjihao====连接RTMP服务器=====");
        int ret =  RtmpJni.initRtmp(rtmpUrl, logPath);
        VideoGather.getInstance().setCallback(mediaPro);
        AudioGather.getInstance().setCallback(mediaPro);
        //启动视频编码线程
        mediaPro.avcEncThread = new AvcEncoderRunnable(VideoGather.IMAGE_WIDTH, VideoGather.IMAGE_HEIGHT);
        mediaPro.avcEncThread.start();
        //启动录音线程
        AudioGather.getInstance().start();
        //启动音频编码线程
        mediaPro.aacEncThread = new AacEncoderRunnable();
        mediaPro.aacEncThread.start();

       return ret;
    }

    public static void stopAVThread() {
        if (mediaPro != null) {
            mediaPro.isExit = true;
            mediaPro.exit();
            mediaPro = null;
        }
        VideoGather.getInstance().setCallback(null);
        AudioGather.getInstance().setCallback(null);
        AudioGather.getInstance().stop();
        AudioGather.getInstance().release();
        Log.d(TAG, "====zhongjihao====断开RTMP服务器=====");
        RtmpJni.stopRtmp();
    }

//    public static void addVideoFrameData(byte[] data) {
//        if (mediaPro != null) {
//            if(!mediaPro.isExit)
//                mediaPro.addVideoData(data);
//        }
//    }

    private void addVideoData(byte[] data) {
        if (avcEncThread != null) {
            avcEncThread.add(data);
        }
    }

//    public static void addAudioFrameData(byte[] data) {
//        if (mediaPro != null) {
//            if(!mediaPro.isExit)
//                mediaPro.addAudioData(data);
//        }
//    }

    private void addAudioData(byte[] data) {
        if (aacEncThread != null) {
            aacEncThread.add(data);
        }
    }

    private void exit() {
        if (avcEncThread != null) {
            avcEncThread.exit();
            try {
                if (DEBUG) Log.d(TAG, "====zhongjihao===等待Video视频编码线程结束开始");
                avcEncThread.join();
                avcEncThread = null;
                if (DEBUG) Log.d(TAG, "====zhongjihao===等待Video视频编码线程结束完成");
            } catch (InterruptedException e) {
            }
        }

        if (aacEncThread != null) {
            aacEncThread.exit();
            try {
                if (DEBUG) Log.d(TAG, "====zhongjihao===等待Audio编码线程结束开始");
                aacEncThread.join();
                aacEncThread = null;
                if (DEBUG) Log.d(TAG, "====zhongjihao===等待Audio编码线程结束完成");
            } catch (InterruptedException e) {
            }
        }
    }

    @Override
    public void audioData(byte[] data){
        if (mediaPro != null) {
            if(!mediaPro.isExit)
                mediaPro.addAudioData(data);
        }
    }

    @Override
    public void videoData(byte[] data){
        if (mediaPro != null) {
            if(!mediaPro.isExit)
                mediaPro.addVideoData(data);
        }
    }

}
