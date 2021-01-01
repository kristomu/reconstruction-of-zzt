#pragma once

// Wikipedia's implementation of xorshift*,
// https://en.wikipedia.org/w/index.php?title=Xorshift&oldid=989828061#xorshift*

// This should be simple enough to port to FreePascal if we want
// synchronization later. (Not the seed routine, but I don't care; fuzzt
// will use a constant anyway.)

#include <cstdint>
#include <limits.h>

class rng {
	private:
		uint64_t state;
		uint64_t xorshift64s();

		uint32_t high_bits_random();

	public:
		rng() {
			seed(1);
		}
		void seed(int64_t in);
		void seed();
		// returns a value on [0...less_than)
		unsigned short randint(unsigned short less_than);
};