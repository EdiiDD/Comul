#ifndef __H264RTPDepacketizer
#define __H264RTPDepacketizer

#include <stdint.h>
#include "rtpreceiver.h"
#define RTPCLOCK 90000

class H264RTPDepacketizer {
public:
    bool init(uint16_t fps, RTPReceiver *receiver);
    bool receive(uint8_t **buffer, uint32_t *size, int64_t *pts);
    void close();
    void stop();
    uint32_t getClock();
private:
    uint32_t period;
    RTPReceiver * receiver;
    uint32_t timestamp, firstTS;
    uint8_t *frame; // Buffer para recomponer frame comprimido
    uint32_t frame_size;
    uint32_t seqNum;
    bool first_packet;
    bool parar;
#ifdef GEN_FILE
    FILE *fout;
#endif
};

#endif
