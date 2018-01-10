CUR_DIR = $(shell pwd)

LIBDIR = $(CUR_DIR)/libRTMP/rtmpdump/librtmp

rtmpflv : CRtmpDumpFlv.o simplest_librtmp_send_recv_flv.o
	g++ CRtmpDumpFlv.o simplest_librtmp_send_recv_flv.o -lrtmp -L$(LIBDIR) -ortmpflv

simplest_librtmp_send_recv_flv.o : simplest_librtmp_send_recv_flv.cpp
	g++ -c -fpic simplest_librtmp_send_recv_flv.cpp -o simplest_librtmp_send_recv_flv.o

CRtmpDumpFlv.o : CRtmpDumpFlv.cpp
	g++ -c -fpic CRtmpDumpFlv.cpp -o CRtmpDumpFlv.o


.Python : clean
clean :
	@rm -f *.o rtmp_flv.log rtmp_recv.flv

