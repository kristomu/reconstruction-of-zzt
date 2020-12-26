#ifndef __dos_h__
#define __dos_h__


/* Flags bit masks */

const integer FCarry = 0x1;
const integer FParity = 0x4;
const integer FAuxiliary = 0x10;
const integer FZero = 0x40;
const integer FSign = 0x80;
const integer FOverflow = 0x800;

/* File mode magic numbers */

const integer fmClosed = 0xd7b0;
const integer fmInput = 0xd7b1;
const integer fmOutput = 0xd7b2;
const integer fmInOut = 0xd7b3;

/* File attribute constants */

const integer ReadOnly = 0x1;
const integer Hidden = 0x2;
const integer SysFile = 0x4;
const integer VolumeID = 0x8;
const integer Directory = 0x10;
const integer Archive = 0x20;
const integer AnyFile = 0x3f;

/* String types */

typedef varying_string<127> ComStr;        /* Command line string */
typedef varying_string<79> PathStr;         /* File pathname string */
typedef varying_string<67> DirStr;         /* Drive and directory string */
typedef varying_string<8> NameStr;          /* File name string */
typedef varying_string<4> ExtStr;          /* File extension string */

/* Registers record used by Intr and MsDos */

struct Registers {
	union {
		struct {
			word AX,BX,CX,DX,BP,SI,DI,DS,ES,Flags;
		} s1;
		struct {
			byte AL,AH,BL,BH,CL,CH,DL,DH;
		} s2;
	};
};

/* Typed-file and untyped-file record */

struct FileRec {
	word Handle;
	word Mode;
	word RecSize;
	array<1,26,byte> private_;
	array<1,16,byte> UserData;
	array<0,79,char> Name;
};

/* Textfile record */

typedef array<0,127,char> TextBuf;
struct TextRec {
	word Handle;
	word Mode;
	word BufSize;
	word private_;
	word BufPos;
	word BufEnd;
	TextBuf* BufPtr;
	pointer OpenFunc;
	pointer InOutFunc;
	pointer FlushFunc;
	pointer CloseFunc;
	array<1,16,byte> UserData;
	array<0,79,char> Name;
	TextBuf Buffer;
};

/* Search record used by FindFirst and FindNext */

struct SearchRec {
	array<1,21,byte> Fill;
	byte Attr;
	longint Time;
	longint Size;
	varying_string<12> Name;
};

/* Date and time record used by PackTime and UnpackTime */

struct DateTime {
	word Year,Month,Day,Hour,Min,Sec;
};


#ifdef __Dos_implementation__
#undef EXTERN
#define EXTERN
#endif

/* Error status variable */

EXTERN integer DosError;
#undef EXTERN
#define EXTERN extern


/* DosVersion returns the DOS version number. The low byte of    */
/* the result is the major version number, and the high byte is  */
/* the minor version number. For example, DOS 3.20 returns 3 in  */
/* the low byte, and 20 in the high byte.                        */

word DosVersion();

/* Intr executes a specified software interrupt with a specified */
/* Registers package.                                            */

void Intr(byte IntNo, Registers & Regs);

/* MsDos invokes the DOS function call handler with a specified  */
/* Registers package.                                            */

void MsDos(Registers & Regs);

/* GetDate returns the current date set in the operating system. */
/* Ranges of the values returned are: Year 1980-2099, Month      */
/* 1-12, Day 1-31 and DayOfWeek 0-6 (0 corresponds to Sunday).   */

void GetDate(word & Year,word & Month,word & Day,word & DayOfWeek);

/* SetDate sets the current date in the operating system. Valid  */
/* parameter ranges are: Year 1980-2099, Month 1-12 and Day      */
/* 1-31. If the date is not valid, the function call is ignored. */

void SetDate(word Year,word Month,word Day);

/* GetTime returns the current time set in the operating system. */
/* Ranges of the values returned are: Hour 0-23, Minute 0-59,    */
/* Second 0-59 and Sec100 (hundredths of seconds) 0-99.          */

void GetTime(word & Hour,word & Minute,word & Second,word & Sec100);

/* SetTime sets the time in the operating system. Valid          */
/* parameter ranges are: Hour 0-23, Minute 0-59, Second 0-59 and */
/* Sec100 (hundredths of seconds) 0-99. If the time is not       */
/* valid, the function call is ignored.                          */

void SetTime(word Hour,word Minute,word Second,word Sec100);

/* GetCBreak returns the state of Ctrl-Break checking in DOS.    */
/* When off (False), DOS only checks for Ctrl-Break during I/O   */
/* to console, printer, or communication devices. When on        */
/* (True), checks are made at every system call.                 */

void GetCBreak(boolean & break_);

/* SetCBreak sets the state of Ctrl-Break checking in DOS.       */

void SetCBreak(boolean break_);

/* GetVerify returns the state of the verify flag in DOS. When   */
/* off (False), disk writes are not verified. When on (True),    */
/* all disk writes are verified to insure proper writing.        */

void GetVerify(boolean & Verify);

/* SetVerify sets the state of the verify flag in DOS.           */

void SetVerify(boolean Verify);

/* DiskFree returns the number of free bytes on the specified    */
/* drive number (0=Default,1=A,2=B,..). DiskFree returns -1 if   */
/* the drive number is invalid.                                  */

longint DiskFree(byte Drive);

/* DiskSize returns the size in bytes of the specified drive     */
/* number (0=Default,1=A,2=B,..). DiskSize returns -1 if the     */
/* drive number is invalid.                                      */

longint DiskSize(byte Drive);

/* GetFAttr returns the attributes of a file. F must be a file   */
/* variable (typed, untyped or textfile) which has been assigned */
/* a name. The attributes are examined by ANDing with the        */
/* attribute masks defined as constants above. Errors are        */
/* reported in DosError.                                         */

void GetFAttr(void* F, word & Attr);

/* SetFAttr sets the attributes of a file. F must be a file      */
/* variable (typed, untyped or textfile) which has been assigned */
/* a name. The attribute value is formed by adding (or ORing)    */
/* the appropriate attribute masks defined as constants above.   */
/* Errors are reported in DosError.                              */

void SetFAttr(void* F, word Attr);

/* GetFTime returns the date and time a file was last written.   */
/* F must be a file variable (typed, untyped or textfile) which  */
/* has been assigned and opened. The Time parameter may be       */
/* unpacked throgh a call to UnpackTime. Errors are reported in  */
/* DosError.                                                     */

void GetFTime(void* F, longint & Time);

/* SetFTime sets the date and time a file was last written.      */
/* F must be a file variable (typed, untyped or textfile) which  */
/* has been assigned and opened. The Time parameter may be       */
/* created through a call to PackTime. Errors are reported in    */
/* DosError.                                                     */

void SetFTime(void* F, longint Time);

/* FindFirst searches the specified (or current) directory for   */
/* the first entry that matches the specified filename and       */
/* attributes. The result is returned in the specified search    */
/* record. Errors (and no files found) are reported in DosError. */

void FindFirst(PathStr Path, word Attr, SearchRec & F);

/* FindNext returs the next entry that matches the name and      */
/* attributes specified in a previous call to FindFirst. The     */
/* search record must be one passed to FindFirst. Errors (and no */
/* more files) are reported in DosError.                         */

void FindNext(SearchRec & F);

/* UnpackTime converts a 4-byte packed date/time returned by     */
/* FindFirst, FindNext or GetFTime into a DateTime record.       */

void UnpackTime(longint P, DateTime & T);

/* PackTime converts a DateTime record into a 4-byte packed      */
/* date/time used by SetFTime.                                   */

void PackTime(DateTime & T, longint & P);

/* GetIntVec returns the address stored in the specified         */
/* interrupt vector.                                             */

void GetIntVec(byte IntNo, pointer & Vector);

/* SetIntVec sets the address in the interrupt vector table for  */
/* the specified interrupt.                                      */

void SetIntVec(byte IntNo, pointer Vector);

/* FSearch searches for the file given by Path in the list of    */
/* directories given by DirList. The directory paths in DirList  */
/* must be separated by semicolons. The search always starts     */
/* with the current directory of the current drive. The returned */
/* value is a concatenation of one of the directory paths and    */
/* the file name, or an empty string if the file could not be    */
/* located.                                                      */

PathStr FSearch(PathStr Path, string DirList);

/* FExpand expands the file name in Path into a fully qualified  */
/* file name. The resulting name consists of a drive letter, a   */
/* colon, a root relative directory path, and a file name.       */
/* Embedded '.' and '..' directory references are removed.       */

PathStr FExpand(PathStr Path);

/* FSplit splits the file name specified by Path into its three  */
/* components. Dir is set to the drive and directory path with   */
/* any leading and trailing backslashes, Name is set to the file */
/* name, and Ext is set to the extension with a preceding dot.   */
/* Each of the component strings may possibly be empty, if Path  */
/* contains no such component.                                   */

void FSplit(PathStr Path, DirStr & Dir,
	NameStr & Name, ExtStr & Ext);

/* EnvCount returns the number of strings contained in the DOS   */
/* environment.                                                  */

integer EnvCount();

/* EnvStr returns a specified environment string. The returned   */
/* string is of the form "VAR=VALUE". The index of the first     */
/* string is one. If Index is less than one or greater than      */
/* EnvCount, EnvStr returns an empty string.                     */

string EnvStr(integer Index);

/* GetEnv returns the value of a specified environment variable. */
/* The variable name can be in upper or lower case, but it must  */
/* not include the '=' character. If the specified environment   */
/* variable does not exist, GetEnv returns an empty string.      */

string GetEnv(string EnvVar);

/* SwapVectors swaps the contents of the SaveIntXX pointers in   */
/* the System unit with the current contents of the interrupt    */
/* vectors. SwapVectors is typically called just before and just */
/* after a call to Exec. This insures that the Exec'd process    */
/* does not use any interrupt handlers installed by the current  */
/* process, and vice versa.                                      */

void SwapVectors();

/* Keep (or Terminate Stay Resident) terminates the program and  */
/* makes it stay in memory. The entire program stays in memory,  */
/* including data segment, stack segment, and heap. The ExitCode */
/* corresponds to the one passed to the Halt standard procedure. */

void Keep(word ExitCode);

/* Exec executes another program. The program is specified by    */
/* the Path parameter, and the command line is specified by the  */
/* CmdLine parameter. To execute a DOS internal command, run     */
/* COMMAND.COM, e.g. "Exec('\COMMAND.COM','/C DIR *.PAS');".     */
/* Note the /C in front of the command. Errors are reported in   */
/* DosError. When compiling a program that uses Exec, be sure    */
/* to specify a maximum heap size as there will otherwise not be */
/* enough memory.                                                */

void Exec(PathStr Path, ComStr ComLine);

/* DosExitCode returns the exit code of a sub-process. The low   */
/* byte is the code sent by the terminating process. The high    */
/* byte is zero for normal termination, 1 if terminated by       */
/* Ctrl-C, 2 if terminated due to a device error, or 3 if        */
/* terminated by the Keep procedure (function call 31 hex).      */

word DosExitCode();

#endif
