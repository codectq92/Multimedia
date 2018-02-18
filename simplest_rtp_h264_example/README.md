本工程实现了在linux平台下通过RTP协议实时发送H264碼流，通过播放器mpv Media Player可以实时观看推送过来的H264碼流

步骤
   1  make
   2  在ubuntu 16.04 软件中心中搜索安装mpv Media Player播放器
   3  将当前目录sdp目录中mplayer.sdp文件拖到mpv Media Player中
   4  执行程序
      ./rtph26i4
   播放器会播放推送来的视频

