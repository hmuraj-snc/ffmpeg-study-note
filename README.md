ffmpeg-study-note
===============
***
How to write a video player based on [FFmpeg](http://www.ffmpeg.org/doxygen/trunk/index.html) and [mpenkov ffmpeg-tutorial](https://github.com/mpenkov/ffmpeg-tutorial)
***

*Requirements*

    ffmpeg version: 3.1.1<br />
    libavutil      55. 28.100<br />
    libavcodec     57. 48.101<br />
    libavformat    57. 41.100<br />
    libavdevice    57.  0.101<br />
    libavfilter     6. 47.100<br />
    libavresample   3.  0.  0<br />
    libswscale      4.  1.100<br />
    libswresample   2.  1.100<br />
    libpostproc    54.  0.100<br />

[Here](http://dranger.com/ffmpeg/) you can find a more detailed explanation and you can also get an updated version from [mpenkov ffmpeg-tutorial](https://github.com/mpenkov/ffmpeg-tutorial) what my code is based on.

I changed some deprecated functions and symbols to their suggested style.You can see those changes in every file.

*Note:* 

    All changes are based on mpenkov ffmpeg-tutorial.

***
All code compiled and run on win10 with vs 2013 community.
