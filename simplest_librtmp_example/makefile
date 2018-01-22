CUR_DIR = $(shell pwd)

LIBDIR = $(CUR_DIR)/libRTMP/rtmpdump/librtmp

all : rtmppushflv rtmppullflv rtmppushh264

#RTMP推流FLV执行程序
rtmppushflv : simplest_librtmp_send_flv.o CRtmpPublicFlv.o CNetByteOper.o
	g++ CRtmpPublicFlv.o CNetByteOper.o simplest_librtmp_send_flv.o -lrtmp -L$(LIBDIR) -ortmppushflv

#RTMP拉流FLV执行程序
rtmppullflv : simplest_librtmp_recv_flv.o CRtmpRecvFlv.o
	g++ CRtmpRecvFlv.o simplest_librtmp_recv_flv.o -lrtmp -L$(LIBDIR) -ortmppullflv

#RTMP推流H264执行程序
rtmppushh264 : simplest_librtmp_send_h264.o CRtmpSendH264.o
	g++ CRtmpSendH264.o CNetByteOper.o simplest_librtmp_send_h264.o -lrtmp -L$(LIBDIR) -ortmppushh264

simplest_librtmp_send_flv.o : simplest_librtmp_send_flv.cpp
	g++ -c -fpic simplest_librtmp_send_flv.cpp -o simplest_librtmp_send_flv.o

CRtmpPublicFlv.o : CRtmpPublicFlv.cpp
	g++ -c -fpic CRtmpPublicFlv.cpp -o CRtmpPublicFlv.o

simplest_librtmp_recv_flv.o : simplest_librtmp_recv_flv.cpp
	g++ -c -fpic simplest_librtmp_recv_flv.cpp -o simplest_librtmp_recv_flv.o

CRtmpRecvFlv.o : CRtmpRecvFlv.cpp
	g++ -c -fpic CRtmpRecvFlv.cpp -o CRtmpRecvFlv.o

simplest_librtmp_send_h264.o : simplest_librtmp_send_h264.cpp
	g++ -c -fpic simplest_librtmp_send_h264.cpp -o simplest_librtmp_send_h264.o

CRtmpSendH264.o : CRtmpSendH264.cpp
	g++ -c -fpic CRtmpSendH264.cpp -o CRtmpSendH264.o

CNetByteOper.o : CNetByteOper.cpp
	g++ -c -fpic CNetByteOper.cpp -o CNetByteOper.o

.Python : clean
clean :
	@rm -f *.o log/*

