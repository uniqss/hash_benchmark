/* just some stuff for later, on smhasher-style hash functions
 * here is the fibonacci hash, inspired by this:
 * https://probablydance.com/2018/06/16/fibonacci-hashing-the-optimization-that-the-world-forgot-or-a-better-alternative-to-integer-modulo/
 * oh, and this is cool also:
 * https://lemire.me/blog/2016/06/27/a-fast-alternative-to-the-modulo-reduction/
 * have fun!
 */

#include <stdint.h>

 /* some Fibonacci constants for a number of bits b
  * floor(2^(b)/ ((1 + √5)/2))
  * 2^8 = 158
  * 2^16 = 40503
  * 2^32 = 2654435769
  * 2^64 = 11400714819323198485
  * 2^128 = 210306068529402873165736369884012333108
  * 2^256 = 71563446777022297856526126342750658392501306254664949883333486863006233104021
  * 2^512 = 8286481015334893988907527251732611664457280877896990125350747801032912124181934735572335005532987901856694870697621088413914768940958605061563703415234102
  * 2^1024 = 111103545868725975185578674770941253548085647175230593275709448371553253006147314587817210981315418038152651252381241848663172381125005611474547150822121573954448475698342462132071640602726690043967503533098589415135032684552325470109005299145741049782192534336153650840562560032276382145192816865771520714469
  */

void fib_hash64(const void* key, int len, uint32_t seed, void* out) {
    const uint64_t fib = 11400714819323198485llu;
    const uint64_t* data = (const uint64_t*)key;
    // 8 bytes in a 64-bit value, so take the last 7 bits
    uint8_t m = len & 7;
    // initialize
    uint64_t h = seed + len;

    for (int i = 0; i < len / 8; i++) {
        h = h * fib + data[i];
    }

    if (m) {
        uint8_t rem = len - m;
        const uint8_t* d = (const uint8_t*)key;

        uint64_t last = 0;
        switch (m) {
        case 7: last = d[rem++];
        case 6: last = (last << 8) | d[rem++];
        case 5: last = (last << 8) | d[rem++];
        case 4: last = (last << 8) | d[rem++];
        case 3: last = (last << 8) | d[rem++];
        case 2: last = (last << 8) | d[rem++];
        case 1: last = (last << 8) | d[rem++];
        }
        h = h * fib + last;
    }

    *(uint64_t*)out = h;
}

void fib_hash32(const void* key, int len, uint32_t seed, void* out) {
    const uint32_t fib = 2654435769u;
    const uint32_t* data = (const uint32_t*)key;

    // 4 bytes in a 32-bit value, so take the last 3 bits
    uint8_t m = len & 3;

    // initalize
    uint32_t h = seed + len;

    for (int i = 0; i < len / 4; i++) {
        h = h * fib + data[i];
    }

    if (m) {
        uint8_t rem = len - m;
        const uint8_t* d = (const uint8_t*)key;

        uint32_t last = 0;
        switch (m) {
        case 3: last = d[rem++];
        case 2: last = (last << 8) | d[rem++];
        case 1: last = (last << 8) | d[rem++];
        }
        h = h * fib + last;
    }

    *(uint32_t*)out = h;
}
void fib_hash16(const void* key, int len, uint16_t seed, void* out) {
    const uint16_t fib = 40503;
    const uint16_t* data = (const uint16_t*)key;

    // 1 bytes in a 16-bit value, so take the last 1 bits
    uint8_t m = len & 1;

    // initalize
    uint16_t h = seed + len;

    for (int i = 0; i < len / 2; i++) {
        h = h * fib + data[i];
    }

    if (m) {
        uint8_t rem = len - m;
        const uint8_t* d = (const uint8_t*)key;

        uint16_t last = 0;
        switch (m) {
        case 1: last = d[rem++];

        }
        h = h * fib + last;
    }

    *(uint16_t*)out = h;
}