#ifndef __sounds_h__
#define __sounds_h__

struct TDrumData {
        integer Len;
        array<1 , 255,word> Data;
};

#ifdef __Sounds_implementation__
#undef EXTERN
#define EXTERN
#endif

EXTERN boolean SoundEnabled;
EXTERN boolean SoundBlockQueueing;
EXTERN integer SoundCurrentPriority;
EXTERN array<1 , 255,word> SoundFreqTable;
EXTERN byte SoundDurationMultiplier;
EXTERN byte SoundDurationCounter;
EXTERN string SoundBuffer;
EXTERN pointer SoundNewVector;
EXTERN pointer SoundOldVector;
EXTERN integer SoundBufferPos;
EXTERN boolean SoundIsPlaying;
EXTERN integer SoundTimeCheckCounter;
EXTERN boolean UseSystemTimeForElapsed;
EXTERN word TimerTicks;
EXTERN integer SoundTimeCheckHsec;
EXTERN array<0 , 9,TDrumData> SoundDrumTable;
#undef EXTERN
#define EXTERN extern

        void SoundQueue(integer priority, string pattern);
        void SoundClearQueue();
        boolean SoundHasTimeElapsed(integer& counter, integer duration);
        void SoundUninstall();
        string SoundParse(string input);

#endif
