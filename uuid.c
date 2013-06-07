#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#define UUID_BYTES 16
#define UUID_STR 37

typedef struct {
	uint8_t bytes[UUID_BYTES];
	char str[UUID_STR];
} UUID;

FILE *
open_rand() {
	return fopen("/dev/urandom", "r");
}

/* RFC4122
	4.4.  Algorithms for Creating a UUID from Truly Random or
	Pseudo-Random Numbers
	The version 4 UUID is meant for generating UUIDs from truly-random or
	pseudo-random numbers.
	The algorithm is as follows:
	o  Set the two most significant bits (bits 6 and 7) of the
	   clock_seq_hi_and_reserved to zero and one, respectively.
	o  Set the four most significant bits (bits 12 through 15) of the
	   time_hi_and_version field to the 4-bit version number from
	   Section 4.1.3.
	o  Set all the other bits to randomly (or pseudo-randomly) chosen
	   values.
*/

/*
	To minimize confusion about bit assignments within octets, the UUID
	record definition is defined only in terms of fields that are
	integral numbers of octets.  The fields are presented with the most
	significant one first.
	Field                  Data Type     Octet  Note
	                                     #
	time_low               unsigned 32   0-3    The low field of the
	                       bit integer          timestamp
	time_mid               unsigned 16   4-5    The middle field of the
	                       bit integer          timestamp
	time_hi_and_version    unsigned 16   6-7    The high field of the
	                       bit integer          timestamp multiplexed
	                                            with the version number
	clock_seq_hi_and_rese  unsigned 8    8      The high field of the
	rved                   bit integer          clock sequence
	                                            multiplexed with the
	                                            variant
	clock_seq_low          unsigned 8    9      The low field of the
	                       bit integer          clock sequence
	node                   unsigned 48   10-15  The spatially unique
	                       bit integer          node identifier
*/

bool
create_uuid4(UUID *u, FILE *f) {
	if (fread(u->bytes, 1, UUID_BYTES, f) != UUID_BYTES) {
		return false;
	}

	// Setting variant - Most significant nibble must be one of 8, 9, a, b
	u->bytes[8] |= 0x80;
	u->bytes[8] &= 0xBF;

	// Setting version - Most significant nibble must be 4
	u->bytes[6] |= 0x40;
	u->bytes[6] &= 0x4F;

	return ((UUID_STR - 1) == snprintf(u->str, UUID_STR,
		"%02x%02x%02x%02x-%02x%02x-%02x%02x-%02x%02x-%02x%02x%02x%02x%02x%02x",
		u->bytes[0], u->bytes[1], u->bytes[2], u->bytes[3], // time-low 
		u->bytes[4], u->bytes[5], // time-mid
		u->bytes[6], u->bytes[7], // time-high-and-version
		u->bytes[8], u->bytes[9], // clock-seq-and-reserved + clock-seq-low
		u->bytes[10], u->bytes[11], u->bytes[12], u->bytes[13],
		u->bytes[14], u->bytes[15])); // node
}

int
main() {
	FILE *f;
	f = open_rand();
	UUID u;
	if (create_uuid4(&u, f)) {
		printf("%s\n", u.str);
		return EXIT_SUCCESS;
	} else {
		fprintf(stderr, "Failed to create UUID.\n");
	}
	return EXIT_FAILURE;
}
