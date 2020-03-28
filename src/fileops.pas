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

{ A simple file-handling shim for Linux. All of these functions alter IOResult,
  as is the convention in ZZT. }

{$I-}

unit Fileops;

interface
	procedure OpenForRead(var f: file; l: LongInt);
	procedure OpenForRead(var f: TypedFile);
	procedure OpenForRead(var f: Text);
	procedure OpenForWrite(var f: file; l: LongInt);
	procedure OpenForWrite(var f: TypedFile);
	procedure OpenForWrite(var f: Text);

	const
		FILE_READ_ONLY = 0;
		FILE_WRITE_ONLY = 1;
		FILE_READ_WRITE = 2;

implementation
uses Dos;

procedure OpenForRead(var f: file; l: LongInt);
	begin
		FileMode := FILE_READ_ONLY;
		Reset(f, l);
	end;

procedure OpenForRead(var f: TypedFile);
	begin
		FileMode := FILE_READ_ONLY;
		Reset(f);
	end;

procedure OpenForRead(var f: Text);
	begin
		FileMode := FILE_READ_ONLY;
		Reset(f);
	end;

procedure OpenForWrite(var f: file; l: LongInt);
	begin
		FileMode := FILE_WRITE_ONLY;
		Reset(f, l);			{ Not Rewrite! I dunno why... }
	end;

procedure OpenForWrite(var f: TypedFile);
	begin
		FileMode := FILE_WRITE_ONLY;
		Reset(f);
	end;

procedure OpenForWrite(var f: Text);
	begin
		FileMode := FILE_WRITE_ONLY;
		Reset(f);
	end;

end.
