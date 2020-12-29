#pragma once

#include <map>

class unicode_converter {
	private:
		/* Credits go to Ben Russell (iamgreaser) for the CP437 to
		   Unicode table.
		   Note, 0xED now redirects to U+03D5 as identified by IBM;
		   see Wikipedia note. https://bit.ly/39t1732 */
		const wchar_t cp437_to_unicode[256];
		std::map<wchar_t, unsigned char> unicode_to_CP437;

		void setup_conversion_table();

	public:
		unicode_converter();

		unsigned char codepoint_to_CP437(wchar_t cp) const;
		wchar_t CP437_to_codepoint(unsigned char c) const;
};