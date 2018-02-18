本工程采用Git管理，通过git log可以查看提交记录

本工程包含如下几个部分：
  RGB、YUV像素数据处理
  PCM音频采样数据处理
  H.264视频码流解析
  AAC音频码流解析
  FLV封装格式解析
  UDP-RTP协议解析

YUV图片播放命令
  ffplay -f rawvideo -video_size 1920x1080 input.yuv
  1920x1080代表图片分辨率   input.yuv代表查看的图片

PCM音频数据可以使用音频编辑软件导入查看。例如免费开源的音频编辑软件Audacity。Ubuntu16.04 软件中心安装即可。

Ubuntu16.0.4下播放H264裸流文件
  1 在软件中心搜索安装VLC media player播放器
  2 打开VLC，打开Tools -- Preferences菜单，在左下角Show settings处选择：全部all选项
  3 在左侧列表框里，选择：输入/编解码器(Input/Codecs) - 去复用器(Demuxers)，在去复用模块(Demux module)里，选择：H264 视频去复用器(H264 video demuxer)
  4 点击右下角的保存，即可开始播放本地h264文件。


推流UDP封装的MPEG-TS
   ffmpeg -re -i sintel.ts -f mpegts udp://127.0.0.1:8888
推流首先经过RTP封装，然后经过UDP封装的MPEG-TS
   ffmpeg -re -i sintel.ts -strict -2 -f rtp_mpegts udp://127.0.0.1:8888

