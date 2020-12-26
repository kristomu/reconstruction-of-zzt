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

{ A linked list structure for stats so as to deal with statId invalidation
  bugs once and for all.

  statId invalidation bugs happen when a stat with a high index destroys a
  stat with a low index. Then its index no longer points to the same stat,
  and may even go out of bounds. Linked lists fix this since the node
  pointer is always valid unless the node itself has been destroyed. }

{ But what about leader and follower? BLEH... }

{ Adapted from https://forum.lazarus.freepascal.org/index.php?topic=40753.0 }
unit Statslist;

interface
	uses GameVars, TxtWind;

	type
		PStatNode = ^TStatNode;

	TStatNode = packed record
		Stat : TStat;
		Next : PStatNode;
	end;

	procedure Append(const aList, aNode: PStatNode);
	procedure Delete(const aList, aNode: PStatNode);

implementation

procedure Append(const aList, aNode:PStatNode);
	var
		vLast: PStatNode;
	begin
		vLast := aList;
		if assigned(vLast) then begin
			while vlast^.Next <> nil do
				vLast := vLast^.Next;
			(vLast^).Next := aNode;
			aNode^.Next := nil;
		end;
	end;

procedure Delete(const aList, aNode:PStatNode);
	var
		vPrev, vNext:PStatNode;
	begin
		vPrev := aList;
		repeat
			if assigned(vPrev) and (vPrev^.Next <> aNode) then
				vPrev := vPrev^.Next;
		until vPrev^.Next = aNode;

		if vPrev^.Next = aNode then begin
			vNext := (vPrev^.Next)^.Next;
			vPrev^.Next := vNext;
			aNode^.Next := nil;
		end;
	end;
end.