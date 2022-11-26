#include <inttypes.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "umash.h"

#define BUFFSIZE 100
static struct umash_params my_params;

int
main(int argc, char **argv)
{
	struct umash_fp fprint;
const char *input = "the quick brown fox";

static const char my_secret[32] = "hello example.c";
	
	uint64_t hash;



	char buff[128];



	uint64_t seed = 42;

//	strcpy(buff, str);

	if (argc >= 2) {
		if (strlen(argv[1]) > BUFFSIZE - 1) return(0);
		strcpy(buff, argv[1]);
	}
	if (argc >= 3) seed = atoi(argv[2]);


	printf("String: %s\n", buff);
	printf("Seed: %d\n\n", seed);

	umash_params_derive(&my_params, 0, my_secret);


	fprint = umash_fprint(&my_params, seed, buff, strlen(str));
	printf("Fingerprint: %" PRIx64 ", %" PRIx64 "\n", fprint.hash[0], fprint.hash[1]);

	hash = umash_full(&my_params, seed, /*which=*/0, buff, strlen(str));
	printf("Hash 0: %" PRIx64 "\n", hash);

	hash = umash_full(&my_params, seed, /*which=*/1,buff, strlen(str));
	printf("Hash 1: %" PRIx64 "\n", hash);
	return 0;
}