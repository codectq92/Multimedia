package com.android.rtmpvideo;

import android.media.AudioFormat;
import android.media.MediaCodec;
import android.media.MediaCodecInfo;
import android.media.MediaCodecList;
import android.media.MediaFormat;
import android.util.Log;

import java.io.IOException;
import java.nio.ByteBuffer;
import java.util.Vector;
import java.util.Date;

/**
 * Created by zhongjihao on 23/02/18.
 */

public class AacEncoderRunnable extends Thread{
    private static final String TAG = "AacEncoderRunnable";
    public static boolean DEBUG = true;
    private static final int TIMEOUT_USEC = 10000;
    // parameters for the encoder
    private static final String MIME_TYPE = "audio/mp4a-latm";
    private static int SAMPLE_RATE = 44100;    // 44.1[KHz] is only setting guaranteed to be available on all devices.
    private static int BIT_RATE = 64000;
    private MediaCodec mMediaCodec;                // API >= 16(Android4.1.2)
    private volatile boolean isExit = false;
    private MediaCodec.BufferInfo mBufferInfo;        // API >= 16(Android4.1.2)
    private MediaCodecInfo audioCodecInfo;
    private boolean mStartCodecFlag = false;
    private Vector<byte[]> frameBytes;
    private final Object lock = new Object();

    /**
     * previous presentationTimeUs for writing
     */
    private long presentationTimeUs = 0;
    private MediaFormat audioFormat = null;

    public AacEncoderRunnable() {
        mBufferInfo = new MediaCodec.BufferInfo();
        frameBytes = new Vector<byte[]>();
        frameBytes.clear();
        prepare();
    }

    private static MediaCodecInfo selectCodec(String mimeType) {
        // get the list of available codecs
        int numCodecs = MediaCodecList.getCodecCount();
        for (int i = 0; i < numCodecs; i++) {
            MediaCodecInfo codecInfo = MediaCodecList.getCodecInfoAt(i);
            if (!codecInfo.isEncoder()) {  // skipp decoder
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
        //frameBytes = null;
        synchronized (lock) {
            lock.notify();
        }
        Log.d(TAG, "=====zhongjihao=========Audio 编码开始退出 isStart: " + mStartCodecFlag);
    }

    public synchronized void add(byte[] data) {
        if (frameBytes != null) {
            frameBytes.add(data);
            synchronized (lock) {
                lock.notify();
            }
        }
    }

    private void prepare() {
        audioCodecInfo = selectCodec(MIME_TYPE);
        if (audioCodecInfo == null) {
            if (DEBUG) Log.e(TAG, "=====zhongjihao====Unable to find an appropriate codec for " + MIME_TYPE);
            return;
        }
        Log.d(TAG, "===zhongjihao===selected codec: " + audioCodecInfo.getName());

        audioFormat = MediaFormat.createAudioFormat(MIME_TYPE, SAMPLE_RATE, 1);
        audioFormat.setInteger(MediaFormat.KEY_AAC_PROFILE, MediaCodecInfo.CodecProfileLevel.AACObjectLC);
        audioFormat.setInteger(MediaFormat.KEY_CHANNEL_MASK, AudioFormat.CHANNEL_IN_STEREO);//CHANNEL_IN_STEREO 立体声
        audioFormat.setInteger(MediaFormat.KEY_BIT_RATE, BIT_RATE);
        audioFormat.setInteger(MediaFormat.KEY_CHANNEL_COUNT, 2);
        audioFormat.setInteger(MediaFormat.KEY_SAMPLE_RATE, SAMPLE_RATE);
        Log.d(TAG, "====zhongjihao=========format: " + audioFormat.toString());
    }

    private void startMediaCodec() throws IOException {
        if (mMediaCodec != null) {
            return;
        }

        try {
            mMediaCodec = MediaCodec.createEncoderByType(MIME_TYPE);
        } catch (IOException e) {
            e.printStackTrace();
            throw new RuntimeException("===zhongjihao===初始化音频编码器失败", e);
        }
        Log.d(TAG, String.format("=====zhongjihao=====编码器:%s创建完成", mMediaCodec.getName()));
        BIT_RATE = AudioGather.getInstance().aSampleRate * AudioGather.getInstance().audioForamt * AudioGather.getInstance().aChannelCount;
        SAMPLE_RATE =  AudioGather.getInstance().aSampleRate;
        audioFormat.setInteger(MediaFormat.KEY_BIT_RATE, BIT_RATE);
        audioFormat.setInteger(MediaFormat.KEY_CHANNEL_COUNT, AudioGather.getInstance().aChannelCount);
        audioFormat.setInteger(MediaFormat.KEY_SAMPLE_RATE, SAMPLE_RATE);
        mMediaCodec.configure(audioFormat, null, null, MediaCodec.CONFIGURE_FLAG_ENCODE);
      //  MediaEncoderWrapper.getMediaEncWrapInstance().presentationTimeUs = System.currentTimeMillis() * 1000;
        mMediaCodec.start();
        Log.d(TAG, "===zhongjihao=====Audio 编码器 start finishing===format: "+audioFormat.toString());

        mStartCodecFlag = true;
    }

    private void stopMediaCodec() {
        if (mMediaCodec != null) {
            mMediaCodec.stop();
            mMediaCodec.release();
            mMediaCodec = null;
        }
        mStartCodecFlag = false;
        Log.d(TAG, "======zhongjihao======stop Audio 编码...");
    }

    //这个参数input就是原始PCM数据
    private void onGetPcmFrame(byte[] input) {
        Log.d(TAG, "=====zhongjihao=====onGetPcmFrame()========");
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
                inputBuffer.put(input);

                //计算pts，这个值是一定要设置的
               // long pts = computePresentationTime(presentationTimeUs);
                long pts = new Date().getTime() * 1000 - MediaEncoderWrapper.getMediaEncWrapInstance().presentationTimeUs;
                if (isExit) {
                    //结束时，发送结束标志，在编码完成后结束
                    if (DEBUG) Log.d(TAG, "=====zhongjihao======send BUFFER_FLAG_END_OF_STREAM");
                    mMediaCodec.queueInputBuffer(inputBufferIndex, 0, input.length,
                            pts, MediaCodec.BUFFER_FLAG_END_OF_STREAM);
                } else {
                    //将缓冲区入队
                    mMediaCodec.queueInputBuffer(inputBufferIndex, 0, input.length,
                            pts, 0);
                }
              //  presentationTimeUs += 1;
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
                //数据已经编码成AAC格式
                //outputBuffer保存的就是AAC数据
                ByteBuffer outputBuffer = outputBuffers[outputBufferIndex];
                if (outputBuffer == null) {
                    throw new RuntimeException("encoderOutputBuffer " + outputBufferIndex +
                            " was null");
                }

                if (mBufferInfo.size != 0) {
                   // byte[] outData = new byte[mBufferInfo.size];
                   // outputBuffer.get(outData);
                    onEncodeAacFrame(outputBuffer,mBufferInfo);
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
        } catch (Throwable t) {
            Log.e(TAG, "========zhongjihao====error: " + t.getMessage());
        }
    }

    private void onEncodeAacFrame(ByteBuffer bb, MediaCodec.BufferInfo aBufferInfo) {
        Log.d(TAG, "======zhongjihao====aBufferInfo.size: " + aBufferInfo.size);
        if (aBufferInfo.size == 2) {
            byte[] bytes = new byte[2];
            bb.get(bytes);
            Log.d(TAG, "======zhongjihao====bytes[0]: " + bytes[0] + "    bytes[1]: " + bytes[1]);
            RtmpJni.sendAacSpec(bytes, 2);
        } else {
            byte[] bytes = new byte[aBufferInfo.size];
            bb.get(bytes);
            if(aBufferInfo.size >=7){
                Log.d(TAG, "======zhongjihao====bytes[0]: " + bytes[0] + "   bytes[1]: " + bytes[1]
                        + "   bytes[2]: " + bytes[2]
                        + "    bytes[3]: " + bytes[3]
                        + "    bytes[4]: " + bytes[4]
                        + "    bytes[5]: " + bytes[5]
                        + "    bytes[6]: " + bytes[6]);
            }

            RtmpJni.sendAacData(bytes, bytes.length, (int) aBufferInfo.presentationTimeUs / 1000);
        }
    }

    @Override
    public void run() {
        while (!isExit) {
            if (!mStartCodecFlag) {
                stopMediaCodec();
                try {
                    if (DEBUG) Log.d(TAG, "=====zhongjihao======Audio -- startMediaCodec...");
                    startMediaCodec();
                } catch (IOException e) {
                    mStartCodecFlag = false;
                }
            } else if (!frameBytes.isEmpty()) {
                byte[] bytes = frameBytes.remove(0);
                if (DEBUG) Log.d(TAG, "======zhongjihao====编码Audio数据大小:" + bytes.length);
                try {
                    onGetPcmFrame(bytes);
                } catch (Exception e) {
                    Log.e(TAG, "===zhongjihao==========编码(Audio)数据 失败");
                    e.printStackTrace();
                }
            } else if (frameBytes.isEmpty()) {
                synchronized (lock) {
                    try {
                        if (DEBUG) Log.d(TAG, "===zhongjihao=======Audio -- 等待数据编码...");
                        lock.wait();
                    } catch (InterruptedException e) {
                    }
                }
            }
        }
        stopMediaCodec();
        Log.d(TAG, "=====zhongjihao=========Audio 编码线程 退出...");
    }

    /**
     * 计算音频pts,单位微秒
     * 时间戳间隔为：presentation_time  = frame_size/sample_rate;
     * frame_size：每帧数据对应的字节数
     * sample_rate：采样率，是指将模拟声音波形进行数字化时，每秒钟抽取声波幅度样本的次数
     * presentation_time：时间间隔，也就是该帧数据播放的时间长度，单位s，如果用毫秒为单位，乘上1000即可
     * presentation_time  = frame_size*1000/sample_rate;
     *
     * 例如：AAC每帧数据对应的字节数为1024，如果sample_rate==32K，对应的时间间隔为1024*1000/32000 = 32ms
     * mp3每帧数据对应的字节数为1152 ，如果smple_rate==8k，对应的时间间隔为1152*1000/8000 = 144ms
     */
    private long computePresentationTime(long frameIndex) {
        return 132 + frameIndex * 1024*1000000 /SAMPLE_RATE;
    }
}
