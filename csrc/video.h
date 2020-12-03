#ifndef __video_h__
#define __video_h__

typedef varying_string<80> TVideoLine;
struct TTextChar {
        char Char;
        byte Color;
};

typedef matrix<1,80, 1,25,TTextChar> TVideoBuffer;

#ifdef __Video_implementation__
#undef EXTERN
#define EXTERN
#endif

EXTERN boolean VideoMonochrome;
EXTERN TVideoBuffer MainBuffer;
#undef EXTERN
#define EXTERN extern


        boolean VideoConfigure();
        void VideoWriteText(byte x, byte y, byte color, TVideoLine text);
        void VideoToggleEGAMode(boolean EGA);
        void VideoInstall(integer columns, integer borderColor);
        void VideoUninstall();
        void VideoShowCursor();
        void VideoHideCursor();
        void VideoSetBorderColor(integer value);
        void VideoCopy(integer xfrom, integer yfrom, integer width, integer height,
                    TVideoBuffer& buf, boolean toVideo);

#endif
