本工程包含了LibRTMP的使用示例，包含如下子工程： \
   simplest_librtmp_receive: 接收RTMP流媒体并在本地保存成FLV格式的文件。\
   simplest_librtmp_send_flv: 将FLV格式的视音频文件使用RTMP推送至RTMP流媒体服务器。\
   simplest_librtmp_send264: 将内存中的H.264数据推送至RTMP流媒体服务器。

Ubuntu16.0.4下播放H264裸流文件 \
   1 在软件中心搜索安装VLC media player播放器 \
   2 打开VLC，打开Tools -- Preferences菜单，在左下角Show settings处选择：全部all选项 \
   3 在左侧列表框里，选择：输入/编解码器(Input/Codecs) - 去复用器(Demuxers)，在去复用模块(Demux module)里，\
      选择：H264 视频去复用器(H264 video demuxer) \
   4 点击右下角的保存，即可开始播放本地h264文件。

Ubuntu16.0.4下播放网络流视频 \
   1 在软件中心搜索安装VLC media player播放器 \
   2 打开VLC，打开Tools -- Preferences菜单，在左下角Show settings处选择：全部all选项 \
   3 在左侧列表框里，选择：输入/编解码器(Input/Codecs) - 去复用器(Demuxers)，在去复用模块(Demux module)里，选择：Automatic \
   4 点击右下角的保存，即可开始观看网络流视频


librtmp库编译步骤 \
   在Ubuntu 16.04 64bit下编译安装rtmpdump并调试输出 \
   1.cd libRTMP/rtmpdump \
     安装相关依赖类 \
     需要用到的依赖库是zlib, openssl库,使用如下命令安装 \
     sudo apt-get install openssl \
     sudo apt-get install libssl-dev \
     sudo apt-get install zlib1g-dev
  
   2.编译 \
     make

   3.设置librtmp库搜索路径 \
    export LD_LIBRARY_PATH=./libRTMP/librtmp:$LD_LIBRARY_PATH

   4 在当前目录下 \
     make clean \
     make \
     生成3个执行程序，rtmppushflv代表推送flv到rtmp文件，rtmppullflv代表接收rtmp服务器推送来的flv视频，rtmppushh264代表推送h264到rtmp服务器
