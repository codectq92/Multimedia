CUR_DIR = $(shell pwd)

LIBDIR = $(CUR_DIR)/libRTMP/rtmpdump/librtmp

all : rtmppushflv rtmppullflv

#RTMP推流执行程序
rtmppushflv : simplest_librtmp_send_flv.o CRtmpPublicFlv.o CNetByteOper.o
	g++ CRtmpPublicFlv.o CNetByteOper.o simplest_librtmp_send_flv.o -lrtmp -L$(LIBDIR) -ortmppushflv

#RTMP拉流执行程序
rtmppullflv : simplest_librtmp_recv_flv.o CRtmpRecvFlv.o
	g++ CRtmpRecvFlv.o simplest_librtmp_recv_flv.o -lrtmp -L$(LIBDIR) -ortmppullflv

simplest_librtmp_send_flv.o : simplest_librtmp_send_flv.cpp
	g++ -c -fpic simplest_librtmp_send_flv.cpp -o simplest_librtmp_send_flv.o

CRtmpRecvFlv.o : CRtmpRecvFlv.cpp
	g++ -c -fpic CRtmpRecvFlv.cpp -o CRtmpRecvFlv.o

CRtmpPublicFlv.o : CRtmpPublicFlv.cpp
	g++ -c -fpic CRtmpPublicFlv.cpp -o CRtmpPublicFlv.o

CNetByteOper.o : CNetByteOper.cpp
	g++ -c -fpic CNetByteOper.cpp -o CNetByteOper.o

.Python : clean
clean :
	@rm -f *.o log/*

