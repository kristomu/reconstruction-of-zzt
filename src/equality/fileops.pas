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
	function ErrorString(e: Integer): string;
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

{https://forum.lazarus.freepascal.org/index.php/topic,46911.0.html}
function ErrorString(e: Integer): string;
	var
		numericError: string;
	begin
		case e of
			  1: ErrorString := 'Invalid function number';
			  2: ErrorString := 'File not found.';
			  3: ErrorString := 'Path not found.';
			  4: ErrorString := 'Too many open files.';
			  5: ErrorString := 'Access denied.';
			  6: ErrorString := 'Invalid file handle.';
			 12: ErrorString := 'Invalid file-access mode.';
			 15: ErrorString := 'Invalid disk number.';
			 16: ErrorString := 'Cannot remove current directory.';
			 17: ErrorString := 'Cannot rename across volumes.';
			100: ErrorString := 'Error when reading from disk.';
			101: ErrorString := 'Error when writing to disk.';
			102: ErrorString := 'File not assigned.';
			103: ErrorString := 'File not open.';
			104: ErrorString := 'File not opened for input.';
			105: ErrorString := 'File not opened for output.';
			106: ErrorString := 'Invalid number.';
			150: ErrorString := 'Disk is write protected.';
			151: ErrorString := 'Unknown device.';
			152: ErrorString := 'Drive not ready.';
			153: ErrorString := 'Unknown command.';
			154: ErrorString := 'CRC check failed.';
			155: ErrorString := 'Invalid drive specified..';
			156: ErrorString := 'Seek error on disk.';
			157: ErrorString := 'Invalid media type.';
			158: ErrorString := 'Sector not found.';
			159: ErrorString := 'Printer out of paper.';
			160: ErrorString := 'Error when writing to device.';
			161: ErrorString := 'Error when reading from device.';
			162: ErrorString := 'Hardware failure.';
			200: ErrorString := 'Division by zero';
			201: ErrorString := 'Range check error';
			202: ErrorString := 'Stack overflow error';
			203: ErrorString := 'Heap overflow error';
			204: ErrorString := 'Invalid pointer operation';
			205: ErrorString := 'Floating point overflow';
			206: ErrorString := 'Floating point underflow';
			207: ErrorString := 'Invalid floating point operation';
			210: ErrorString := 'Object not initialized';
			211: ErrorString := 'Call to abstract method';
			212: ErrorString := 'Stream registration error';
			213: ErrorString := 'Collection index out of range';
			214: ErrorString := 'Collection overflow error';
			215: ErrorString := 'Arithmetic overflow error';
			216: ErrorString := 'General protection fault';
			else begin
				Str(e, numericError);
				ErrorString := '#' + numericError;
			end;
		end;
	end;

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
		{ Somewhat of a kludge to make it work both with existing
		  write-only files and files that don't exist yet. It might
		  have trouble with write-only directories. I don't know why
		  Rewrite doesn't respect FileMode yet Reset does... }

		FileMode := FILE_WRITE_ONLY;
		Reset(f, l);
		if IOResult = 0 then Exit;

		{ Couldn't open existing file. Try opening a new one. }
		Rewrite(f, l);
	end;

procedure OpenForWrite(var f: TypedFile);
	begin
		FileMode := FILE_WRITE_ONLY;
		Reset(f);
		if IOResult = 0 then Exit;
		Rewrite(f);
	end;

procedure OpenForWrite(var f: Text);
	begin
		FileMode := FILE_WRITE_ONLY;
		Reset(f);
		if IOResult = 0 then Exit;
		Rewrite(f);
	end;

end.
