// tutorial02.c
// A pedagogical video player that will stream through every video frame as fast as it can.
//
// This tutorial was written by Stephen Dranger (dranger@gmail.com).
//
// Code based on FFplay, Copyright (c) 2003 Fabrice Bellard, 
// and a tutorial by Martin Bohme (boehme@inb.uni-luebeckREMOVETHIS.de)
// Tested on Gentoo, CVS version 5/01/07 compiled with GCC 4.1.1
//
// Use the Makefile to build all examples.
//
// Run using
// tutorial02 myvideofile.mpg
//
// to play the video stream on your screen.

extern "C"{
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
#include <libavutil\imgutils.h>
}
#include <SDL.h>

#ifdef __MINGW32__
#undef main /* Prevents SDL from overriding main() */
#endif

#include <stdio.h>
#define SEND_PACKET_SUCCESS 0
#define RECEIVE_FRAME_SUCCESS 0

int main(int argc, char *argv[]) {
    AVFormatContext *pFormatCtx = NULL;
    int             i, videoStream;
    AVCodecContext  *pCodecCtx = NULL;
    AVCodecParameters       *pCodecParam = NULL;
    AVCodec         *pCodec = NULL;
    AVFrame         *pFrame = NULL;
    AVPacket        packet;
    int             send_packet, receive_frame;
    //float           aspect_ratio;
    AVFrame        *pict;
    /*
    std::unique_ptr<AVFrame, std::function<void(AVFrame*)>> frame_converted{
        av_frame_alloc(),
        [](AVFrame* f){ av_free(f->data[0]); } };
    if (av_frame_copy_props(frame_converted.get(),
        frame_decoded.get()) < 0) {
        throw std::runtime_error("Copying frame properties");
    }
    if (av_image_alloc(
        frame_converted->data, frame_converted->linesize,
        video_decoder_->width(), video_decoder_->height(),
        video_decoder_->pixel_format(), 1) < 0) {
        throw std::runtime_error("Allocating picture");
    }
    */
    AVDictionary    *optionsDict = NULL;
    struct SwsContext *sws_ctx = NULL;

    SDL_Texture*    pTexture = nullptr;
    SDL_Window*     pWindows = nullptr;
    SDL_Renderer*   pRenderer = nullptr;

    SDL_Event       event;

    if (argc < 2) {
        fprintf(stderr, "Usage: test <file>\n");
        exit(1);
    }
    // Register all formats and codecs
    av_register_all();

    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_TIMER)) {
        fprintf(stderr, "Could not initialize SDL - %s\n", SDL_GetError());
        exit(1);
    }

    // Open video file
    if (avformat_open_input(&pFormatCtx, argv[1], NULL, NULL) != 0)
        return -1; // Couldn't open file

    // Retrieve stream information
    if (avformat_find_stream_info(pFormatCtx, NULL)<0)
        return -1; // Couldn't find stream information

    // Dump information about file onto standard error
    av_dump_format(pFormatCtx, 0, argv[1], 0);

    // Find the first video stream
    videoStream = -1;
    for (i = 0; i<pFormatCtx->nb_streams; i++)
        if (pFormatCtx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
            videoStream = i;
            break;
        }
    if (videoStream == -1)
        return -1; // Didn't find a video stream

    // Get a pointer to the codec context for the video stream
    //AVCodecContext *codec is deprecated,so use the codecpar struct (AVCodecParameters) instead.
    pCodecParam = pFormatCtx->streams[videoStream]->codecpar;
    //but function avcodec_open2() need pCodecCtx,so copy  (AVCodecParameters) pCodecParam to (AVCodecContext) pCodecCtx
    pCodec = avcodec_find_decoder(pCodecParam->codec_id);
    // Find the decoder for the video stream
    if (pCodec == NULL) {
        fprintf(stderr, "Unsupported codec!\n");
        return -1; // Codec not found
    }
    pCodecCtx = avcodec_alloc_context3(pCodec);
    avcodec_parameters_to_context(pCodecCtx, pCodecParam);

    // Open codec
    if (avcodec_open2(pCodecCtx, pCodec, &optionsDict)<0)
        return -1; // Could not open codec

    // Allocate video frame
    pFrame = av_frame_alloc();

    // Make a screen to put our video
#ifndef __DARWIN__
    pWindows = SDL_CreateWindow(argv[1],SDL_WINDOWPOS_CENTERED,SDL_WINDOWPOS_CENTERED,pCodecParam->width, pCodecParam->height,SDL_WINDOW_BORDERLESS|SDL_WINDOW_RESIZABLE);
#else
    screen = SDL_SetVideoMode(pCodecParam->width, pCodecParam->height, 24, 0);
#endif
    if (!pWindows) {
        fprintf(stderr, "SDL: could not set video mode - exiting\n");
        exit(1);
    }
    
    // Allocate a place to put our YUV image on that screen
    pRenderer = SDL_CreateRenderer(pWindows, -1, 0);
    if (!pRenderer) {
        fprintf(stderr, "SDL: could not create renderer - exiting\n");
        exit(1);
    }
    pTexture = SDL_CreateTexture(pRenderer, SDL_PIXELFORMAT_YV12, SDL_TEXTUREACCESS_STREAMING, pCodecParam->width, pCodecParam->height);
    sws_ctx =
        sws_getContext
        (
        pCodecParam->width,
        pCodecParam->height,
        (AVPixelFormat)pCodecParam->format,
        pCodecParam->width,
        pCodecParam->height,
        AV_PIX_FMT_YUV420P,
        SWS_BILINEAR,
        NULL,
        NULL,
        NULL
        );
    pict = av_frame_alloc();
    if (pict == nullptr){
        exit(1);
    }
    if (av_image_alloc(pict->data, pict->linesize,
        pCodecParam->width, pCodecParam->height,
        (AVPixelFormat)pCodecParam->format, 1) < 0){
        exit(1);
    }


    // Read frames and save first five frames to disk
    i = 0;
    while (av_read_frame(pFormatCtx, &packet) >= 0) {
        // Is this a packet from the video stream?
        if (packet.stream_index == videoStream) {
            // Decode video frame
            //avcodec_decode_video2 is deprecated Use avcodec_send_packet() and avcodec_receive_frame().
            send_packet = avcodec_send_packet(pCodecCtx, &packet);
            receive_frame = avcodec_receive_frame(pCodecCtx, pFrame);

            // Did we get a video frame?
            if (send_packet == SEND_PACKET_SUCCESS && receive_frame == RECEIVE_FRAME_SUCCESS) {
                //SDL_LockYUVOverlay(bmp);
                //SDL_LockTexture(pTexture,NULL,);
                // Convert the image into YUV format that SDL uses
                if (av_frame_copy_props(pFrame,
                    pict) < 0) {
                    exit(1);
                }

                sws_scale
                    (
                    sws_ctx,
                    pFrame->data,
                    pFrame->linesize,
                    0,
                    pCodecParam->height,
                    pict->data,
                    pict->linesize
                    );
                
                //SDL_UnlockYUVOverlay(bmp);
                SDL_UpdateYUVTexture(pTexture, NULL, pict->data[0], pict->linesize[0], pict->data[1], pict->linesize[1], pict->data[2], pict->linesize[2]);
                SDL_RenderCopy(pRenderer, pTexture, NULL, NULL);
                SDL_RenderPresent(pRenderer);

            }
        }

        // Free the packet that was allocated by av_read_frame
        av_packet_unref(&packet);
        SDL_PollEvent(&event);
        switch (event.type) {
        case SDL_QUIT:
            SDL_DestroyRenderer(pRenderer);
            SDL_DestroyTexture(pTexture);
            SDL_DestroyWindow(pWindows);
            SDL_Quit();
            exit(0);
            break;
        default:
            break;
        }

    }

    // Free the YUV frame
    av_frame_free(&pFrame);
    //free pict
    av_freep(&pict->data[0]);
    av_frame_free(&pict);

    // Close the codec
    avcodec_close(pCodecCtx);

    // Close the video file
    avformat_close_input(&pFormatCtx);

    return 0;
}
