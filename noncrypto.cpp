#include <iostream>
#include <fstream>
#include <cstdlib>
#include <cstring>
#include <intrin.h>

__m128i falkhash(	_In_reads_bytes_(len) void* pbuf,	_In_                  uint64_t  len,	_In_                  uint64_t  pseed);


#include <algorithm>
#include <array>
#include <iostream>
#include <random>
#include <string>


#include "City.h"
#include "CityCrc.h"
#include "farmhash.h"
#include "farmhash-c.h"
#include "xxh3.h"
#include "xxhash.h"
#include "xxh3.h"
#include "mum.h"

#include "MurmurHash1.h"
#include "MurmurHash2.h"
#include "MurmurHash3.h"
#include "SpookyV2.h"

#include "metrohash.h"
#include "metrohash128.h"

#include "t1ha.h"
#include "pearson.h"
#include "fnv.h"
#include "fnv128.h"
#include "halftime-hash.hpp"
#include "highwayhash.h"
#include "siphash.h"
#include "umash.h"
#include "clhash.h"
#include "wyhash.h"
#include "wyhash32.h"
#include "vmac.h"
#include "lookup3.h"

void fib_hash64(const void* key, int len, uint32_t seed, void* out);
void fib_hash32(const void* key, int len, uint32_t seed, void* out);
void fib_hash16(const void* key, int len, uint16_t seed, void* out);

#define hash_pearson
#define hash_t1ha
#define hash_spooky
#define hash_murmur
#define hash_mum
#define hash_xxhash
#define hash_city
#define hash_farm
#define hash_metro
#define hash_fnv
#define hash_halftime
#define hash_highway
#define hash_siphash
#define hash_clhash
#define hash_falkhash
#define hash_wyhash
#define hash_fib
#define hash_lookup3

using namespace std;
using std::cout;
using std::cin;


typedef uint64_t uint64;
typedef uint32_t uint32;


std::vector<char> HexToBytes(const std::string& hex) {
	std::vector<char> bytes;

	for (unsigned int i = 0; i < hex.length(); i += 2) {
		std::string byteString = hex.substr(i, 2);
		char byte = (char)strtol(byteString.c_str(), NULL, 16);
		bytes.push_back(byte);
	}

	return bytes;
}



uint64_t swapVal(uint64_t x)
{
	uint64_t y;
	char* px = (char*)&x;
	char* py = (char*)&y;
	for (int i = 0; i < sizeof(uint64_t); i++)
		py[i] = px[sizeof(uint64_t) - 1 - i];
	return y;
}


void byte_swap32(unsigned int* pVal32) {

	unsigned char* pByte;

	pByte = (unsigned char*)pVal32;
	// move lsb to msb
	pByte[0] = pByte[0] ^ pByte[3];
	pByte[3] = pByte[0] ^ pByte[3];
	pByte[0] = pByte[0] ^ pByte[3];
	// move lsb to msb
	pByte[1] = pByte[1] ^ pByte[2];
	pByte[2] = pByte[1] ^ pByte[2];
	pByte[1] = pByte[1] ^ pByte[2];

}


#define BUFFSIZE 128
int main(int argc, char* argv[])
{

	string str = "abc";

	char buff[128];

	uint32 u32;
	uint64 u64;

	uint128_c_t u128;
	int seed = 0;
	char hexdata[] = "0123456789ABCDEF";

	strcpy_s(buff, str.c_str());

	if (argc >= 2) {
		if (strlen(argv[1]) > BUFFSIZE - 1) return(0);
		strcpy_s(buff, argv[1]);
	}
	if (argc >= 3) seed = atoi(argv[2]);
	
	if (argc >= 4)
	{
		
		if (strlen(argv[3]) != 16) {
			printf("Encryption key should have 16 characters");
			return(0);
		}
		strcpy_s(hexdata, argv[3]);
		
		printf("Key: %s\n",hexdata);
	} 
	

	printf("String: %s\n", buff);
	printf("Seed: %d\n\n", seed);

#ifdef hash_city
	
	puts("===City Hash===");
	uint32 u32c = CityHash32(buff, strlen(buff));
	printf("CityHash32:\t\t%lu Hex: %lx\n", u32c, u32c);
//	uint32 u32c = CityHash32WithSeed(buff, strlen(buff),(uint32) seed);
//	printf("CityHash32+Seed:\t%lu Hex: %lx\n", u32c, u32c);
	
	u64 = CityHash64(buff, strlen(buff));
	printf("CityHash64:\t\t%llu Hex: %llx\n", u64, u64);
	u64 = CityHash64WithSeed(buff, strlen(buff), (uint64)0);
	printf("CityHash64+seed:\t%llu Hex: %llx\n", u64, u64);



//	uint128 u128c = CityHashCrc128(buff, strlen(buff));
//	printf("CityHashCRC128:\t\t%llx%llx\n", u128c.first, u128c.second);

	uint128 seedv;
	seedv.second = seed;
	seedv.first = 0;

	uint128 u128h = CityHash128(buff, strlen(buff));
	printf("CityHash128:\t\t%llx%llx\n", u128h.first, u128h.second);

	u128h = CityHash128WithSeed(buff, strlen(buff), seedv);
	printf("CityHash128+seed:\t%llx%llx\n\n", u128h.first, u128h.second);





#endif // hash_city

#ifdef hash_farm


	puts("===Farm Hash===");
	u32 = farmhash(buff, strlen(buff));

	printf("FarmHash\t\t\t%lu Hex: %lx\n", u32, u32);

	u32 = farmhash32(buff, strlen(buff));
	printf("FarmHash32\t\t\t%lu Hex: %lx\n", u32, u32);
	u32 = farmhash32_with_seed(buff, strlen(buff), (uint32_t)seed);
	printf("FarmHash32+seed\t\t\t%lu Hex: %lx\n", u32, u32);


	u64 = farmhash64(buff, strlen(buff));
	printf("FarmHashm64:\t\t\t%llu Hex: %llx\n", u64, u64);
	u64 = farmhash64_with_seed(buff, strlen(buff), (uint64_t)seed);
	printf("FarmHash64+seed:\t\t%llu Hex: %llx\n", u64, u64);


	u128 = farmhash128(buff, strlen(buff));
	printf("FarmHash128:\t\t\t%llx%llx\n", u128.b, u128.a);
	uint128_c_t seedv2;
	seedv2.a = (uint64_t) seed;
	seedv2.b = 0;
	u128 = farmhash128_with_seed(buff, strlen(buff), seedv2);
	printf("FarmHash128+seed:\t\t%llx%llx\n", u128.b, u128.a);

	u32 = farmhash_fingerprint32(buff, strlen(buff));
	printf("FarmHash32 fingerprint\t\t%lu Hex: %x\n", u32, u32);

	u64 = farmhash_fingerprint64(buff, strlen(buff));
	printf("FarmHash64 fingerprint\t\t%llu Hex: %llx\n", u64, u64);

	u128 = farmhash_fingerprint128(buff, strlen(buff));
	printf("FarmHash128 fingerprint:\t%llx%llx\n", u128.b, u128.a);
#endif

#ifdef hash_xxhash
	puts("\n===xxHash===");
	XXH32_hash_t u32x = XXH32(buff, strlen(buff), (XXH32_hash_t)0);
	printf("xx3Hash32\t\t\t%lu Hex: %lx\n", u32x, u32x);
	u32x = XXH32(buff, strlen(buff), (XXH32_hash_t)seed);
	printf("xx3Hash32+Seed\t\t\t%lu Hex: %lx\n", u32x, u32x);

	XXH64_hash_t u64x = XXH64(buff, strlen(buff), (XXH64_hash_t)0);
	printf("xx3Hash64\t\t\t%lld Hex: %llx\n", u64x, u64x);
	u64x = XXH64(buff, strlen(buff), (XXH64_hash_t)seed);
	printf("xx3Hash64+Seed\t\t\t%llu Hex: %llx\n", u64x, u64x);

	XXH128_hash_t u128x = XXH128(buff, strlen(buff), (XXH64_hash_t)0);
	printf("xx3Hash128\t\t\t%llx%llx\n", u128x.high64, u128x.low64);


	u128x = XXH128(buff, strlen(buff), (XXH64_hash_t)seed);
	printf("xx3Hash128+Seed\t\t\t%llx%llx\n", u128x.high64, u128x.low64);
#endif

#ifdef hash_mum

	puts("\n===Mum===");
	uint64_t u64m = mum_hash(buff, strlen(buff), (uint64_t)0);
	printf("Mum\t\t\t%llu Hex: %llx\n", u64m, u64m);
#endif

#ifdef hash_murmur

	puts("\n===Murmur===");
	uint32_t u32m = MurmurHash1(buff, strlen(buff), (uint32_t)0);
	printf("MurmurHash1:\t\t\t%lu Hex: %lx\n", u32m, u32m);

	u32m = MurmurHash1(buff, strlen(buff), (uint32_t)seed);
	printf("MurmurHash1+Seed:\t\t\t%lu Hex: %lx\n", u32m, u32m);

	u32m = MurmurHash2(buff, strlen(buff), (uint32_t)0);
	printf("MurmurHash2:\t\t\t%lu Hex: %lx\n", u32m, u32m);

	u32m = MurmurHash2(buff, strlen(buff), (uint32_t)seed);
	printf("MurmurHash2+Seed:\t\t\t%lu Hex: %lx\n", u32m, u32m);

	uint32_t h1;
	MurmurHash3_x86_32(buff, strlen(buff), 0, &h1);
	printf("MurmurHash3 32 bit:\t\t\t%lu Hex: %lx\n", h1, h1);

	MurmurHash3_x86_32(buff, strlen(buff), (uint32_t)seed, &h1);
	printf("MurmurHash3 32+Seed:\t\t\t%lu Hex: %lx\n", h1, h1);

	uint32_t h2[4];
	MurmurHash3_x86_128(buff, strlen(buff), 0, h2);
	byte_swap32(&h2[0]); 	byte_swap32(&h2[1]); 	byte_swap32(&h2[2]); 	byte_swap32(&h2[3]);
	printf("MurmurHash3 128 bit (x86):\t\t%lx%lx%lx%lx\n", h2[0], h2[1], h2[2], h2[3]);

	MurmurHash3_x86_128(buff, strlen(buff), (uint32_t)seed, h2);
	byte_swap32(&h2[0]); 	byte_swap32(&h2[1]); 	byte_swap32(&h2[2]); 	byte_swap32(&h2[3]);
	printf("MurmurHash3 128 bit (x86)+Seed:\t\t%lx%lx%lx%lx\n", h2[0], h2[1], h2[2], h2[3]);

	MurmurHash3_x64_128(buff, strlen(buff), 0, h2);
	byte_swap32(&h2[0]); 	byte_swap32(&h2[1]); 	byte_swap32(&h2[2]); 	byte_swap32(&h2[3]);
	printf("MurmurHash3 128 bit (x68):\t\t%lx%lx%lx%lx\n", h2[0], h2[1], h2[2], h2[3]);

	MurmurHash3_x64_128(buff, strlen(buff), (uint32_t)seed, h2);
	byte_swap32(&h2[0]); 	byte_swap32(&h2[1]); 	byte_swap32(&h2[2]); 	byte_swap32(&h2[3]);
	printf("MurmurHash3 128 bit (x68)+Seed:\t\t%lx%lx%lx%lx\n", h2[0], h2[1], h2[2], h2[3]);
#endif

#ifdef hash_spooky
	{
		puts("\n===Spooky V2===");

		uint32 u32s;
		uint64 u64s, u64s2;
		u32s = SpookyHash::Hash32(buff, strlen(buff), (uint32_t)0);
		printf("SpookyV2 32\t\t\t%lu Hex: %lx\n", u32s, u32s);
		u32s = SpookyHash::Hash32(buff, strlen(buff), (uint32_t)seed);
		printf("SpookyV2 32+Seed\t\t%lu Hex: %lx\n", u32s, u32s);

		u64s = SpookyHash::Hash64(buff, strlen(buff), (uint64_t)0);
		printf("SpookyV2 64\t\t\t%llu Hex: %llx\n", u64s, u64s);

		u64s = SpookyHash::Hash64(buff, strlen(buff), (uint64_t)seed);
		printf("SpookyV2 64+Seed\t\t%llu Hex: %llx\n", u64s, u64s);

		u64s = 0; //seed
		u64s2 = 0;

		SpookyHash::Hash128(buff, strlen(buff), &u64s, &u64s2);

		printf("SpookyV2 128\t\t\t%llx%llx\n", swapVal(u64s), swapVal(u64s2));
		u64s = seed; //seed
		u64s2 = 0;
		SpookyHash::Hash128(buff, strlen(buff), &u64s, &u64s2);
		printf("SpookyV2 128+Seed\t\t%llx%llx\n", swapVal(u64s), swapVal(u64s2));
	}

#endif	
#ifdef hash_metro
	puts("\n===Metro===");
	
	uint8_t  hash[8];
	uint64_t h;
	h=MetroHash64::Hash((uint8_t*)buff, strlen(buff), hash,(uint64_t)0);

	printf("MetroHash 64:\t\t\t%llu Hex: %02x%02x%02x%02x%02x%02x%02x%02x\n",h,hash[7],hash[6], hash[5], hash[4], hash[3], hash[2], hash[1], hash[0]);

	h = MetroHash64::Hash((uint8_t*)buff, strlen(buff), hash, (uint64_t)seed);

	printf("MetroHash 64+Seed:\t\t%llu Hex: %02x%02x%02x%02x%02x%02x%02x%02x\n", h, hash[7], hash[6], hash[5], hash[4], hash[3], hash[2], hash[1], hash[0]);


	uint8_t  hash1[16];
	MetroHash128::Hash((uint8_t*)buff, strlen(buff), hash1, (uint64_t)0);

	printf("MetroHash 128:\t\t\tHex: %02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x\n", hash1[15], hash1[14], hash1[13], hash1[12], hash1[11], hash1[10], hash1[9], hash1[8], hash1[7], hash1[6], hash1[5], hash1[4], hash1[3], hash1[2], hash1[1], hash1[0]);
	MetroHash128::Hash((uint8_t*)buff, strlen(buff), hash1, (uint64_t)seed);

	printf("MetroHash 128+seed:\t\tHex: %02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x\n", hash1[15], hash1[14], hash1[13], hash1[12], hash1[11], hash1[10], hash1[9], hash1[8], hash1[7], hash1[6], hash1[5], hash1[4], hash1[3], hash1[2], hash1[1], hash1[0]);

	
#endif
#ifdef hash_t1ha

	puts("\n===t1ha===");
	uint64_t u64t;
	uint32_t u32t;

	u32t = t1ha0_32le(buff, strlen(buff), (uint64_t)0);
	printf("t1ha0 32(le)\t\t\t%lu Hex: %lx\n", u32t, u32t);

	u64t=t1ha0(buff, strlen(buff), (uint64_t)0);
	printf("t1ha0 64\t\t\t%llu Hex: %llx\n", u64t, u64t);
	u64t = t1ha0(buff, strlen(buff), (uint64_t)seed);
	printf("t1ha0 64+Seed\t\t\t%llu Hex: %llx\n", u64t, u64t);

	u64t = t1ha1_le(buff, strlen(buff), (uint64_t)0);
	printf("t1ha1 64\t\t\t%llu Hex: %llx\n", u64t, u64t);
	u64t = t1ha1_le(buff, strlen(buff), (uint64_t)seed);
	printf("t1ha1 64+Seed\t\t\t%llu Hex: %llx\n", u64t, u64t);

	u64t = t1ha2_atonce(buff, strlen(buff), (uint64_t)0);
	printf("t1ha2 64\t\t\t%llu Hex: %llx\n", u64t, u64t);
	u64t = t1ha2_atonce(buff, strlen(buff), (uint64_t)seed);
	printf("t1ha2 64+Seed\t\t\t%llu Hex: %llx\n", u64t, u64t);

	uint64_t  hashlower,hashupper=0;
	hashlower=t1ha2_atonce128(&hashupper, buff, strlen(buff), (uint64_t)seed);
	printf("t1ha2 128\t\t\t%llx%llx\n", hashupper, hashlower);

#endif


#ifdef hash_pearson

	puts("\n===Pearson (using AES S-box)===");
	uint32_t u32p=0;
	uint64_t u64p=0;
	uint8_t  hashp[16];

	u32p=pearson_hash_32((uint8_t *)buff, strlen(buff), (uint32_t)u32p);
	printf("Pearson 32\t\t\t%lu Hex: %lx\n", u32p, u32p);

	u64p = pearson_hash_64((uint8_t*)buff, strlen(buff),(uint64_t)0);
	printf("Pearson 64\t\t\t%llu Hex: %llx\n", u64p, u64p);

	
	pearson_hash_128(hashp,(uint8_t*)buff, strlen(buff));
	printf("Pearson 128:\t\t\tHex: %02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x\n", 
		hashp[15], hashp[14], hashp[13], hashp[12], hashp[11], hashp[10], hashp[9], hashp[8], hashp[7], hashp[6], hashp[5],
		hashp[4], hashp[3], hashp[2], hashp[1], hashp[0]);
#endif 

#ifdef hash_fnv
// FNV-1 
//	{ "a", (Fnv32_t)0x050c5d7e }
//	{ "b", (Fnv32_t)0x050c5d7d }
// FNV-1a 
//	{ "a", (Fnv32_t)0xe40c292cUL }
//	{ "b", (Fnv32_t)0xe70c2de5UL }


	puts("\n==FNV 1/1a===");
	printf("Actual Seed: %lu\n\n", FNV1_32_INIT);
	Fnv32_t res;
	Fnv32_t h1f = FNV1_32_INIT;
	res=fnv_32_buf(buff, strlen(buff), h1f);
	printf("FNV1 32\t\t\t%lu Hex: %lx\n", res, res);

	res = fnv_32a_buf((void *)buff, strlen(buff),h1f);
	printf("FNV1a 32\t\t%lu Hex: %lx\n", res, res);

// FNV1 64
//	{ "a", (Fnv64_t)0xaf63bd4c8601b7beULL },
//    { "b", (Fnv64_t) 0xaf63bd4c8601b7bdULL },

//FNV1a 64
//	{ "a", (Fnv64_t)0xaf63dc4c8601ec8cULL },
//	{ "b", (Fnv64_t)0xaf63df4c8601f1a5ULL },

	Fnv64_t res1;
	Fnv64_t h2f = (Fnv64_t)FNV1A_64_INIT;
	res1 =fnv_64_buf(buff, strlen(buff),h2f);
	printf("FNV1 64\t\t\t%llu Hex: %llx\n", res1, res1);

	res1 = fnv_64a_buf(buff, strlen(buff),h2f);
	printf("FNV1a 64\t\t%llu Hex: %llx\n", res1, res1);

	uint8_t  hash3[16];
	FNV128string(buff, hash3);
	printf("FVN 128:\t\t%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x\n", hash3[15], hash3[14], hash3[13],
		hash3[12], hash3[11], hash3[10], hash3[9], hash3[8], hash3[7], hash3[6], hash3[5], hash3[4], hash3[3], hash3[2], 
		hash3[1], hash3[0]);



#endif

#ifdef hash_halftime

	puts("\n==Half-time hash===");
	uint64 u64h;

	array<uint64_t, halftime_hash::kEntropyBytesNeeded / sizeof(uint64_t)> entropy;
	generate(entropy.begin(), entropy.end(), mt19937_64{});

	printf("%x %x %d\n", entropy.data()[0],entropy.data()[1], halftime_hash::kEntropyBytesNeeded);
	u64h=halftime_hash::HalftimeHashStyle64(entropy.data(), buff, strlen(buff));
	printf("Halftime 64Style\t\t%llu Hex: %llx\n", u64h, u64h);

	u64h = halftime_hash::HalftimeHashStyle128(entropy.data(), buff, strlen(buff));
	printf("Halftime 128Style\t\t%llu Hex: %llx\n", u64h, u64h);

	u64h = halftime_hash::HalftimeHashStyle256(entropy.data(), buff, strlen(buff));
	printf("Halftime 256Style\t\t%llu Hex: %llx\n", u64h, u64h);

	u64h = halftime_hash::HalftimeHashStyle512(entropy.data(), buff, strlen(buff));
	printf("Halftime 512Style\t\t%llu Hex: %llx\n", u64h, u64h);

#endif
#ifdef hash_highway
	puts("\n=== HighwayHash ===");
	uint64_t tkey[4] = { 0,0,0,0 };
	uint64_t u64hi = HighwayHash64((uint8_t *)buff, strlen(buff), tkey);

	printf("\nHighwayHash 64\t\t%llu Hex: %llx\n", swapVal(u64hi), swapVal(u64hi));

	uint64_t t_hash[2];
	HighwayHash128((uint8_t*)buff, strlen(buff), tkey,t_hash);

	printf("HighwayHash 128\t\t%llx %llx\n", swapVal (t_hash[0]), swapVal( t_hash[1]));

	uint64_t t_hash2[4];
	HighwayHash256((uint8_t*)buff, strlen(buff), tkey, t_hash2);

	printf("HighwayHash 256\t\t%llx %llx %llx %llx\n", swapVal(t_hash2[0]), swapVal(t_hash2[1]), swapVal(t_hash2[2]), swapVal(t_hash2[3]));
#endif

	

#ifdef hash_siphash
	puts("\n=== SipHash ===");

	unsigned char sipkey[16] = {0,0,0,0,0,0,0,0 ,0,0,0,0 ,0,0,0,0};
	//char hexdata[] = "0123456789ABCDEF";
	for (int j = 0; j < sizeof(hexdata) ; j++) {
		sscanf_s(hexdata + j , "%c", sipkey + j);
	}

	uint64_t u64sip = siphash(sipkey,(unsigned char *)buff, strlen(buff));

	printf("\nSipHash 64\t\t%llu Hex: %llx\n", (u64sip), (u64sip));

	u64sip = siphash13(sipkey, (uint8_t*)buff, strlen(buff));

	printf("SipHash13\t\t%llu Hex: %llx\n", (u64sip), (u64sip));

	uint32_t u32sip = halfsiphash(sipkey, (uint8_t*)buff, strlen(buff));

	printf("HalfSipHash\t\t%lu Hex: %lx\n", (u32sip), (u32sip));

	
#endif

#ifdef hash_clhash
	puts("\n===CLHash===");
	printf("Seed generated from 0x23a23cf5033c3c81, 0xb3816f6a2c68e530\n");
	uint64_t u64cl;
	void* random = get_random_key_for_clhash(UINT64_C(0x23a23cf5033c3c81), UINT64_C(0xb3816f6a2c68e530));
	u64cl = clhash(random, (char*)buff, strlen(buff));
	
	printf("\nClHash 64\t\t%llu Hex: %llx\n", u64cl, u64cl);

#endif
#ifdef hash_falkhash
	puts("\n===FalkHash===");
		

		
		__m128i	h128 = falkhash((void *)buff, strlen(buff), 0);
		
		
		uint64_t n[2];

		_mm_storeu_si128((__m128i*)n, h128);

		

		printf("\nFalkHash128\t\t%llx%llx\n", n[1], n[0]);

#endif	

#ifdef hash_wyhash
		puts("\n===wyhash===");

		uint64_t u64wy,secret=0;
	//	static const uint64_t _wyp[4] = { 0xa0761d6478bd642full, 0xe7037ed1a0b428dbull, 0x8ebc6af09c88c6e3ull, 0x589965cc75374cc3ull };
		static const uint64_t _wyp[4] = { 0xa0761d6478bd642full, 0xe7037ed1a0b428dbull, 0x8ebc6af09c88c6e3ull, 0x589965cc75374cc3ull };


		uint32_t u32wy;
		u32wy = wyhash32((void*)buff, strlen(buff), seed);

		printf("\nwyhash 32\t\t%lu Hex: %lx\n", u32wy, u32wy);

		printf("\nDefault secret: 0xa0761d6478bd642f, 0xe7037ed1a0b428db, 0x8ebc6af09c88c6e3, 0x589965cc75374cc3\n");
		u64wy = wyhash((void *)buff, strlen(buff),seed,_wyp);

		printf("\nwyhash 64\t\t%llu Hex: %llx\n", u64wy, u64wy);
#endif

#ifdef hash_fib
		puts("===Fibonacci===");
		uint16_t u16f;
		fib_hash16((void*)buff, strlen(buff), seed, &u16f);

		printf("\nFibonacci 16\t\t%u Hex: %04x\n", u16f, u16f);

		uint32_t u32f;
		fib_hash32((void*)buff, strlen(buff), seed, &u32f);

		printf("Fibonacci 32\t\t%lu Hex: %lx\n", u32f, u32f);

		uint64_t u64f;
		fib_hash64((void*)buff, strlen(buff), seed, &u64f);

		printf("Fibonacci 64\t\t%llu Hex: %llx\n", u64f, u64f);
#endif
#ifdef hash_lookup3
		{
			puts("===Bob Jenkins Lookup3 ===");
			uint32_t u32l = hashlittle((void*)buff, strlen(buff), seed);
			printf("Lookup 32\t\t\t%lu Hex: %lx\n", u32l, u32l);

			puts("\n===Bob Jenkins Lookup3 ===");
			u32l = jenkins_one_at_a_time_hash((uint8_t*)buff, strlen(buff));
			printf("One_at_a_time_hash 32\t\t%lu Hex: %lx\n", u32l, u32l);


			puts("\n===Spooky V2===");

			uint32 u32s;
			uint64 u64s, u64s2;
			u32s = SpookyHash::Hash32(buff, strlen(buff), (uint32_t)0);
			printf("SpookyV2 32\t\t\t%lu Hex: %lx\n", u32s, u32s);
			u32s = SpookyHash::Hash32(buff, strlen(buff), (uint32_t)seed);
			printf("SpookyV2 32+Seed\t\t%lu Hex: %lx\n", u32s, u32s);

			u64s = SpookyHash::Hash64(buff, strlen(buff), (uint64_t)0);
			printf("SpookyV2 64\t\t\t%llu Hex: %llx\n", u64s, u64s);

			u64s = SpookyHash::Hash64(buff, strlen(buff), (uint64_t)seed);
			printf("SpookyV2 64+Seed\t\t%llu Hex: %llx\n", u64s, u64s);

			u64s = 0; //seed
			u64s2 = 0;

			SpookyHash::Hash128(buff, strlen(buff), &u64s, &u64s2);

			printf("SpookyV2 128\t\t\t%llx%llx\n", swapVal(u64s), swapVal(u64s2));
			u64s = seed; //seed
			u64s2 = 0;
			SpookyHash::Hash128(buff, strlen(buff), &u64s, &u64s2);
			printf("SpookyV2 128+Seed\t\t%llx%llx\n", swapVal(u64s), swapVal(u64s2));
		}

#endif

}