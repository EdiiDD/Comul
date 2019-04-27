CXX         = c++
LIBS	    = sdl2 libavcodec libavutil x11 vdpau libva libva-drm libva-x11 jrtplib jthread
CFLAGS      = -c $$(pkg-config $(LIBS) --cflags)
CFLAGS     += -g
CFLAGS     += -DGEN_FILE
LDFLAGS     = $$(pkg-config $(LIBS) --libs) -lturbojpeg
OBJS        = umurtpsession.o rtpreceiver.o receptor.o h264rtpdepacketizer.o h264decoder.o display.o

all:	receptor

clean:
	rm -f *.o receptor

receptor: $(OBJS)
	$(CXX) -o $@ $(OBJS) $(LDFLAGS)

.SUFFIXES: .o .cc .h

.cc.o:
	$(CXX) -o $@ $(CFLAGS) $<
