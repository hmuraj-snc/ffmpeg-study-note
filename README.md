ffmpeg-study-note
===============
***
How to write a video player based on [FFmpeg](http://www.ffmpeg.org/doxygen/trunk/index.html) and [mpenkov ffmpeg-tutorial](https://github.com/mpenkov/ffmpeg-tutorial)
***

*Requirements*

    ffmpeg version: 3.1.1
    libavutil      55. 28.100
    libavcodec     57. 48.101
    libavformat    57. 41.100
    libavdevice    57.  0.101
    libavfilter     6. 47.100
    libavresample   3.  0.  0
    libswscale      4.  1.100
    libswresample   2.  1.100
    libpostproc    54.  0.100

[Here](http://dranger.com/ffmpeg/) you can find a more detailed explanation and you can also get an updated version from [mpenkov ffmpeg-tutorial](https://github.com/mpenkov/ffmpeg-tutorial) what my code is based on.

I changed some deprecated functions and symbols to their suggested style.You can see those changes in every file.

*Note:* 

    All changes are based on mpenkov ffmpeg-tutorial.

***
All code compiled and run on win10 with vs 2013 community.
