/* implementation based on libtomcrypt */
#include <stdio.h>
#include <stdint.h>

#define LOAD32H(x, y) \
do { x = ((uint32_t)((y)[0] & 255)<<24) | \
         ((uint32_t)((y)[1] & 255)<<16) | \
         ((uint32_t)((y)[2] & 255)<< 8) | \
         ((uint32_t)((y)[3] & 255)); } while(0)

#define STORE32H(x, y) \
do { (y)[0] = (unsigned char)(((x)>>24)&255);\
     (y)[1] = (unsigned char)(((x)>>16)&255);\
     (y)[2] = (unsigned char)(((x)>> 8)&255);\
     (y)[3] = (unsigned char)((x)&255); } while(0)

#define LOAD64H(x, y) \
do { x = (((uint64_t)((y)[0] & 255))<<56) | \
         (((uint64_t)((y)[1] & 255))<<48) | \
         (((uint64_t)((y)[2] & 255))<<40) | \
         (((uint64_t)((y)[3] & 255))<<32) | \
         (((uint64_t)((y)[4] & 255))<<24) | \
         (((uint64_t)((y)[5] & 255))<<16) | \
         (((uint64_t)((y)[6] & 255))<< 8) | \
         (((uint64_t)((y)[7] & 255))); } while(0)

#define STORE64H(x, y) \
do { (y)[0] = (unsigned char)(((x)>>56)&255);\
     (y)[1] = (unsigned char)(((x)>>48)&255);\
     (y)[2] = (unsigned char)(((x)>>40)&255);\
     (y)[3] = (unsigned char)(((x)>>32)&255);\
     (y)[4] = (unsigned char)(((x)>>24)&255);\
     (y)[5] = (unsigned char)(((x)>>16)&255);\
     (y)[6] = (unsigned char)(((x)>> 8)&255);\
     (y)[7] = (unsigned char)((x)&255); } while(0)

struct sha1_state {
	uint64_t length;
	uint32_t state[5];
	uint8_t buf[64];
};

struct sha256_state {
	uint64_t length;
	uint32_t state[8];
	uint8_t buf[64];
};

struct sha512_state {
	uint64_t length;
	uint64_t state[8];
	uint8_t buf[128];
};

union hash_state {
	struct sha512_state sha512;
	struct sha256_state sha256;
	struct sha256_state sha224;
	struct sha1_state sha1;
};

struct crypto {
	union hash_state *md;
	void (*init)(union hash_state *);
	void (*process)(union hash_state *, uint8_t *, unsigned long);
	void (*done)(union hash_state *, uint8_t *);
	size_t bsiz;
	uint8_t *buf;
};

int crypto_check(struct crypto *, FILE *, const char *);
int crypto_print(struct crypto *, FILE *, const char *);

void sha1_init(union hash_state *);
void sha1_process(union hash_state *, uint8_t *, unsigned long);
void sha1_done(union hash_state *, uint8_t *);

void sha224_init(union hash_state *);
void sha224_done(union hash_state *, uint8_t *);

void sha256_init(union hash_state *);
void sha256_process(union hash_state *, uint8_t *, unsigned long);
void sha256_done(union hash_state *, uint8_t *);

void sha512_init(union hash_state *);
void sha512_process(union hash_state *, uint8_t *, unsigned long);
void sha512_done(union hash_state *, uint8_t *);

