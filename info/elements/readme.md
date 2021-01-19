# Out-of-bounds elements information

The .info files contain dumps of the element information past the natural
boundary (MAX_STAT). The summary is: most elements/objects have a zero (false)
HasDrawProc and will thus behave like ordinary text (of unusual color). Some
have HasDrawProc set to true; most of these crash. As every object crashes on
touch, Walkable never comes into play.

ALIENOBJ.ZZT contains objects that have HasDrawProc set to true but don't
crash.

DARK.ZZT demonstrates that object 0x36 (blinking text) is visible in the dark.

Since some of the out-of-bounds data depends on what has been loaded already,
I've made multiple .info files: the first one is after starting ZZT with no
TOWN.ZZT available. The second is after having played the beginning of Fred! 1
(FRED1GLD.ZZT). The last two are after another blank start (TOWN.ZZT is
unavailable), but one is with no ZZT.DAT and another with.

## Experimental results

For Pascal booleans, zero is false and nonzero is true.

Experimental results for tiles with nonzero HasDrawProc:

|ALIENOBJ code|Element number|Behavior|
|-------------|--------------|--------|
|A            | 67           | crash  |
|B            | 68           | crash  |
|C            | 70           | crash (runtime error 200)|
|D            | 71           | crash  |
|E            | 74           | shows blinking asterisk-ish symbol (0xF, same as color) |
|F            | 75           | shows blinking asterisk-ish symbol |
|G            | 76           | shows blinking asterisk-ish symbol |
|H            | 77           | shows blinking asterisk-ish symbol |
|I            | 78           | shows asterisk-ish symbol |
|J            | 79           | shows asterisk-ish symbol |
|K            | 83           | crash |
|L            | 84           | shows key symbol (0xC), visible in dark, may change earance when dark is turned on and off. |
|M            | 85           | crash |
|N	      | 86           | shows blinking asterisk-ish symbol |
|O            | 108          | crash |
|P            | 116          | shows key symbol, visible in dark, may change appearance when dark is turned on and off. (DrawProc points to the interrupt table) |

Some of these drastically slow down the editor or cause the game to behave
unpredictably. All the objects will crash ZZT upon touch.

The Dosbox debugger shows that e.g. element 75 calls 5247:4E41 (as given by its
DrawProc), which then lands in the middle of a zero sequence, which translates
into add [bx+si],al, in turn repeatedly firing INT 6. Somehow, it manages to
recover from the jump, but I guess that's what's slowing ZZT down.

These non-crash DrawProcs end up in the middle of a zero sequence:

- 74 (26F2:7E50)
- 75 (5247:4E41)
- 76 (468B:FFFF)
- 77 (C4E4:3005)
- 78 (76FF:1D0D)
- 79 (8904:452B)

Element 116 jumps to 155B:1428, which is a code sequence starting B2 D7 EB 03
BA B3 D7... This is an offset into function FUN_23b4_13e7 (as given by Ghidra);
the actual function starts BA B2 D7... However, as B2 D7 is a valid opcode, it
gets back on track and follows the function flow afterwards, so these elements
don't crash. I couldn't figure out why it turns into a key shape, though; or
why it changes on +/- dark, and if there's any pattern to it.

Only a few elements (74, 75, and 116) have non-null TouchProcs that don't end
up in a zero sequence. These crash the game with a runtime error instead of
hanging.

The following non-crash DrawProc elements have a TickProc that lead somewhere
else than into a zero sequence:

- 116 (0000:F000)
- 54 (0001:0084)
- 74 (0036:BA02)
- 77 (0245:0326)

I don't know what they do, but I doubt they would do anything interesting since
functions that change the board have to be crafted to do so: just jumping to
a random location is unlikely to suffice.
