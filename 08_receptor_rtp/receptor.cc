#include "rtpreceiver.h"
#include "h264rtpdepacketizer.h"
#include "h264decoder.h"
#include "display.h"
#include <signal.h>
#include <stdio.h>
#include <unistd.h>

bool stop;

H264RTPDepacketizer depack;

void sighand(int sig) {
  stop = true;
  depack.stop();
}

int main(int argc, char *argv[]) {
  if (argc != 3) {
    fprintf(stderr, "Uso: %s puerto fps\n", argv[0]);
    exit(1);
  }

  RTPReceiver receiver;
  bool ret = receiver.init(atoi(argv[1]),90000);
  if (!ret) { printf("Error inicializando RTP\n"); exit(1); }

  ret = depack.init(atoi(argv[2]),&receiver);
  if (!ret) { printf("Error inicializando desempaquetador\n"); exit(1); }

  H264Decoder decod;
  ret = decod.init();
  if (!ret) { printf("Error inicalizando decodificador\n"); exit(1); }

  stop = false;
  signal(SIGINT, sighand);
  signal(SIGQUIT, sighand);


#ifdef GEN_FILE
  FILE *fout = fopen("out.yuv","wb");
#endif

  // Llamar a H264RTPDepacketizer
  uint8_t *buffer;
  uint32_t size;
  int64_t pts;
  FrameYUV *f = NULL;

  if (!DisplayYUV::init()) exit(1);
  DisplayYUV * display = NULL;

  while(!stop) {
     if (depack.receive(&buffer, &size, &pts)) {
        printf("Frame recibido: %ld %d\n", pts, size);
        decod.decode_packet(buffer, size, pts);
        if ((f = decod.read_frame()) != NULL) {
           // Volcar a fichero
#ifdef GEN_FILE
            fwrite(f->data,1,f->size,fout);
#endif
           if (display == NULL) {
              display = new DisplayYUV((char *)"Receptor",f->width,f->height,&ret);
              if (!ret) exit(1);
           }
           if (!display->display(f)) {
              fprintf(stderr, "Error visualizando\n"); exit(1);
           }
           free(f);
        }
     }
     else stop = true;
  }

#ifdef GEN_FILE
  fclose(fout);
#endif
  receiver.close();
  DisplayYUV::quit();
  if (f != 0) delete f;
}
