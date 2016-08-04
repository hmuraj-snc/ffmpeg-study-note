//RAW
/*========================================================================*/
// tutorial01.c
//
// This tutorial was written by Stephen Dranger (dranger@gmail.com).
//
// Code based on a tutorial by Martin Bohme (boehme@inb.uni-luebeckREMOVETHIS.de)
// Tested on Gentoo, CVS version 5/01/07 compiled with GCC 4.1.1

// A small sample program that shows how to use libavformat and libavcodec to
// read video from a file.
//
// Use the Makefile to build all examples.
//
// Run using
//
// tutorial01 myvideofile.mpg
//
// to write the first five frames from "myvideofile.mpg" to disk in PPM
// format.
/*========================================================================*/
//NEW
/*
 * Update 2016.8.4 : 
 1. replace some functions and variables which are deprecated to new-style
 * ffmpeg version: 3.1.1
     libavutil      55. 28.100
     libavcodec     57. 48.101
     libavformat    57. 41.100
     libavdevice    57.  0.101
     libavfilter     6. 47.100
     libavresample   3.  0.  0
     libswscale      4.  1.100
     libswresample   2.  1.100
     libpostproc    54.  0.100

 *                  RAW                                       |                         New
 *  avpicture_get_size()                                     ==>    av_image_get_buffer_size()
 *  avpicture_fill()                                         ==>    av_image_fill_arrays()
 *  avcodec_decode_video2()                                  ==>    avcodec_send_packet() and  avcodec_receive_frame()
 *  av_free_packet()                                         ==>    av_packet_unref()

 *  PIX_FMT_RGB24                                            ==>    AV_PIX_FMT_RGB24
 *  pCodecCtx=pFormatCtx->streams[videoStream]->codec        ==>    pCodecParam = pFormatCtx->streams[videoStream]->codecpar;
 
 2. this code compiled and run on win10 with vs2013 community
 

*/
//ffmpeg need compile as c
extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
#include <libavutil/imgutils.h>
}
#include <stdio.h>
#define SEND_PACKET_SUCCESS 0
#define RECEIVE_FRAME_SUCCESS 0


void SaveFrame(AVFrame *pFrame, int width, int height, int iFrame) {
    FILE *pFile;
    char szFilename[32];
    int  y;

    // Open file
    /*
     * if you compile this code on win 

     * sprintf                           ==>  sprintf_s
     * pFile = fopen(szFilename, "wb");  ==>  fopen_s(&pFile, szFilename, "wb");
    */
    sprintf(szFilename, "frame%d.ppm", iFrame);
    pFile = fopen(szFilename, "wb");
    if (pFile == NULL){
        return;
    }

    // Write header
    fprintf(pFile, "P6\n%d %d\n255\n", width, height);
    
    // Write pixel data
    for (y = 0; y<height; y++)
        fwrite(pFrame->data[0] + y*pFrame->linesize[0], 1, width * 3, pFile);
    
    // Close file
    fclose(pFile);
}

int main(int argc, char *argv[]) {
    AVFormatContext         *pFormatCtx = NULL;
    unsigned int            videoStream;
    int                     i;
    AVCodecContext          *pCodecCtx = NULL;
    AVCodecParameters       *pCodecParam = NULL;
    AVCodec                 *pCodec = NULL;
    AVFrame                 *pFrame = NULL;
    AVFrame                 *pFrameRGB = NULL;
    AVPacket                packet;
    int                     send_packet, receive_frame;
    int                     numBytes;
    uint8_t                 *buffer = NULL;

    AVDictionary            *optionsDict = NULL;
    struct SwsContext       *sws_ctx = NULL;

    if (argc < 2) {
        printf("Please provide a movie file\n");
        return -1;
    }
    // Register all formats and codecs
    av_register_all();

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
    pCodecCtx = avcodec_alloc_context3(pCodec);
    avcodec_parameters_to_context(pCodecCtx, pCodecParam);

    // Find the decoder for the video stream
    if (pCodec == NULL) {
        fprintf(stderr, "Unsupported codec!\n");
        return -1; // Codec not found
    }
    // Open codec
    if (avcodec_open2(pCodecCtx, pCodec, &optionsDict)<0)
        return -1; // Could not open codec

    // Allocate video frame
    pFrame = av_frame_alloc();

    // Allocate an AVFrame structure
    pFrameRGB = av_frame_alloc();
    if (pFrameRGB == NULL)
        return -1;

    // Determine required buffer size and allocate buffer
    // fuction avpicture_get_size is deprecated ,use av_image_get_buffer_size() instead.
    numBytes = av_image_get_buffer_size(AV_PIX_FMT_RGB24, pCodecParam->width,
        pCodecParam->height, 1);
    //raw source malloc size is numBytes*sizeof(uint8_t) , I have no idea why it is numBytes*sizeof(uint8_t) rather than numBytes.
    buffer = (uint8_t *)av_malloc(numBytes);
    //fill sws context
    sws_ctx =
        sws_getContext
        (
        //use (AVCodecParameters) pCodecParam instead of (AVCodecContext) pCodecCtx
        pCodecParam->width,
        pCodecParam->height,
        (AVPixelFormat)pCodecParam->format,
        pCodecParam->width,
        pCodecParam->height,
        AV_PIX_FMT_RGB24,
        SWS_BILINEAR,
        NULL,
        NULL,
        NULL
        );

    // Assign appropriate parts of buffer to image planes in pFrameRGB
    // Note that pFrameRGB is an AVFrame, but AVFrame is a superset
    // of AVPicture
    //avpicture_fill is deprecated, use av_image_fill_arrays() instead.
    av_image_fill_arrays(pFrameRGB->data, pFrameRGB->linesize, buffer, AV_PIX_FMT_RGB24,
        pCodecParam->width, pCodecParam->height, 1);

    // Read frames and save first five frames to disk
    i = 0;
    //there add (i<=5) to the while loop can reduece unnecessary read and deal
    while (av_read_frame(pFormatCtx, &packet) >= 0 && (i <= 5) ) {
        // Is this a packet from the video stream?
        if (packet.stream_index == videoStream) {
            // Decode video frame
            //avcodec_decode_video2 is deprecated Use avcodec_send_packet() and avcodec_receive_frame().
            send_packet = avcodec_send_packet(pCodecCtx, &packet);
            receive_frame = avcodec_receive_frame(pCodecCtx, pFrame);

            // Did we get a video frame?check it
            if (send_packet == SEND_PACKET_SUCCESS && receive_frame == RECEIVE_FRAME_SUCCESS) {
                
                // Convert the image from its native format to RGB
                    sws_scale
                        (
                        sws_ctx,
                        pFrame->data,
                        pFrame->linesize,
                        0,
                        pCodecParam->height,
                        pFrameRGB->data,
                        pFrameRGB->linesize
                        );

                    // Save the frame to disk
                    if (++i <= 5)
                        SaveFrame(pFrame, pCodecParam->width, pCodecParam->height, i);
    
            }
        }

        // Free the packet that was allocated by av_read_frame
        //av_free_packet is deprecated Use av_packet_unref
        av_packet_unref(&packet);
    }

    // Free the RGB image
    //change but unnecessary,you can  use av_free still.
    av_freep(&buffer);
    av_frame_free(&pFrameRGB);

    // Free the YUV frame
    av_frame_free(&pFrame);

    // Close the codec
    avcodec_close(pCodecCtx);

    // Close the video file
    avformat_close_input(&pFormatCtx);

    return 0;
}
