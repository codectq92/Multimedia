

rtph264 : simplest_rtp_send_h264.o CRtpH264.o
	g++ simplest_rtp_send_h264.o CRtpH264.o -ortph264


simplest_rtp_send_h264.o : simplest_rtp_send_h264.cpp
	g++ -c -fpic simplest_rtp_send_h264.cpp -o simplest_rtp_send_h264.o

CRtpH264.o : CRtpH264.cpp
	g++ -c -fpic CRtpH264.cpp -o CRtpH264.o

.Python : clean
clean :
	@rm -f *.o rtph264
