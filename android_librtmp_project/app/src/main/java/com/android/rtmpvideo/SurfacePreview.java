package com.android.rtmpvideo;

/**
 * Created by zhongjihao on 18-2-7.
 */
import android.view.SurfaceHolder;
import android.util.Log;

public class SurfacePreview  implements SurfaceHolder.Callback{
    private final static String TAG = "SurfacePreview";
    private VideoGather.CamOpenOverCallback mCallback;

    public SurfacePreview(VideoGather.CamOpenOverCallback cb){
        mCallback = cb;
    }

    @Override
    public void surfaceDestroyed(SurfaceHolder arg0) {
        Log.d(TAG, "======zhongjihao=====surfaceDestroyed()====");
        VideoGather.getInstance().doStopCamera();
    }

    @Override
    public void surfaceCreated(SurfaceHolder arg0) {
        Log.d(TAG, "======zhongjihao=====surfaceCreated()====");
        // 打开摄像头
        VideoGather.getInstance().doOpenCamera(mCallback);
    }

    @Override
    public void surfaceChanged(SurfaceHolder arg0, int arg1, int arg2, int arg3) {
        Log.d(TAG, "======zhongjihao=====surfaceChanged()====");
        // 打开摄像头
        VideoGather.getInstance().doOpenCamera(mCallback);
    }

}
