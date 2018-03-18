package com.android.rtmpvideo;

import android.Manifest;
import android.os.Build;
import android.os.Environment;
import android.support.v4.content.ContextCompat;
import android.content.pm.PackageManager;
import android.os.Bundle;
import android.support.v4.app.ActivityCompat;
import android.support.v7.app.AppCompatActivity;
import android.support.annotation.NonNull;
import android.util.Log;
import android.view.KeyEvent;
import android.view.SurfaceHolder;
import android.view.SurfaceView;
import android.view.View;
import android.view.Window;
import android.view.WindowManager;
import android.widget.Button;
import android.widget.Toast;

public class MainActivity extends AppCompatActivity implements View.OnClickListener, VideoGather.CameraOperateCallback{
    private final static String TAG = "MainActivity";
    private Button btnStart;
    private SurfaceView mSurfaceView;
    private SurfaceHolder mSurfaceHolder;
    private SurfacePreview mSurfacePreview;
    private MediaPublisher mediaPublisher;
    private boolean isStarted;
    private static final String rtmpUrl = "rtmp://192.168.1.101:1935/zhongjihao/myh264";

    // 要申请的权限
    private String[] permissions = {Manifest.permission.WRITE_EXTERNAL_STORAGE, Manifest.permission.CAMERA,
            Manifest.permission.RECORD_AUDIO};

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        // 设置全屏
        requestWindowFeature(Window.FEATURE_NO_TITLE);
        getWindow().setFlags(WindowManager.LayoutParams.FLAG_FULLSCREEN,
                WindowManager.LayoutParams.FLAG_FULLSCREEN);
        setContentView(R.layout.activity_main);

        isStarted = false;
        btnStart = (Button) findViewById(R.id.btn_start);
        mSurfaceView = (SurfaceView) findViewById(R.id.surface_view);
        mSurfaceView.setKeepScreenOn(true);
        // 获得SurfaceView的SurfaceHolder
        mSurfaceHolder = mSurfaceView.getHolder();
        // 设置surface不需要自己的维护缓存区
        mSurfaceHolder.setType(SurfaceHolder.SURFACE_TYPE_PUSH_BUFFERS);
        // 为srfaceHolder添加一个回调监听器
        mSurfacePreview = new SurfacePreview(this);
        mSurfaceHolder.addCallback(mSurfacePreview);
        btnStart.setOnClickListener(this);

        // 版本判断。当手机系统大于 23 时，才有必要去判断权限是否获取
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.M) {
            // 检查该权限是否已经获取
            boolean permission = false;
            for (int i = 0; i < permissions.length; i++) {
                int result = ContextCompat.checkSelfPermission(this, permissions[i]);
                // 权限是否已经 授权 GRANTED---授权  DINIED---拒绝
                if (result != PackageManager.PERMISSION_GRANTED) {
                    permission = false;
                    break;
                } else
                    permission = true;
            }
            if(!permission){
                // 如果没有授予权限，就去提示用户请求
                ActivityCompat.requestPermissions(this,
                        permissions, 100);
            }

        }
        String logPath = Environment
                .getExternalStorageDirectory()
                + "/" + "zhongjihao/rtmp.log";
        mediaPublisher = MediaPublisher.newInstance(rtmpUrl,logPath);
        mediaPublisher.initMediaPublish();
    }

    private void codecToggle() {
        if (isStarted) {
            isStarted = false;
            //停止音频采集
            mediaPublisher.stopAudioGather();
            //停止编码
            mediaPublisher. stopEncoder();
            //释放编码器
            mediaPublisher.release();
            //停止发布
            mediaPublisher.stopRtmpPublish();
        } else {
            isStarted = true;
//            //采集音频
//            mediaPublisher.startAudioGather();
//            //初始化音频编码器
//            mediaPublisher.initAudioEncoder();
            //启动编码
            mediaPublisher.startEncoder();
            //发布
            mediaPublisher.startRtmpPublish();

        }
        btnStart.setText(isStarted ? "停止" : "开始");
    }

    @Override
    protected void onDestroy() {
        super.onDestroy();
        if(isStarted){
            isStarted = false;
            //停止音频采集
            mediaPublisher.stopAudioGather();
            //停止编码
            mediaPublisher. stopEncoder();
            //释放编码器
            mediaPublisher.release();
            //停止发布
            mediaPublisher.stopRtmpPublish();
        }
        mediaPublisher = null;
        VideoGather.getInstance().doStopCamera();
    }

    @Override
    public boolean onKeyDown(int keyCode, KeyEvent event) {
        if ((keyCode == KeyEvent.KEYCODE_BACK)) {
            finish();
            return true;
        } else {
            return super.onKeyDown(keyCode, event);
        }
    }

    @Override
    public void onClick(View v) {
        int id = v.getId();
        switch (id) {
            case R.id.btn_start:
                codecToggle();
                break;
        }
    }

    @Override
    public void cameraHasOpened() {
        VideoGather.getInstance().doStartPreview(this, mSurfaceHolder);
    }

    @Override
    public void cameraHasPreview(int width,int height,int fps) {
        //初始化视频编码器
        mediaPublisher.initVideoEncoder(width,height,fps);
    }

    @Override
    public void onRequestPermissionsResult(int requestCode, @NonNull String[] permissions, @NonNull int[] grantResults) {
        super.onRequestPermissionsResult(requestCode, permissions, grantResults);

        if (requestCode == 100
                && permissions.length == 3
                && permissions[0].equals(Manifest.permission.WRITE_EXTERNAL_STORAGE)
                && permissions[1].equals(Manifest.permission.CAMERA)
                && permissions[2].equals(Manifest.permission.RECORD_AUDIO)
                && grantResults[0] ==PackageManager.PERMISSION_GRANTED
                && grantResults[1] == PackageManager.PERMISSION_GRANTED
                && grantResults[2] ==PackageManager.PERMISSION_GRANTED
                ) {

        }
    }


}
