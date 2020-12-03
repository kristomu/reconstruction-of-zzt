#include "ptoc.h"
#include "array.h"

const integer maxint_ = 2147483647;

typedef unsigned short word1;
const int min_word1 = 0;
const int max_word1 = 65535;
typedef signed char byte1;
const int min_byte1 = -128;
const int max_byte1 = 127;

text input1;
text output1;

extern integer inc1(integer x);

extern integer dec1(integer x);

extern real abs_(real x);

extern real arctan(real x);

extern integer bitsize_(integer x);

extern integer size(integer x);

extern char chr_(integer x);

extern real cos1(real x);

extern boolean eof_(text f);

extern boolean eoln_(text f);

extern real exp1(real x);

extern boolean ioerror_(text f);

extern integer iostatus_(text f);

extern real ln(real x);

extern boolean odd_(integer x);

extern integer ord_(char c);

extern integer pred_(integer x);

extern integer* ref1(integer x);

extern integer round1(real x);

extern real sin1(real x);

extern real sqr_(real x);

extern real sqrt1(real x);

extern integer succ_(integer x);

extern real time1();

extern void timestamp1(integer& day, integer& month, integer& year,
                       integer& hour, integer& min, integer& sec);

extern integer  trunc_(real x);

extern void break_(text x);

extern void close1(text x);

extern void delete_(text x);

extern void get_(text x);

extern void noioerror_();

/*extern void pack_(conf_array<integer> a, integer i, integer z);

extern void unpack_(integer z, conf_array<integer> a, integer i);*/

extern void page_(text f);

extern void put_(text x);

extern void assign1(text f, char* name );


extern void rename_(text f, char* name );


extern void open(text f, char* name, char* history,
                 integer& error_code );


extern void reset_(text f, char* name, char* options,
                   integer& error_code );


extern void rewrite_(text f, char* name, char* options,
                     integer& error_code);


extern void seek_(text s, integer offset);


