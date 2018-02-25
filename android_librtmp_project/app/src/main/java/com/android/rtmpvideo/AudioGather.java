package com.android.rtmpvideo;

/**
 * Created by zhongjihao on 23/02/18.
 */

import android.media.AudioFormat;
import android.media.AudioRecord;
import android.media.MediaRecorder;
import android.util.Log;

import java.nio.ByteBuffer;

public class AudioGather {
    private static final String TAG = "AudioGather";
    private static AudioGather mAudioGather;
    public static int audioBufferSize = 1024;
    private AudioRecord audioRecord;
    private int aChannelCount;
    private int aSampleRate;
    private ByteBuffer audioBuf = null;

    private Thread workThread;
    private volatile boolean loop = false;
    private Callback mCallback;

    public static AudioGather getInstance() {
        if (mAudioGather == null) {
            synchronized (AudioGather.class) {
                if (mAudioGather == null) {
                    mAudioGather = new AudioGather();
                }
            }
        }
        return mAudioGather;
    }

    private AudioGather() {

    }

    public void prepareAudioRecord() {
        if (audioRecord != null) {
            audioRecord.stop();
            audioRecord.release();
            audioRecord = null;
        }
        //音频采样率，44100是目前的标准，但是某些设备仍然支持22050,16000,11025,8000,4000
        int[] sampleRates = {44100, 22050, 16000, 11025, 8000, 4000};
        android.os.Process.setThreadPriority(android.os.Process.THREAD_PRIORITY_URGENT_AUDIO);
        try {
            for (int sampleRate : sampleRates) {
                //编码制式PCM
                int audioForamt = AudioFormat.ENCODING_PCM_16BIT;
                // stereo 立体声,mono单声道
                int channelConfig = AudioFormat.CHANNEL_CONFIGURATION_STEREO;

                final int min_buffer_size = 2 * AudioRecord.getMinBufferSize(sampleRate, channelConfig, audioForamt);
                audioRecord = new AudioRecord(MediaRecorder.AudioSource.MIC, sampleRate, channelConfig, AudioFormat.ENCODING_PCM_16BIT, min_buffer_size);
                if (audioRecord.getState() != AudioRecord.STATE_INITIALIZED) {
                    audioRecord = null;
                    Log.e(TAG, "====zhongjihao===initialized the mic failed");
                    continue;
                }

                aSampleRate = sampleRate;
                aChannelCount = channelConfig == AudioFormat.CHANNEL_CONFIGURATION_STEREO ? 2 : 1;
                //ByteBuffer分配内存的最大值为4096
                audioBufferSize = Math.min(4096, min_buffer_size);
                audioBuf = ByteBuffer.allocateDirect(audioBufferSize);
                Log.d(TAG, "====zhongjihao===aSampleRate: " + aSampleRate + "   aChannelCount: " + aChannelCount + "   min_buffer_size: " + min_buffer_size);
                break;
            }
        } catch (final Exception e) {
            Log.e(TAG, "AudioThread#run", e);
        }
    }

    /**
     * 开始录音
     */
    public void start() {
        workThread = new Thread() {
            @Override
            public void run() {
                if (audioRecord != null) {
                    audioRecord.startRecording();
                }
                while (loop && !Thread.interrupted()) {
                    audioBuf.clear();
                    //读取音频数据到buf
                    int size = audioRecord.read(audioBuf, audioBufferSize);
                    if (size > 0) {
                        // set audio data to encoder
                        audioBuf.position(size);
                        audioBuf.flip();
                        Log.d(TAG, "======zhongjihao========录音字节数:" + size);
                        if (mCallback != null) {
                            mCallback.audioData(audioBuf);
                        }
                    }
                }
                Log.d(TAG, "=====zhongjihao======Audio录音线程退出...");
            }
        };

        loop = true;
        workThread.start();
    }

    public void stop() {
        loop = false;
        workThread.interrupt();
        Log.d(TAG, "run: ===zhongjihao====调用stop");
        audioRecord.stop();
    }

    public void release() {
        audioRecord.release();
    }

    public void setCallback(Callback callback) {
        this.mCallback = callback;
    }

    public interface Callback {
        void audioData(ByteBuffer data);
    }
}
