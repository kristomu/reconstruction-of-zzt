#pragma once

#include <map>

/* Credits go to Ben Russell (iamgreaser) for this table. */
/* Note, 0xED now redirects to U+03D5 as identified by IBM;
    see Wikipedia note. https://bit.ly/39t1732 Better still
    would be U+1D719.*/
extern wchar_t cp437ToUnicode[];

extern std::map<wchar_t, unsigned char>  UnicodeToCP437;

void SetupCodepointToCP437();
unsigned char CodepointToCP437(wchar_t cp);
wchar_t CP437ToCodepoint(unsigned char c);