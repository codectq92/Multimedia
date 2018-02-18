package com.android.rtmpvideo;

/**
 * Created by zhongjihao on 18-2-7.
 */

import android.media.MediaCodec;
import android.media.MediaCodecInfo;
import android.media.MediaCodecList;
import android.media.MediaFormat;
import android.os.Environment;
import android.util.Log;

import java.io.IOException;
import java.nio.ByteBuffer;
import java.util.Vector;

public class VideoRunnable extends Thread {
    private static final String TAG = "VideoRunnable";
    // parameters for the encoder
    private static final String MIME_TYPE = "video/avc"; // H.264 Advanced Video
    private static final int IFRAME_INTERVAL = 5; // 10 between
    // I-frames
    private static final int TIMEOUT_USEC = 10000;
    private static int BIT_RATE;
    public static boolean DEBUG = true;
    private final Object lock = new Object();
    //保存h264码流
    private byte[] mH264 = null;
    //保存sps帧
    private byte[] mSpsNalu = null;
    //保存pps帧
    private byte[] mPpsNalu = null;
    //转成后的数据
    private byte[] yuv420 = null;
    //旋转后的数据
    private byte[] rotateYuv420 = null;
    private Vector<byte[]> frameBytes;
    private int mWidth;
    private int mHeight;
    private MediaCodec mMediaCodec = null;
    private int mColorFormat;
    private volatile boolean isExit = false;
    private MediaFormat mediaFormat = null;
    private MediaCodecInfo codecInfo = null;
    private MediaCodec.BufferInfo mBufferInfo = null;
    private boolean isStartCodec = false;
    //pts时间基数
    private long presentationTimeUs = 0;

    public VideoRunnable(int mWidth, int mHeight) {
        this.mWidth = mWidth;
        this.mHeight = mHeight;
        frameBytes = new Vector<byte[]>();
        frameBytes.clear();
        BIT_RATE = (CameraWrapper.IMAGE_HEIGHT * CameraWrapper.IMAGE_WIDTH * 3 / 2) * 8 * CameraWrapper.FRAME_RATE;
        prepare();
    }

    private static int selectColorFormat(MediaCodecInfo codecInfo,
                                         String mimeType) {
        MediaCodecInfo.CodecCapabilities capabilities = codecInfo
                .getCapabilitiesForType(mimeType);
        for (int i = 0; i < capabilities.colorFormats.length; i++) {
            int colorFormat = capabilities.colorFormats[i];
            if (isRecognizedFormat(colorFormat)) {
                return colorFormat;
            }
        }
        if (DEBUG) Log.d(TAG,
                "=====zhongjihao=======couldn't find a good color format for " + codecInfo.getName()
                        + " / " + mimeType);
        return 0; // not reached
    }

    private static boolean isRecognizedFormat(int colorFormat) {
        switch (colorFormat) {
            // these are the formats we know how to handle for this test
            case MediaCodecInfo.CodecCapabilities.COLOR_FormatYUV420Planar:
            case MediaCodecInfo.CodecCapabilities.COLOR_FormatYUV420PackedPlanar:
            case MediaCodecInfo.CodecCapabilities.COLOR_FormatYUV420SemiPlanar:
            case MediaCodecInfo.CodecCapabilities.COLOR_FormatYUV420PackedSemiPlanar:
            case MediaCodecInfo.CodecCapabilities.COLOR_TI_FormatYUV420PackedSemiPlanar:
                return true;
            default:
                return false;
        }
    }

    private static MediaCodecInfo selectCodec(String mimeType) {
        int numCodecs = MediaCodecList.getCodecCount();
        for (int i = 0; i < numCodecs; i++) {
            MediaCodecInfo codecInfo = MediaCodecList.getCodecInfoAt(i);
            if (!codecInfo.isEncoder()) {
                continue;
            }
            String[] types = codecInfo.getSupportedTypes();
            for (int j = 0; j < types.length; j++) {
                if (types[j].equalsIgnoreCase(mimeType)) {
                    return codecInfo;
                }
            }
        }
        return null;
    }

    public void exit() {
        isExit = true;
        frameBytes.clear();
        frameBytes = null;
        synchronized (lock) {
            lock.notify();
        }
        if (DEBUG) Log.d(TAG, "=====zhongjihao=========Video 编码开始退出 isStart: " + isStartCodec);
    }

    public void add(byte[] data) {
        if (frameBytes != null) {
            frameBytes.add(data);
            synchronized (lock) {
                lock.notify();
            }
        }
    }

    private int getYuvBuffer(int width, int height) {
        int yStride = (int) Math.ceil(width / 16.0) * 16;
        int uvStride = (int) Math.ceil( (yStride / 2) / 16.0) * 16;
        int ySize = yStride * height;
        int uvSize = uvStride * height / 2;
        return ySize + uvSize * 2;
    }

    private void prepare() {
        mH264 = new byte[getYuvBuffer(mWidth, mHeight)];
//        rotateYuv420 = new byte[this.mWidth * this.mHeight * 3 / 2];
//        yuv420 = new byte[this.mWidth * this.mHeight * 3 / 2];
        yuv420 = new byte[getYuvBuffer(mWidth, mHeight)];
        rotateYuv420 = new byte[getYuvBuffer(mWidth, mHeight)];
        Log.d(TAG, "===zhongjihao=====prepare()====width: "+mWidth+"  height: "+mHeight+"  bufferSize: "+getYuvBuffer(mWidth, mHeight));
        mBufferInfo = new MediaCodec.BufferInfo();
        //选择系统用于编码H264的编码器信息
        codecInfo = selectCodec(MIME_TYPE);
        if (codecInfo == null) {
            Log.e(TAG, "====zhongjihao=====Unable to find an appropriate codec for " + MIME_TYPE);
            return;
        }

        Log.d(TAG, "======zhongjihao====found codec: " + codecInfo.getName());
        //根据MIME格式,选择颜色格式
        mColorFormat = selectColorFormat(codecInfo, MIME_TYPE);

        Log.d(TAG, "=====zhongjihao====found colorFormat: " + mColorFormat);
        //根据MIME创建MediaFormat
        // sensor出来的是逆时针旋转90度的数据，hal层没有做旋转导致APP显示和编码需要自己做顺时针旋转90,这样看到的图像才是正常的
        mediaFormat = MediaFormat.createVideoFormat(MIME_TYPE,
                this.mHeight, this.mWidth);
        //设置比特率
        mediaFormat.setInteger(MediaFormat.KEY_BIT_RATE, BIT_RATE);
        //设置帧率
        mediaFormat.setInteger(MediaFormat.KEY_FRAME_RATE, CameraWrapper.FRAME_RATE);
        //设置颜色格式
        mediaFormat.setInteger(MediaFormat.KEY_COLOR_FORMAT, mColorFormat);
        //设置关键帧的时间
        mediaFormat.setInteger(MediaFormat.KEY_I_FRAME_INTERVAL, IFRAME_INTERVAL);

        Log.d(TAG, "=====zhongjihao=======format: " + mediaFormat);
    }

    private void startMediaCodec() throws IOException {
        //创建一个MediaCodec
        mMediaCodec = MediaCodec.createByCodecName(codecInfo.getName());
        mMediaCodec.configure(mediaFormat, null, null,
                MediaCodec.CONFIGURE_FLAG_ENCODE);
        mMediaCodec.start();

        isStartCodec = true;
    }

    private void stopMediaCodec() {
        if (mMediaCodec != null) {
            mMediaCodec.stop();
            mMediaCodec.release();
            mMediaCodec = null;
        }
        isStartCodec = false;
        Log.d(TAG, "======zhongjihao======stop video 编码...");
    }

    //这个参数input就是上面回调拿到的原始数据
    private void encodeFrame(byte[] input) {
        if (DEBUG)
            Log.d(TAG, "=====zhongjihao=====encodeFrame()========");
        int pos = 0;
        if(mColorFormat == MediaCodecInfo.CodecCapabilities.COLOR_FormatYUV420SemiPlanar){
            //nv21格式转为nv12格式
            Yuv420POperate.NV21ToNV12(input,yuv420,mWidth,mHeight);
        }else if(mColorFormat == MediaCodecInfo.CodecCapabilities.COLOR_FormatYUV420Planar){
            //用于NV21格式转换为YUV420P格式
            Yuv420POperate.NV21toYUV420P(input, yuv420, mWidth, mHeight);
        }
        Yuv420POperate.YUV420PClockRot90(rotateYuv420, yuv420, mWidth, mHeight);
        try {
            //拿到输入缓冲区,用于传送数据进行编码
            ByteBuffer[] inputBuffers = mMediaCodec.getInputBuffers();
            //得到当前有效的输入缓冲区的索引
            int inputBufferIndex = mMediaCodec.dequeueInputBuffer(TIMEOUT_USEC);
            if (inputBufferIndex >= 0) { //输入缓冲区有效
                if (DEBUG) Log.d(TAG, "======zhongjihao======inputBufferIndex: " + inputBufferIndex);
                ByteBuffer inputBuffer = inputBuffers[inputBufferIndex];
                inputBuffer.clear();
                //往输入缓冲区写入数据
                inputBuffer.put(rotateYuv420);

                //计算pts，这个值是一定要设置的
                long pts = computePresentationTime(presentationTimeUs);
                if (isExit) {
                    //结束时，发送结束标志，在编码完成后结束
                    if (DEBUG) Log.d(TAG, "=====zhongjihao======send BUFFER_FLAG_END_OF_STREAM");
                    mMediaCodec.queueInputBuffer(inputBufferIndex, 0, rotateYuv420.length,
                            pts, MediaCodec.BUFFER_FLAG_END_OF_STREAM);
                } else {
                    //将缓冲区入队
                    mMediaCodec.queueInputBuffer(inputBufferIndex, 0, rotateYuv420.length,
                            pts, 0);
                }
                presentationTimeUs += 1;
            } else {
                // either all in use, or we timed out during initial setup
                if (DEBUG) Log.d(TAG, "====zhongjihao=====input buffer not available");
            }

            //拿到输出缓冲区,用于取到编码后的数据
            ByteBuffer[] outputBuffers = mMediaCodec.getOutputBuffers();
            //拿到输出缓冲区的索引
            int outputBufferIndex = mMediaCodec.dequeueOutputBuffer(mBufferInfo, TIMEOUT_USEC);
            while (outputBufferIndex >= 0) {
                Log.d(TAG, "=====zhongjihao====outputBufferIndex: " + outputBufferIndex);
                //数据已经编码成H264格式
                //outputBuffer保存的就是H264数据
                ByteBuffer outputBuffer = outputBuffers[outputBufferIndex];
                if (outputBuffer == null) {
                    throw new RuntimeException("encoderOutputBuffer " + outputBufferIndex +
                            " was null");
                }

                if (mBufferInfo.size != 0) {
                    byte[] outData = new byte[mBufferInfo.size];
                    outputBuffer.get(outData);

                    // 0 0 0 1 103 66 -128 41 -38 15 10 104 6 -48 -95 53  0 0 0 1 104 -50 6 -30
                    //sps序列参数集，即0x67 pps图像参数集，即0x68，MediaCodec编码输出的头两个NALU即为sps和pps
                    //并且在h264码流的开始两帧即为sps和pps，在这里MediaCodec将sps和pps作为一个buffer输出。
                    if (mSpsNalu != null && mPpsNalu != null) {
                        System.arraycopy(outData, 0, mH264, pos, outData.length);
                        pos += outData.length;
                    } else {
                        //保存pps sps 即h264码流开始两帧，保存起来后面用
                        ByteBuffer spsPpsBuffer = ByteBuffer.wrap(outData);
                        if (spsPpsBuffer.getInt() == 0x00000001 && (spsPpsBuffer.get(4) == 0x67)) {
                            //通过上面的打印看到sps帧长度为输出buffer的前面12字节
                            mSpsNalu = new byte[outData.length - 4 - 8]; //8为两个startCode的长度，一个startCode为0x00000001
                            //通过上面的打印看到pps帧长度为输出buffer的最后4字节
                            mPpsNalu = new byte[4];
                            //保存sps帧
                            spsPpsBuffer.get(mSpsNalu, 0, mSpsNalu.length);
                            //跳过startCode 0x00000001
                            spsPpsBuffer.getInt();
                            //保存pps帧
                            spsPpsBuffer.get(mPpsNalu, 0, mPpsNalu.length);
                            Log.d(TAG, "=====zhongjihao=====1==sps==pps==:" + outData.length);
                            for (int i = 0; i < outData.length; i++) {
                                Log.d(TAG, "=====zhongjihao===2==sps==pps==:" + outData[i]);
                            }
                            Log.d(TAG, "=====zhongjihao====3==sps==pps==:" + outData.length);
                        }
                    }
                }
                //释放资源
                mMediaCodec.releaseOutputBuffer(outputBufferIndex, false);
                //拿到输出缓冲区的索引
                outputBufferIndex = mMediaCodec.dequeueOutputBuffer(mBufferInfo, 0);
                //编码结束的标志
                if ((mBufferInfo.flags & MediaCodec.BUFFER_FLAG_END_OF_STREAM) != 0) {
                    return;
                }
            }

            if (mH264[0] == 0 && mH264[1] == 0 && mH264[2] == 0 && mH264[3] == 1 && mH264[4] == 0x65) {//IDR帧
                Log.d(TAG, "=====zhongjihao=====IDR帧==========");
                //推流h264的IDR帧时一定要先推送pps 、sps
                RtmpH264.sendSpsAndPps(mSpsNalu, mSpsNalu.length, mPpsNalu, mPpsNalu.length);
            }
            if(pos > 0){
                Log.d(TAG, "=====zhongjihao=====NALU 帧大小====pos: "+pos+"   时间戳: "+mBufferInfo.presentationTimeUs / 1000);
                RtmpH264.sendVideoFrame(mH264, pos, (int)(mBufferInfo.presentationTimeUs / 1000));
            }
        } catch (Throwable t) {
            Log.e(TAG, "========zhongjihao====error: " + t.getMessage());
        }
    }

    @Override
    public void run() {
        if (DEBUG) Log.d(TAG, "====zhongjihao====连接RTMP服务器=====");
        String logPath = Environment
                .getExternalStorageDirectory()
                + "/" + "zhongjihao/rtmp.log";
        RtmpH264.initRtmp("rtmp://192.168.1.103:1935/zhongjihao/myh264", logPath);
        while (!isExit) {
            if (!isStartCodec) {
                stopMediaCodec();
                try {
                    if (DEBUG) Log.d(TAG, "=====zhongjihao======video -- startMediaCodec...");
                    startMediaCodec();
                } catch (IOException e) {
                    isStartCodec = false;
                }
            } else if (!frameBytes.isEmpty()) {
                byte[] bytes = frameBytes.remove(0);
                if (DEBUG) Log.d(TAG, "======zhongjihao====编码视频数据:" + bytes.length);
                try {
                    encodeFrame(bytes);
                } catch (Exception e) {
                    if (DEBUG) Log.e(TAG, "===zhongjihao==========编码视频(Video)数据 失败");
                    e.printStackTrace();
                }
            }
            else if (frameBytes.isEmpty()) {
                synchronized (lock) {
                    try {
                        if (DEBUG) Log.d(TAG, "===zhongjihao=======video -- 等待数据编码...");
                        lock.wait();
                    } catch (InterruptedException e) {
                    }
                }
            }
        }
        stopMediaCodec();
        if (DEBUG) Log.d(TAG, "====zhongjihao====断开RTMP服务器=====");
        RtmpH264.stopRtmp();
        if (DEBUG) Log.d(TAG, "=====zhongjihao=========Video 编码线程 退出...");
    }

    /**
     * 计算视频pts,单位微秒
     */
    private long computePresentationTime(long frameIndex) {
        return 132 + frameIndex * 1000000 / CameraWrapper.FRAME_RATE;
    }
}
