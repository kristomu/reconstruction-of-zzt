#include "random.h"

#include <random>
#include <stdexcept>

// The seed can't be 0.
void rng::seed(int64_t in) {
	if (in == 0) {
		state = 1;
	} else {
		state = in;
	}
}

// From Quadelect. https://github.com/kristomu/quadelect/
void rng::seed() {
	// std::random_device is a C++ hardware RNG class. It returns
	// unsigned 32-bit ints, so we need two of them.
	std::random_device rd;
	uint64_t seed_in = 0;

	// Guard against a 2^-64 probability event and platforms where
	// rd is not random at all, but returns 0 all the time.
	for (int i = 0; i < 10 && seed_in == 0; ++i) {
		uint64_t a = rd(), b = rd();
		seed_in = (a << 32ULL) + b;
	}

	if (seed_in == 0) {
		throw std::runtime_error(
			"RNG: std::random_device always returns 0");
	}

	seed(seed_in);
}

uint64_t rng::xorshift64s() {
	uint64_t x = state;	/* The state must be seeded with a nonzero value. */
	x ^= x >> 12; // a
	x ^= x << 25; // b
	x ^= x >> 27; // c
	state = x;
	return x * UINT64_C(0x2545F4914F6CDD1D);
}

// Passes BigCrush: see the Wikipedia article.
uint32_t rng::high_bits_random() {
	return xorshift64s() >> 32ULL;
}

unsigned short rng::randint(unsigned short less_than) {
	// https://stackoverflow.com/questions/11758809

	unsigned short remainder = ULONG_MAX % less_than;
	uint32_t x;
	do {
		x = high_bits_random();
	} while (x >= ULONG_MAX - remainder);

	return x % less_than;
}