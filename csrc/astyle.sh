# Java brace style, C/C++ code, tab indentation, 75 character lines,
# indent (not align) after parens, align references to middle,
# indent classes and case switches, and keep one-line statements
# and if spacing to parens.

astyle -t -A2 -o --mode=c -xC75 -W2 -S -U -xU -H $1
