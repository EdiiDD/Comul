#include "h264rtpdepacketizer.h"
#include <unistd.h>

bool H264RTPDepacketizer::init(uint16_t fps, RTPReceiver *receiver) {
#ifdef GEN_FILE
  fout = fopen("recv.264","wb");
#endif
  period = 90000/fps;
  this->receiver = receiver;
  firstTS = 0;
  seqNum = 0;
  parar = false;
  frame = NULL;
  frame_size = 0;
  first_packet = true;
  return true;
}

bool H264RTPDepacketizer::receive(uint8_t **buffer, uint32_t *size, int64_t *pts) {
  const uint8_t *payload;
  uint32_t payload_size;

  while(!parar) {
    if (!receiver->getPayloadData()) {
      // No hay paquete RTP disponible.
      // Compruebo si el otro extremo sigue emitiendo...
      if (receiver->isClosed())
        // Ha cerrado el otro extremo.
        return false;
      // No se tiene constancia de que el otro extremo haya cerrado.
      // Intento obtener un paquete RTP nuevo...
      if (!receiver->receive()) {
	// No lo he conseguido. Espero el minimo posible antes de reintentar
	usleep(1);
      }
      else {
        // Recibido un paquete. Imprimo su TS
        printf("TS: %u SN: %u Size: %u\n",receiver->getTimestamp(),receiver->getSeqNum(), receiver->getPayloadSize());
      }
    }
    else {
      // Hay un paquete RTP disponible
      if (first_packet) {
        // Primer paquete
        firstTS = timestamp = receiver->getTimestamp();
        first_packet = false;
      }

      payload = receiver->getPayloadData();
      payload_size = receiver->getPayloadSize();

      // Compruebo si es del mismo frame
      if (timestamp == receiver->getTimestamp()) {
        // Mismo frame
        frame = (uint8_t *)realloc(frame,frame_size+1+payload_size);
        frame[frame_size++] = 0x00;
        frame[frame_size++] = 0x00;
        frame[frame_size++] = 0x00;
        frame[frame_size++] = 0x01;
        memcpy(frame + frame_size, payload + 3,payload_size - 3);
        frame_size += payload_size - 3;
        // Sólo liberar porque es el mismo frame
        receiver->freePacket();
      }
      else {
        // Distinto frame: devolver
        *buffer = frame;
        *size = frame_size;
        *pts = (timestamp-firstTS)/period;
#ifdef GEN_FILE
      fwrite(frame,frame_size,1,fout);
      fflush(fout);
#endif
        // Actualizo datos para siguiente frame
        timestamp = receiver->getTimestamp();
        frame_size = 0;
        frame = NULL;
        return true;
      }
    }
  }
  return false;
}

void H264RTPDepacketizer::close() {
  // Do nothing
#ifdef GEN_FILE
    fclose(fout);
#endif
}

void H264RTPDepacketizer::stop() {
  parar = true;
}

uint32_t H264RTPDepacketizer::getClock() {
  return RTPCLOCK;
}

