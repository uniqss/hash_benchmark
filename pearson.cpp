#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

#include "pearson.h"

// compile with 'LOW_MEM_FOOTPRINT' defined to make use of 256 byte look-up table only
// otherwise, a 16-bit look-up table is used which allows considerably faster hashing
// however, it needs to be generated by once calling pearson_hash_init() upfront
#define LOW_MEM_FOOTPRINT


// AES S-Box table -- allows for eventually supported hardware accelerated look-up
static const uint8_t t[256] ={
	0x63, 0x7c, 0x77, 0x7b, 0xf2, 0x6b, 0x6f, 0xc5, 0x30, 0x01, 0x67, 0x2b, 0xfe, 0xd7, 0xab, 0x76,
	0xca, 0x82, 0xc9, 0x7d, 0xfa, 0x59, 0x47, 0xf0, 0xad, 0xd4, 0xa2, 0xaf, 0x9c, 0xa4, 0x72, 0xc0,
	0xb7, 0xfd, 0x93, 0x26, 0x36, 0x3f, 0xf7, 0xcc, 0x34, 0xa5, 0xe5, 0xf1, 0x71, 0xd8, 0x31, 0x15,
	0x04, 0xc7, 0x23, 0xc3, 0x18, 0x96, 0x05, 0x9a, 0x07, 0x12, 0x80, 0xe2, 0xeb, 0x27, 0xb2, 0x75,
	0x09, 0x83, 0x2c, 0x1a, 0x1b, 0x6e, 0x5a, 0xa0, 0x52, 0x3b, 0xd6, 0xb3, 0x29, 0xe3, 0x2f, 0x84,
	0x53, 0xd1, 0x00, 0xed, 0x20, 0xfc, 0xb1, 0x5b, 0x6a, 0xcb, 0xbe, 0x39, 0x4a, 0x4c, 0x58, 0xcf,
	0xd0, 0xef, 0xaa, 0xfb, 0x43, 0x4d, 0x33, 0x85, 0x45, 0xf9, 0x02, 0x7f, 0x50, 0x3c, 0x9f, 0xa8,
	0x51, 0xa3, 0x40, 0x8f, 0x92, 0x9d, 0x38, 0xf5, 0xbc, 0xb6, 0xda, 0x21, 0x10, 0xff, 0xf3, 0xd2,
	0xcd, 0x0c, 0x13, 0xec, 0x5f, 0x97, 0x44, 0x17, 0xc4, 0xa7, 0x7e, 0x3d, 0x64, 0x5d, 0x19, 0x73,
	0x60, 0x81, 0x4f, 0xdc, 0x22, 0x2a, 0x90, 0x88, 0x46, 0xee, 0xb8, 0x14, 0xde, 0x5e, 0x0b, 0xdb,
	0xe0, 0x32, 0x3a, 0x0a, 0x49, 0x06, 0x24, 0x5c, 0xc2, 0xd3, 0xac, 0x62, 0x91, 0x95, 0xe4, 0x79,
	0xe7, 0xc8, 0x37, 0x6d, 0x8d, 0xd5, 0x4e, 0xa9, 0x6c, 0x56, 0xf4, 0xea, 0x65, 0x7a, 0xae, 0x08,
	0xba, 0x78, 0x25, 0x2e, 0x1c, 0xa6, 0xb4, 0xc6, 0xe8, 0xdd, 0x74, 0x1f, 0x4b, 0xbd, 0x8b, 0x8a,
	0x70, 0x3e, 0xb5, 0x66, 0x48, 0x03, 0xf6, 0x0e, 0x61, 0x35, 0x57, 0xb9, 0x86, 0xc1, 0x1d, 0x9e,
	0xe1, 0xf8, 0x98, 0x11, 0x69, 0xd9, 0x8e, 0x94, 0x9b, 0x1e, 0x87, 0xe9, 0xce, 0x55, 0x28, 0xdf,
	0x8c, 0xa1, 0x89, 0x0d, 0xbf, 0xe6, 0x42, 0x68, 0x41, 0x99, 0x2d, 0x0f, 0xb0, 0x54, 0xbb, 0x16 };



#ifndef LOW_MEM_FOOTPRINT
static uint16_t t16[65536];
#endif



// table as in original paper "Fast Hashing of Variable-Length Text Strings" by Peter K. Pearson
// as published in The Communications of the ACM  Vol.33, No.  6 (June 1990), pp. 677-680./
/*
static const uint8_t t[256] ={
	0x01, 0x57, 0x31, 0x0c, 0xb0, 0xb2, 0x66, 0xa6, 0x79, 0xc1, 0x06, 0x54, 0xf9, 0xe6, 0x2c, 0xa3,
	0x0e, 0xc5, 0xd5, 0xb5, 0xa1, 0x55, 0xda, 0x50, 0x40, 0xef, 0x18, 0xe2, 0xec, 0x8e, 0x26, 0xc8,
	0x6e, 0xb1, 0x68, 0x67, 0x8d, 0xfd, 0xff, 0x32, 0x4d, 0x65, 0x51, 0x12, 0x2d, 0x60, 0x1f, 0xde,
	0x19, 0x6b, 0xbe, 0x46, 0x56, 0xed, 0xf0, 0x22, 0x48, 0xf2, 0x14, 0xd6, 0xf4, 0xe3, 0x95, 0xeb,
	0x61, 0xea, 0x39, 0x16, 0x3c, 0xfa, 0x52, 0xaf, 0xd0, 0x05, 0x7f, 0xc7, 0x6f, 0x3e, 0x87, 0xf8,
	0xae, 0xa9, 0xd3, 0x3a, 0x42, 0x9a, 0x6a, 0xc3, 0xf5, 0xab, 0x11, 0xbb, 0xb6, 0xb3, 0x00, 0xf3,
	0x84, 0x38, 0x94, 0x4b, 0x80, 0x85, 0x9e, 0x64, 0x82, 0x7e, 0x5b, 0x0d, 0x99, 0xf6, 0xd8, 0xdb,
	0x77, 0x44, 0xdf, 0x4e, 0x53, 0x58, 0xc9, 0x63, 0x7a, 0x0b, 0x5c, 0x20, 0x88, 0x72, 0x34, 0x0a,
	0x8a, 0x1e, 0x30, 0xb7, 0x9c, 0x23, 0x3d, 0x1a, 0x8f, 0x4a, 0xfb, 0x5e, 0x81, 0xa2, 0x3f, 0x98,
	0xaa, 0x07, 0x73, 0xa7, 0xf1, 0xce, 0x03, 0x96, 0x37, 0x3b, 0x97, 0xdc, 0x5a, 0x35, 0x17, 0x83,
	0x7d, 0xad, 0x0f, 0xee, 0x4f, 0x5f, 0x59, 0x10, 0x69, 0x89, 0xe1, 0xe0, 0xd9, 0xa0, 0x25, 0x7b,
	0x76, 0x49, 0x02, 0x9d, 0x2e, 0x74, 0x09, 0x91, 0x86, 0xe4, 0xcf, 0xd4, 0xca, 0xd7, 0x45, 0xe5,
	0x1b, 0xbc, 0x43, 0x7c, 0xa8, 0xfc, 0x2a, 0x04, 0x1d, 0x6c, 0x15, 0xf7, 0x13, 0xcd, 0x27, 0xcb,
	0xe9, 0x28, 0xba, 0x93, 0xc6, 0xc0, 0x9b, 0x21, 0xa4, 0xbf, 0x62, 0xcc, 0xa5, 0xb4, 0x75, 0x4c,
	0x8c, 0x24, 0xd2, 0xac, 0x29, 0x36, 0x9f, 0x08, 0xb9, 0xe8, 0x71, 0xc4, 0xe7, 0x2f, 0x92, 0x78,
	0x33, 0x41, 0x1c, 0x90, 0xfe, 0xdd, 0x5d, 0xbd, 0xc2, 0x8b, 0x70, 0x2b, 0x47, 0x6d, 0xb8, 0xd1 };
*/

/*
// alternative table as used in RFC 3074 and NOT as in original paper
static const uint8_t t[256] ={
	0xfb, 0xaf, 0x77, 0xd7, 0x51, 0x0e, 0x4f, 0xbf, 0x67, 0x31, 0xb5, 0x8f, 0xba, 0x9d, 0x00, 0xe8,
	0x1f, 0x20, 0x37, 0x3c, 0x98, 0x3a, 0x11, 0xed, 0xae, 0x46, 0xa0, 0x90, 0xdc, 0x5a, 0x39, 0xdf,
	0x3b, 0x03, 0x12, 0x8c, 0x6f, 0xa6, 0xcb, 0xc4, 0x86, 0xf3, 0x7c, 0x5f, 0xde, 0xb3, 0xc5, 0x41,
	0xb4, 0x30, 0x24, 0x0f, 0x6b, 0x2e, 0xe9, 0x82, 0xa5, 0x1e, 0x7b, 0xa1, 0xd1, 0x17, 0x61, 0x10,
	0x28, 0x5b, 0xdb, 0x3d, 0x64, 0x0a, 0xd2, 0x6d, 0xfa, 0x7f, 0x16, 0x8a, 0x1d, 0x6c, 0xf4, 0x43,
	0xcf, 0x09, 0xb2, 0xcc, 0x4a, 0x62, 0x7e, 0xf9, 0xa7, 0x74, 0x22, 0x4d, 0xc1, 0xc8, 0x79, 0x05,
	0x14, 0x71, 0x47, 0x23, 0x80, 0x0d, 0xb6, 0x5e, 0x19, 0xe2, 0xe3, 0xc7, 0x4b, 0x1b, 0x29, 0xf5,
	0xe6, 0xe0, 0x2b, 0xe1, 0xb1, 0x1a, 0x9b, 0x96, 0xd4, 0x8e, 0xda, 0x73, 0xf1, 0x49, 0x58, 0x69,
	0x27, 0x72, 0x3e, 0xff, 0xc0, 0xc9, 0x91, 0xd6, 0xa8, 0x9e, 0xdd, 0x94, 0x9a, 0x7a, 0x0c, 0x54,
	0x52, 0xa3, 0x2c, 0x8b, 0xe4, 0xec, 0xcd, 0xf2, 0xd9, 0x0b, 0xbb, 0x92, 0x9f, 0x40, 0x56, 0xef,
	0xc3, 0x2a, 0x6a, 0xc6, 0x76, 0x70, 0xb8, 0xac, 0x57, 0x02, 0xad, 0x75, 0xb0, 0xe5, 0xf7, 0xfd,
	0x89, 0xb9, 0x63, 0xa4, 0x66, 0x93, 0x2d, 0x42, 0xe7, 0x34, 0x8d, 0xd3, 0xc2, 0xce, 0xf6, 0xee,
	0x38, 0x6e, 0x4e, 0xf8, 0x3f, 0xf0, 0xbd, 0x5d, 0x5c, 0x33, 0x35, 0xb7, 0x13, 0xab, 0x48, 0x32,
	0x21, 0x68, 0x65, 0x45, 0x08, 0xfc, 0x53, 0x78, 0x4c, 0x87, 0x55, 0x36, 0xca, 0x7d, 0xbc, 0xd5,
	0x60, 0xeb, 0x88, 0xd0, 0xa2, 0x81, 0xbe, 0x84, 0x9c, 0x26, 0x2f, 0x01, 0x07, 0xfe, 0x18, 0x04,
	0xd8, 0x83, 0x59, 0x15, 0x1c, 0x85, 0x25, 0x99, 0x95, 0x50, 0xaa, 0x44, 0x06, 0xa9, 0xea, 0x97 };
*/

#define ROR64(x,r) (((x)>>(r))|((x)<<(64-(r))))
#define ROR32(x,r) (((x)>>(r))|((x)<<(32-(r))))
#define ROR16(x,r) (((x)>>(r))|((x)<<(16-(r))))


#if defined (__SSSE3__) //  SSSE3 (& AES-NI) -----------------------------------------------


void pearson_hash_256 (uint8_t *out, const uint8_t *in, size_t len) {

        size_t i;

        uint8_t upper[8] = { 0x0F, 0x0E, 0x0D, 0x0C, 0x0B, 0x0A, 0x09, 0x08 };
        uint8_t lower[8] = { 0x07, 0x06, 0x05, 0x04, 0x03, 0x02, 0x01, 0x00 };

        uint64_t upper_hash_mask = *(uint64_t*)&upper;
        uint64_t lower_hash_mask = *(uint64_t*)&lower;

        __m128i tmp = _mm_set1_epi8(0x10);

        __m128i hash_mask = _mm_set_epi64 ((__m64)lower_hash_mask, (__m64)upper_hash_mask);
        __m128i high_hash_mask = _mm_xor_si128 (tmp, hash_mask);
        __m128i hash= _mm_setzero_si128();
        __m128i high_hash= _mm_setzero_si128(); hash;

       // table lookup preparation
#if defined (__AES__) // AES-NI -------------
        __m128i ZERO = _mm_setzero_si128();
        __m128i ISOLATE_SBOX_MASK = _mm_set_epi32(0x0306090C, 0x0F020508, 0x0B0E0104, 0x070A0D00);
#else // ------------------------------------
        __m128i const p16  = _mm_set1_epi8 (0x10);
        __m128i lut_result  = _mm_xor_si128 (lut_result, lut_result);
        __m128i high_lut_result  = _mm_xor_si128 (high_lut_result, high_lut_result);
        __m128i selected_entries;
        __m128i high_selected_entries;
        __m128i table_line;
#endif // -----------------------------------

        for (i = 0; i < len; i++) {
                // broadcast the character, xor into hash, make them different permutations
                __m128i cc = _mm_set1_epi8 (in[i]);
                hash = _mm_xor_si128 (hash, cc);
                high_hash = _mm_xor_si128 (high_hash, cc);
                hash = _mm_xor_si128 (hash, hash_mask);
                high_hash = _mm_xor_si128 (high_hash, high_hash_mask);

                // table lookup
#if defined (__AES__) // AES-NI -------------
                hash = _mm_shuffle_epi8(hash, ISOLATE_SBOX_MASK);           // re-order along AES round
                high_hash = _mm_shuffle_epi8(high_hash, ISOLATE_SBOX_MASK); // re-order along AES round
                hash = _mm_aesenclast_si128(hash, ZERO);
                high_hash = _mm_aesenclast_si128(high_hash, ZERO);
#else // ------------------------------------
		size_t j;
		__m128i lut_index = hash;
                __m128i high_lut_index = high_hash;
		lut_result = _mm_xor_si128 (lut_result, lut_result);
		high_lut_result = _mm_xor_si128 (lut_result, lut_result);
		for (j = 0; j < 256; j += 16) {
			table_line = _mm_load_si128 ((__m128i *)&t[j]);
			selected_entries = _mm_min_epu8 (lut_index, p16);
			selected_entries = _mm_cmpeq_epi8 (selected_entries, p16);
			selected_entries = _mm_or_si128 (selected_entries, lut_index);
			selected_entries = _mm_shuffle_epi8 (table_line, selected_entries);
			high_selected_entries = _mm_min_epu8 (high_lut_index, p16);
			high_selected_entries = _mm_cmpeq_epi8 (high_selected_entries, p16);
			high_selected_entries = _mm_or_si128 (high_selected_entries, high_lut_index);
			high_selected_entries = _mm_shuffle_epi8 (table_line, high_selected_entries);
			lut_result  = _mm_or_si128 (lut_result, selected_entries);
			lut_index = _mm_sub_epi8 (lut_index, p16);
			high_lut_result  = _mm_or_si128 (high_lut_result, high_selected_entries);
			high_lut_index = _mm_sub_epi8 (high_lut_index, p16);
		}
		hash = lut_result;
		high_hash = high_lut_result;
#endif // -----------------------------------
        }
        // store output
        _mm_store_si128 ((__m128i*)out , high_hash);
        _mm_store_si128 ((__m128i*)&out[16] , hash);
}


void pearson_hash_128 (uint8_t *out, const uint8_t *in, size_t len) {

        size_t i;

        uint8_t upper[8] = { 0x0F, 0x0E, 0x0D, 0x0C, 0x0B, 0x0A, 0x09, 0x08 };
        uint8_t lower[8] = { 0x07, 0x06, 0x05, 0x04, 0x03, 0x02, 0x01, 0x00 };

        uint64_t upper_hash_mask = *(uint64_t*)&upper;
        uint64_t lower_hash_mask = *(uint64_t*)&lower;

        __m128i hash_mask = _mm_set_epi64 ((__m64)lower_hash_mask, (__m64)upper_hash_mask);
        __m128i hash = _mm_setzero_si128();

       // table lookup preparation
#if defined (__AES__) // AES-NI -------------
        __m128i ZERO = _mm_setzero_si128();
        __m128i ISOLATE_SBOX_MASK = _mm_set_epi32(0x0306090C, 0x0F020508, 0x0B0E0104, 0x070A0D00);
#else // ------------------------------------
	__m128i const p16  = _mm_set1_epi8 (0x10);
	__m128i lut_result  = _mm_xor_si128 (lut_result, lut_result);
	__m128i selected_entries;
	__m128i table_line;
#endif // -----------------------------------

        for (i = 0; i < len; i++) {
                // broadcast the character, xor into hash, make them different permutations
                __m128i cc = _mm_set1_epi8 (in[i]);
                hash = _mm_xor_si128 (hash, cc);
                hash = _mm_xor_si128 (hash, hash_mask);

                // table lookup
#if defined (__AES__) // AES-NI -------------
                hash = _mm_shuffle_epi8(hash, ISOLATE_SBOX_MASK); // re-order along AES round
                hash = _mm_aesenclast_si128(hash, ZERO);
#else // ------------------------------------
		size_t j;
		__m128i lut_index = hash;
		lut_result = _mm_xor_si128 (lut_result, lut_result);
		for (j = 0; j < 256; j += 16) {
			table_line = _mm_load_si128 ((__m128i *)&t[j]);
			selected_entries = _mm_min_epu8 (lut_index, p16);
			selected_entries = _mm_cmpeq_epi8 (selected_entries, p16);
			selected_entries = _mm_or_si128 (selected_entries, lut_index);
			selected_entries = _mm_shuffle_epi8 (table_line, selected_entries);
			lut_result  = _mm_or_si128 (lut_result, selected_entries);
			lut_index = _mm_sub_epi8 (lut_index, p16);
		}
		hash = lut_result;
#endif // -----------------------------------
        }
        // store output
        _mm_store_si128 ((__m128i*)out , hash);
}


uint64_t pearson_hash_64 (const uint8_t *in, size_t len, uint64_t hash_in) {

        size_t i;

        __m128i hash_mask = _mm_cvtsi64_si128(0x0706050403020100);

        __m128i hash = _mm_cvtsi64_si128 (hash_in);

	// table lookup preparation
#if defined (__AES__) // AES-NI -------------
        __m128i ZERO = _mm_setzero_si128();
        __m128i ISOLATE_SBOX_MASK = _mm_set_epi32(0x0306090C, 0x0F020508, 0x0B0E0104, 0x070A0D00);
#else // ------------------------------------
	__m128i const p16  = _mm_set1_epi8 (0x10);
	__m128i lut_result  = _mm_xor_si128 (lut_result, lut_result);
	__m128i selected_entries;
	__m128i table_line;
#endif // -----------------------------------

        for (i = 0; i < len; i++) {
                // broadcast the character, xor into hash, make them different permutations
                __m128i cc = _mm_set1_epi8 (in[i]);
                hash = _mm_xor_si128 (hash, cc);
                hash = _mm_xor_si128 (hash, hash_mask);

                // table lookup
#if defined (__AES__) // AES-NI -------------
                hash = _mm_shuffle_epi8(hash, ISOLATE_SBOX_MASK); // re-order along AES round
                hash = _mm_aesenclast_si128(hash, ZERO);
#else // ------------------------------------
		size_t j;
		__m128i lut_index = hash;
		lut_result = _mm_xor_si128 (lut_result, lut_result);
		for (j = 0; j < 256; j += 16) {
			table_line = _mm_load_si128 ((__m128i *)&t[j]);
			selected_entries = _mm_min_epu8 (lut_index, p16);
			selected_entries = _mm_cmpeq_epi8 (selected_entries, p16);
			selected_entries = _mm_or_si128 (selected_entries, lut_index);
			selected_entries = _mm_shuffle_epi8 (table_line, selected_entries);
			lut_result  = _mm_or_si128 (lut_result, selected_entries);
			lut_index = _mm_sub_epi8 (lut_index, p16);
		}
		hash = lut_result;
#endif // -----------------------------------
        }
        // store output
        return _mm_cvtsi128_si64 (hash);
}


#else // plain C --------------------------------------------------------------------------


void pearson_hash_256 (uint8_t *out, const uint8_t *in, size_t len) {

	size_t i;
	/* initial values -  astonishingly, assembling using SHIFTs and ORs (in register)
         * works faster on well pipelined CPUs than loading the 64-bit value from memory.
         * however, there is one advantage to loading from memory: as we also store back to
         * memory at the end, we do not need to care about endianess! */
	uint8_t upper[8] = { 0x0F, 0x0E, 0x0D, 0x0C, 0x0B, 0x0A, 0x09, 0x08 };
	uint8_t lower[8] = { 0x07, 0x06, 0x05, 0x04, 0x03, 0x02, 0x01, 0x00 };

	uint64_t upper_hash_mask = *(uint64_t*)&upper;
	uint64_t lower_hash_mask = *(uint64_t*)&lower;
	uint64_t high_upper_hash_mask = upper_hash_mask + 0x1010101010101010;
	uint64_t high_lower_hash_mask = lower_hash_mask + 0x1010101010101010;

	uint64_t upper_hash = 0;
	uint64_t lower_hash = 0;
	uint64_t high_upper_hash = 0;
	uint64_t high_lower_hash = 0;

	for (i = 0; i < len; i++) {
		// broadcast the character, xor into hash, make them different permutations
		uint64_t c = (uint8_t)in[i];
		c |= c <<  8;
		c |= c << 16;
		c |= c << 32;
		upper_hash ^= c ^ upper_hash_mask;
		lower_hash ^= c ^ lower_hash_mask;
		high_upper_hash ^= c ^ high_upper_hash_mask;
		high_lower_hash ^= c ^ high_lower_hash_mask;

		// table lookup
		uint64_t h = 0;
#ifdef LOW_MEM_FOOTPRINT // 256 byte look-up table ----------
		uint8_t x;
		x = upper_hash; x = t[x]; upper_hash >>= 8; h  = x; h = ROR64 (h, 8);
		x = upper_hash; x = t[x]; upper_hash >>= 8; h |= x; h = ROR64 (h, 8);
		x = upper_hash; x = t[x]; upper_hash >>= 8; h |= x; h = ROR64 (h, 8);
		x = upper_hash; x = t[x]; upper_hash >>= 8; h |= x; h = ROR64 (h, 8);
		x = upper_hash; x = t[x]; upper_hash >>= 8; h |= x; h = ROR64 (h, 8);
		x = upper_hash; x = t[x]; upper_hash >>= 8; h |= x; h = ROR64 (h, 8);
		x = upper_hash; x = t[x]; upper_hash >>= 8; h |= x; h = ROR64 (h, 8);
		x = upper_hash; x = t[x];                 ; h |= x; h = ROR64 (h, 8);
		upper_hash = h;

		h = 0;
		x = lower_hash; x = t[x]; lower_hash >>= 8; h  = x; h = ROR64 (h, 8);
		x = lower_hash; x = t[x]; lower_hash >>= 8; h |= x; h = ROR64 (h, 8);
		x = lower_hash; x = t[x]; lower_hash >>= 8; h |= x; h = ROR64 (h, 8);
		x = lower_hash; x = t[x]; lower_hash >>= 8; h |= x; h = ROR64 (h, 8);
		x = lower_hash; x = t[x]; lower_hash >>= 8; h |= x; h = ROR64 (h, 8);
		x = lower_hash; x = t[x]; lower_hash >>= 8; h |= x; h = ROR64 (h, 8);
		x = lower_hash; x = t[x]; lower_hash >>= 8; h |= x; h = ROR64 (h, 8);
		x = lower_hash; x = t[x];                 ; h |= x; h = ROR64 (h, 8);
		lower_hash = h;

		h = 0;
		x = high_upper_hash; x = t[x]; high_upper_hash >>= 8; h  = x; h = ROR64 (h, 8);
		x = high_upper_hash; x = t[x]; high_upper_hash >>= 8; h |= x; h = ROR64 (h, 8);
		x = high_upper_hash; x = t[x]; high_upper_hash >>= 8; h |= x; h = ROR64 (h, 8);
		x = high_upper_hash; x = t[x]; high_upper_hash >>= 8; h |= x; h = ROR64 (h, 8);
		x = high_upper_hash; x = t[x]; high_upper_hash >>= 8; h |= x; h = ROR64 (h, 8);
		x = high_upper_hash; x = t[x]; high_upper_hash >>= 8; h |= x; h = ROR64 (h, 8);
		x = high_upper_hash; x = t[x]; high_upper_hash >>= 8; h |= x; h = ROR64 (h, 8);
		x = high_upper_hash; x = t[x];                      ; h |= x; h = ROR64 (h, 8);
		high_upper_hash = h;

		h = 0;
		x = high_lower_hash; x = t[x]; high_lower_hash >>= 8; h  = x; h = ROR64 (h, 8);
		x = high_lower_hash; x = t[x]; high_lower_hash >>= 8; h |= x; h = ROR64 (h, 8);
		x = high_lower_hash; x = t[x]; high_lower_hash >>= 8; h |= x; h = ROR64 (h, 8);
		x = high_lower_hash; x = t[x]; high_lower_hash >>= 8; h |= x; h = ROR64 (h, 8);
		x = high_lower_hash; x = t[x]; high_lower_hash >>= 8; h |= x; h = ROR64 (h, 8);
		x = high_lower_hash; x = t[x]; high_lower_hash >>= 8; h |= x; h = ROR64 (h, 8);
		x = high_lower_hash; x = t[x]; high_lower_hash >>= 8; h |= x; h = ROR64 (h, 8);
		x = high_lower_hash; x = t[x];                      ; h |= x; h = ROR64 (h, 8);
		high_lower_hash = h;
#else // 16-bit look-up table -------------------------------
		uint16_t x;
		x = upper_hash; x = t16[x]; upper_hash >>= 16; h  = x; h = ROR64 (h, 16);
		x = upper_hash; x = t16[x]; upper_hash >>= 16; h |= x; h = ROR64 (h, 16);
		x = upper_hash; x = t16[x]; upper_hash >>= 16; h |= x; h = ROR64 (h, 16);
		x = upper_hash; x = t16[x];                  ; h |= x; h = ROR64 (h, 16);
 		upper_hash = h;

		h = 0;
		x = lower_hash; x = t16[x]; lower_hash >>= 16; h  = x; h = ROR64 (h, 16);
		x = lower_hash; x = t16[x]; lower_hash >>= 16; h |= x; h = ROR64 (h, 16);
		x = lower_hash; x = t16[x]; lower_hash >>= 16; h |= x; h = ROR64 (h, 16);
		x = lower_hash; x = t16[x];                  ; h |= x; h = ROR64 (h, 16);
		lower_hash = h;

		h = 0;
		x = high_upper_hash; x = t16[x]; high_upper_hash >>= 16; h  = x; h = ROR64 (h, 16);
		x = high_upper_hash; x = t16[x]; high_upper_hash >>= 16; h |= x; h = ROR64 (h, 16);
		x = high_upper_hash; x = t16[x]; high_upper_hash >>= 16; h |= x; h = ROR64 (h, 16);
		x = high_upper_hash; x = t16[x];                       ; h |= x; h = ROR64 (h, 16);
 		high_upper_hash = h;

		h = 0;
		x = high_lower_hash; x = t16[x]; high_lower_hash >>= 16; h  = x; h = ROR64 (h, 16);
		x = high_lower_hash; x = t16[x]; high_lower_hash >>= 16; h |= x; h = ROR64 (h, 16);
		x = high_lower_hash; x = t16[x]; high_lower_hash >>= 16; h |= x; h = ROR64 (h, 16);
		x = high_lower_hash; x = t16[x];                       ; h |= x; h = ROR64 (h, 16);
		high_lower_hash = h;
#endif // LOW_MEM_FOOTPRINT ---------------------------------
	}
	// store output
	uint64_t *o;
	o = (uint64_t*)&out[0];
	*o = high_upper_hash;
	o = (uint64_t*)&out[8];
	*o = high_lower_hash;
	o = (uint64_t*)&out[16];
	*o = upper_hash;
	o = (uint64_t*)&out[24];
	*o = lower_hash;
}


void pearson_hash_128 (uint8_t *out, const uint8_t *in, size_t len) {

	size_t i;
	/* initial values -  astonishingly, assembling using SHIFTs and ORs (in register)
         * works faster on well pipelined CPUs than loading the 64-bit value from memory.
         * however, there is one advantage to loading from memory: as we also store back to
         * memory at the end, we do not need to care about endianess! */
	uint8_t upper[8] = { 0x0F, 0x0E, 0x0D, 0x0C, 0x0B, 0x0A, 0x09, 0x08 };
	uint8_t lower[8] = { 0x07, 0x06, 0x05, 0x04, 0x03, 0x02, 0x01, 0x00 };

	uint64_t upper_hash_mask = *(uint64_t*)&upper;
	uint64_t lower_hash_mask = *(uint64_t*)&lower;

	uint64_t upper_hash = 0;
	uint64_t lower_hash = 0;

	for (i = 0; i < len; i++) {
		// broadcast the character, xor into hash, make them different permutations
		uint64_t c = (uint8_t)in[i];
		c |= c <<  8;
		c |= c << 16;
		c |= c << 32;
		upper_hash ^= c ^ upper_hash_mask;
		lower_hash ^= c ^ lower_hash_mask;

		// table lookup
		uint64_t h = 0;
#ifdef LOW_MEM_FOOTPRINT // 256 byte look-up table ----------
		uint8_t x;
		x = upper_hash; x = t[x]; upper_hash >>= 8; h  = x; h = ROR64 (h, 8);
		x = upper_hash; x = t[x]; upper_hash >>= 8; h |= x; h = ROR64 (h, 8);
		x = upper_hash; x = t[x]; upper_hash >>= 8; h |= x; h = ROR64 (h, 8);
		x = upper_hash; x = t[x]; upper_hash >>= 8; h |= x; h = ROR64 (h, 8);
		x = upper_hash; x = t[x]; upper_hash >>= 8; h |= x; h = ROR64 (h, 8);
		x = upper_hash; x = t[x]; upper_hash >>= 8; h |= x; h = ROR64 (h, 8);
		x = upper_hash; x = t[x]; upper_hash >>= 8; h |= x; h = ROR64 (h, 8);
		x = upper_hash; x = t[x];                 ; h |= x; h = ROR64 (h, 8);
		upper_hash = h;

		h = 0;
		x = lower_hash; x = t[x]; lower_hash >>= 8; h  = x; h = ROR64 (h, 8);
		x = lower_hash; x = t[x]; lower_hash >>= 8; h |= x; h = ROR64 (h, 8);
		x = lower_hash; x = t[x]; lower_hash >>= 8; h |= x; h = ROR64 (h, 8);
		x = lower_hash; x = t[x]; lower_hash >>= 8; h |= x; h = ROR64 (h, 8);
		x = lower_hash; x = t[x]; lower_hash >>= 8; h |= x; h = ROR64 (h, 8);
		x = lower_hash; x = t[x]; lower_hash >>= 8; h |= x; h = ROR64 (h, 8);
		x = lower_hash; x = t[x]; lower_hash >>= 8; h |= x; h = ROR64 (h, 8);
		x = lower_hash; x = t[x];                 ; h |= x; h = ROR64 (h, 8);
		lower_hash= h;
#else // 16-bit look-up table -------------------------------
		uint16_t x;
		x = upper_hash; x = t16[x]; upper_hash >>= 16; h  = x; h = ROR64 (h,16);
		x = upper_hash; x = t16[x]; upper_hash >>= 16; h |= x; h = ROR64 (h,16);
		x = upper_hash; x = t16[x]; upper_hash >>= 16; h |= x; h = ROR64 (h,16);
		x = upper_hash; x = t16[x];                  ; h |= x; h = ROR64 (h,16);
 		upper_hash = h;

		h = 0;
		x = lower_hash; x = t16[x]; lower_hash >>= 16; h  = x; h = ROR64 (h, 16);
		x = lower_hash; x = t16[x]; lower_hash >>= 16; h |= x; h = ROR64 (h, 16);
		x = lower_hash; x = t16[x]; lower_hash >>= 16; h |= x; h = ROR64 (h, 16);
		x = lower_hash; x = t16[x];                  ; h |= x; h = ROR64 (h, 16);
		lower_hash = h;
#endif // LOW_MEM_FOOTPRINT ---------------------------------
	}
	// store output
	uint64_t *o;
	o = (uint64_t*)&out[0];
	*o = upper_hash;
	o = (uint64_t*)&out[8];
	*o = lower_hash;
}


// 64-bit hash: the return value has to be interpreted as uint64_t and
// follows machine-specific endianess in memory
uint64_t pearson_hash_64 (const uint8_t *in, size_t len, uint64_t hash_in) {

        size_t i;
        uint64_t hash_mask = 0x0706050403020100;
	uint64_t hash = hash_in;

        for (i = 0; i < len; i++) {
                // broadcast the character, xor into hash, make them different permutations
                uint64_t c = (uint8_t)in[i];
                c |= c <<  8;
                c |= c << 16;
                c |= c << 32;
                hash ^= c ^ hash_mask;
                // table lookup
#ifdef LOW_MEM_FOOTPRINT // 256 byte look-up table ----------
// Version 1
/*
                uint64_t h = 0;
                uint8_t x;
                x = hash; x = t[x]; hash >>= 8; h  = x; h = ROR64 (h, 8);
                x = hash; x = t[x]; hash >>= 8; h |= x; h = ROR64 (h, 8);
                x = hash; x = t[x]; hash >>= 8; h |= x; h = ROR64 (h, 8);
                x = hash; x = t[x]; hash >>= 8; h |= x; h = ROR64 (h, 8);
                x = hash; x = t[x]; hash >>= 8; h |= x; h = ROR64 (h, 8);
                x = hash; x = t[x]; hash >>= 8; h |= x; h = ROR64 (h, 8);
                x = hash; x = t[x]; hash >>= 8; h |= x; h = ROR64 (h, 8);
                x = hash; x = t[x];           ; h |= x; h = ROR64 (h, 8);
                hash = h;
*/
// Version 2
/*
                uint32_t h1 = 0;
                uint32_t h2 = 0;
		uint32_t hash1 = hash;
		uint32_t hash2 = (hash >> 32);

                uint8_t x = 0;
		uint8_t y = 0;

                x = hash1; x = t[x]; hash1 >>= 8; h1  = x; h1 = ROR32 (h1, 8);
                x = hash1; x = t[x]; hash1 >>= 8; h1 |= x; h1 = ROR32 (h1, 8);
                x = hash1; x = t[x]; hash1 >>= 8; h1 |= x; h1 = ROR32 (h1, 8);
                x = hash1; x = t[x];            ; h1 |= x; h1 = ROR32 (h1, 8);
                hash1 = h1;

                y = hash2; y = t[y]; hash2 >>= 8; h2  = y; h2 = ROR32 (h2, 8);
                y = hash2; y = t[y]; hash2 >>= 8; h2 |= y; h2 = ROR32 (h2, 8);
                y = hash2; y = t[y]; hash2 >>= 8; h2 |= y; h2 = ROR32 (h2, 8);
                y = hash2; y = t[y];            ; h2 |= y; h2 = ROR32 (h2, 8);
                hash2 = h2;

                hash = h1 | ((uint64_t)h2 << 32);
*/
// Version 3

                uint16_t h1, h2, h3, h4;
		uint16_t hash1 = hash;
		uint16_t hash2 = (hash >> 16);
		uint16_t hash3 = (hash >> 32);
		uint16_t hash4 = (hash >> 48);

                uint8_t x = 0;
		uint8_t y = 0;
		uint8_t z = 0;
		uint8_t a = 0;

                x = hash1; x = t[x]; hash1 >>= 8; h1  = x; h1 = ROR16 (h1, 8);
                x = hash1; x = t[x];            ; h1 |= x; h1 = ROR16 (h1, 8);
                hash1 = h1;

                y = hash2; y = t[y]; hash2 >>= 8; h2  = y; h2 = ROR16 (h2, 8);
                y = hash2; y = t[y];            ; h2 |= y; h2 = ROR16 (h2, 8);
                hash2 = h2;

                z = hash3; z = t[z]; hash3 >>= 8; h3  = z; h3 = ROR16 (h3, 8);
                z = hash3; z = t[z];            ; h3 |= z; h3 = ROR16 (h3, 8);
                hash3 = h3;

                a = hash4; a = t[a]; hash4 >>= 8; h4  = a; h4 = ROR16 (h4, 8);
                a = hash4; a = t[a];            ; h4 |= a; h4 = ROR16 (h4, 8);
                hash4 = h4;

                hash = (uint64_t)h1 | ((uint64_t)h2 << 16) | ((uint64_t)h3 << 32)| ((uint64_t)h4 << 48);

#else // 16-bit look-up table -------------------------------
// Version 1

		uint64_t h = 0;
                h   = (t16[(uint16_t)(hash >> 16)] << 16) + t16[(uint16_t)hash];
                h <<= 32;
                h |= (uint32_t)((t16[(uint16_t)(hash >> 48)] << 16)) + t16[(uint16_t)(hash >> 32)];
                hash = ROR64 (h, 32);

// Version 2
/*
                uint64_t h = 0;
                uint16_t x;
                x = hash; x = t16[x]; hash >>= 16; h  = x; h = ROR64 (h, 16);
                x = hash; x = t16[x]; hash >>= 16; h |= x; h = ROR64 (h, 16);
                x = hash; x = t16[x]; hash >>= 16; h |= x; h = ROR64 (h, 16);
                x = hash; x = t16[x];            ; h |= x; h = ROR64 (h, 16);
		hash = h;
*/
#endif // LOW_MEM_FOOTPRINT ---------------------------------
        }
        // output
        return hash;
}


#endif // SSE, plain C --------------------------------------------------------------------


// 32-bit hash: the return value has to be interpreted as uint32_t and
// follows machine-specific endianess in memory
uint32_t pearson_hash_32 (const uint8_t *in, size_t len, uint32_t hash) {

	size_t i;
	uint32_t hash_mask = 0x03020100;

	for (i = 0; i < len; i++) {
		// broadcast the character, xor into hash, make them different permutations
		uint32_t c = (uint8_t)in[i];
		c |= c <<  8;
		c |= c << 16;
		hash ^= c ^ hash_mask;

		// table lookup
#ifdef LOW_MEM_FOOTPRINT // 256 byte look-up table ----------
		uint32_t h = 0;
		uint8_t x;
		x = hash; x = t[x]; hash >>= 8; h  = x; h = ROR32 (h, 8);
		x = hash; x = t[x]; hash >>= 8; h |= x; h = ROR32 (h, 8);
		x = hash; x = t[x]; hash >>= 8; h |= x; h = ROR32 (h, 8);
		x = hash; x = t[x];           ; h |= x; h = ROR32 (h, 8);
		hash = h;
#else // 16-bit look-up table -------------------------------
		hash = (t16[hash >> 16] << 16) + t16[(uint16_t)hash];
	
#endif // LOW_MEM_FOOTPRINT ---------------------------------
	}
	// output
	return hash;
}


void pearson_hash_init () {

#ifndef LOW_MEM_FOOTPRINT // 16-bit look-up table -----------
	size_t i;

	for (i = 0; i < 65536; i++)
		t16[i] = (t[i >> 8] << 8) + t[(uint8_t)i];
#endif // LOW_MEM_FOOTPRINT ---------------------------------
}
