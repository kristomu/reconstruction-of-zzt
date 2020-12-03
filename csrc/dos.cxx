#include "ptoc.h"


/*******************************************************/
/*                                                       */
/*       Turbo Pascal Runtime Library                    */
/*       DOS Interface Unit                              */
/*                                                       */
/*       Copyright (C) 1988,92 Borland International     */
/*                                                       */
/*******************************************************/

#define __Dos_implementation__


/*$I-,O+,S-*/

#include "dos.h"


/*$L VERS.OBJ*/           /* DOS version routine */
/*$L TIME.OBJ*/           /* Date and time routines */
/*$L CBRK.OBJ*/           /* Ctrl-Break flag handling */
/*$L VERF.OBJ*/           /* Verify flag handling */
/*$L DISK.OBJ*/           /* Disk status routines */
/*$L FATR.OBJ*/           /* File attribute routines */
/*$L FTIM.OBJ*/           /* File date and time routines */
/*$L FIND.OBJ*/           /* Directory search routines */
/*$L PTIM.OBJ*/           /* Time pack and unpack routines */
/*$L VECT.OBJ*/           /* Interrupt vector handling */
/*$L SRCH.OBJ*/           /* File search routine */
/*$L EXPN.OBJ*/           /* File name expansion routine */
/*$L SPLT.OBJ*/           /* File name split routine */
/*$L ENVS.OBJ*/           /* Environment string routines */
/*$L ENVV.OBJ*/           /* Environment variable routine */
/*$L KEEP.OBJ*/           /* TSR support routine */
/*$L EXEC.OBJ*/           /* Program execution routines */

#ifdef DPMI

/*$L INTR.OBP*/           /* Software interrupt routines */
/*$L SWAP.OBP*/           /* Interrupt vector swapping */

#else

/*$L INTR.OBJ*/           /* Software interrupt routines */
/*$L SWAP.OBJ*/           /* Interrupt vector swapping */

#endif

extern word DosVersion();

extern void Intr(byte IntNo, Registers& Regs);

extern void MsDos(Registers& Regs);

extern void GetDate(word& Year,word& Month,word& Day,word& DayOfWeek);

extern void SetDate(word Year,word Month,word Day);

extern void GetTime(word& Hour,word& Minute,word& Second,word& Sec100);

extern void SetTime(word Hour,word Minute,word Second,word Sec100);

extern void GetCBreak(boolean& break_);

extern void SetCBreak(boolean break_);

extern void GetVerify(boolean& Verify);

extern void SetVerify(boolean Verify);

extern longint DiskFree(byte Drive);

extern longint DiskSize(byte Drive);

extern void GetFAttr(void* F, word& Attr);

extern void SetFAttr(void* F, word Attr);

extern void GetFTime(void* F, longint& Time);

extern void SetFTime(void* F, longint Time);

extern void FindFirst(PathStr Path, word Attr, SearchRec& F);


extern void FindNext(SearchRec& F);

extern void UnpackTime(longint P, DateTime& T);

extern void PackTime(DateTime& T, longint& P);

extern void GetIntVec(byte IntNo, pointer& Vector);

extern void SetIntVec(byte IntNo, pointer Vector);

extern PathStr FSearch(PathStr Path, string DirList);

extern PathStr FExpand(PathStr Path);

extern void FSplit(PathStr Path, DirStr& Dir,
                   NameStr& Name, ExtStr& Ext);

extern integer EnvCount();

extern string EnvStr(integer Index);

extern string GetEnv(string EnvVar);

extern void SwapVectors();

extern void Keep(word ExitCode);

extern void Exec(PathStr Path, ComStr ComLine);

extern word DosExitCode();


