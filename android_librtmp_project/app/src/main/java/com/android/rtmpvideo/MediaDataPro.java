package com.android.rtmpvideo;

/**
 * Created by zhongjihao on 18-1-31.
 */

import android.util.Log;

public class MediaDataPro {
    private final static String TAG = "MediaDataPro";
    public static boolean DEBUG = true;
    private static MediaDataPro mediaPro;

    //   private AudioRunnable audioThread;
    private VideoRunnable videoThread;


    private MediaDataPro() {
        videoThread = new VideoRunnable(CameraWrapper.IMAGE_WIDTH, CameraWrapper.IMAGE_HEIGHT);
        videoThread.start();
    }

    public static void startAVThread() {
        if (mediaPro == null) {
            synchronized (MediaDataPro.class) {
                if (mediaPro == null) {
                    mediaPro = new MediaDataPro();
                }
            }
        }
    }

    public static void stopAVThread() {
        if (mediaPro != null) {
            mediaPro.exit();
            mediaPro = null;
        }
    }

    public static void addVideoFrameData(byte[] data) {
        if (mediaPro != null) {
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
