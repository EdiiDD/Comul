#ifndef __H264DECODER
#define __H264DECODER
#include "frame.h"

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavutil/opt.h>
#include <libavutil/frame.h>
}

class H264Decoder {
public:
    H264Decoder();
    ~H264Decoder();
    bool init();
    bool close();
    bool decode_packet(uint8_t *pkt, uint32_t size, uint64_t pts);
    FrameYUV *read_frame();
protected:
    AVCodec *codec;
    AVCodecContext *context;
    uint16_t pts;
};
#endif
