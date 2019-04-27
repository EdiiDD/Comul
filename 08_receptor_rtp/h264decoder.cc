#include "h264decoder.h"
#include <stdio.h>
#include <iostream>


H264Decoder::H264Decoder() {
    avcodec_register_all();
    // Ya no es necesario a partir de FFMPEG 4
    codec = avcodec_find_decoder(AV_CODEC_ID_H264); // AV_CODEC_ID_XXX -> Ver avcodec.h
    // El que quiera hacer decodificación por hardware: http://ffmpeg.org/doxygen/3.4/group__lavc__codec__hwaccel__vdpau.html
    if (!codec) {
        std::cerr << "Imposible encontrar el decodificador" << std::endl;
        exit(1);
    }
}

H264Decoder::~H264Decoder()
{
    close();
}


bool H264Decoder::init()
{
    context = avcodec_alloc_context3(codec);
    context->pix_fmt = AV_PIX_FMT_YUV420P; // AV_PIX_FMT_XXXX -> Ver pxfmt.h
    avcodec_open2(context, codec, NULL);
    pts = 0;
    return true;
}

bool H264Decoder::close()
{
    avcodec_close(context);
    av_free(context);

    return true;
}

bool H264Decoder::decode_packet(uint8_t* pkt, uint32_t size, uint64_t pts)
{
    int ret = 0;
    char errormsg[255];
    AVPacket avpkt;
    av_init_packet(&avpkt);
    avpkt.data = pkt;
    avpkt.size = size;
    avpkt.pts = pts;
    if ((ret = avcodec_send_packet(context, &avpkt)) < 0) {
        switch(ret) {
            case EAGAIN:
                // El frame no está preparado, vamos a seguir alimentando al decodificador
                std::cout << "Frame not ready, try again" << std::endl;
                av_packet_unref(&avpkt);
                return true;
                break;
            case EINVAL:
                std::cerr << "El codec no está preparado al recuperar el frame" << std::endl;
                av_packet_unref(&avpkt);
                exit(3);
                return true;
            case EOF:
                std::cerr << "EOF" << std::endl;
                av_packet_unref(&avpkt);
                exit(0);
                return false;
            default:
                std::cerr << "Imposible to decode net packet from pts " << pts << " with size " << size << std::endl;
                av_packet_unref(&avpkt);
                av_strerror(ret, errormsg, 255);
                fprintf(stderr, "Error: %s\n", errormsg);
                exit(2);
                return false;
        }
    }
    // DEPRECATED av_free_packet(&avpkt);
    av_packet_unref(&avpkt);
    return true;
}

FrameYUV * H264Decoder::read_frame()
{
    char errorbuff[256];
    uint8_t *Yin, *Yout, *Uout, *Vout, *Uin, *Vin;
    AVFrame *frame;
    frame = av_frame_alloc();
    frame->pts = pts;
    int ret = avcodec_receive_frame(context, frame);
    if (ret < 0) {
        switch(ret) {
            case EAGAIN:
                // El frame no está preparado, vamos a seguir alimentando al decodificador
                std::cout << "Frame not ready, try again" << std::endl;
                return NULL;
                break;
            case EINVAL:
                std::cerr << "El codec no está preparado al recuperar el frame" << std::endl;
                exit(3);
                return NULL;
            case EOF:
                std::cerr << "EOF" << std::endl;
                exit(0);
                return NULL;
            default:
                av_make_error_string(errorbuff, sizeof(errorbuff), ret);
                std::cerr << "FATAL: Código de error desconocido: " << AVERROR(ret) << ":" << errorbuff << std::endl;
                return NULL;
        }
    }

    // Frame contiene un AVFrame válido.
    FrameYUV *fyuv = new FrameYUV(frame->width, frame->height);
#ifdef DEBUG
    std::cout << "Frame: " << frame->width << "x" << frame->height << std::endl;
#endif
    Yin = frame->data[0];
    Yout = fyuv->Y();
    for (int i = 0; i < frame->height; i++ )
    {
        //Para cada fila
        // copiamos la fila del avframe al YUV
        // CUIDADO: El AVFrame tiene un tamaño de fila distinto a la anchura, el linesize indica la anchura verdadera en memoria
        memcpy(Yout, Yin, frame->width);
        Yin += frame->linesize[0];
        Yout += frame->width;
        //memcpy(fyuv->Y() + (i*frame->linesize[0]), frame->data[0 + (i*frame->width)], frame->width);
    }
    Uout = fyuv->U();
    Vout = fyuv->V();
    Uin = frame->data[1];
    Vin = frame->data[2];
    for (int i = 0; i < frame->height / 2; i++ )
    {
        memcpy(Uout, Uin, frame->width/2);
        memcpy(Vout, Vin, frame->width/2);
        Uin += frame->linesize[1];
        Uout += frame->width/2;
        Vin += frame->linesize[2];
        Vout += frame->width/2;
    }

    av_frame_free(&frame);
    pts ++;
    return fyuv;
}






