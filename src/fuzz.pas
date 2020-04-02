{
        Copyright (c) 2020 Kristofer Munsterhjelm

        Permission is hereby granted, free of charge, to any person obtaining a copy
        of this software and associated documentation files (the "Software"), to deal
        in the Software without restriction, including without limitation the rights
        to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
        copies of the Software, and to permit persons to whom the Software is
        furnished to do so, subject to the following conditions:

        The above copyright notice and this permission notice shall be included in all
        copies or substantial portions of the Software.

        THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
        IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
        FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
        AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
        LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
        OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
        SOFTWARE.
}

{ Fuzzing-related functions. }

unit Fuzz;

interface
	procedure DisableSignalHandlers;
	procedure Wait(ms: Cardinal);
var
    TFuzzMode : boolean = false;     {Timer fuzz mode}

implementation

uses BaseUnix, Crt;

{ https://www.freepascal.org/docs-html/rtl/baseunix/fpsigaction.html }
procedure DisableSignalHandlers;
    var
        oa,na : PSigActionRec;
    begin
        { Disable Free Pascal's signal handler for SIGILL and SIGSEGV.}
        new(na);
        new(oa);
        na^.sa_Handler:=sigactionhandler(SIG_DFL);
        fillchar(na^.Sa_Mask,sizeof(na^.sa_mask),#0);
        na^.Sa_Flags:=0;
        na^.Sa_Restorer:=nil;

        fpSigAction(SIGILL,na,oa);
        fpSigAction(SIGSEGV,na,oa);
    end;

procedure Wait(ms: Cardinal);
    begin
        if TFuzzMode then Exit;
        Delay(ms);
    end;

begin
    TFuzzMode := false;

end.
