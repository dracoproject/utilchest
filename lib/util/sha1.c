/* implementation based on libtomcrypt */
#include <stdint.h>
#include <string.h>

#include "crypto.h"

#define REV(a,b,c,d,e,t) t=e;e=d;d=c;c=b;b=a;a=t
#define F0(a,b,c) (c ^ (a & (b ^ c)))
#define F1(a,b,c) (a ^ b ^ c)
#define F2(a,b,c) ((a & b) | (c & (a | b)))
#define F3(a,b,c) (a ^ b ^ c)
#define FF0(a,b,c,d,e,i) e = (rol(a, 5) + F0(b,c,d) + e + W[i] + 0x5a827999UL); b = rol(b, 30);
#define FF1(a,b,c,d,e,i) e = (rol(a, 5) + F1(b,c,d) + e + W[i] + 0x6ed9eba1UL); b = rol(b, 30);
#define FF2(a,b,c,d,e,i) e = (rol(a, 5) + F2(b,c,d) + e + W[i] + 0x8f1bbcdcUL); b = rol(b, 30);
#define FF3(a,b,c,d,e,i) e = (rol(a, 5) + F3(b,c,d) + e + W[i] + 0xca62c1d6UL); b = rol(b, 30);

static uint32_t
rol(uint32_t n, int k)
{
	return ((n << k) | (n >> (32 - k)));
}

static void
sha1_compress(union hash_state *md, uint8_t *buf)
{
	uint32_t W[80], a, b, c, d, e, t;
	int i;

	for (i = 0; i < 16; i++)
		LOAD32H(W[i], buf + (4 * i));

	for (i = 16; i < 80; i++)
		W[i] = rol(W[i - 3] ^ W[i - 8] ^ W[i - 14] ^ W[i - 16], 1);

	a = md->sha1.state[0];
	b = md->sha1.state[1];
	c = md->sha1.state[2];
	d = md->sha1.state[3];
	e = md->sha1.state[4];

	for (i = 0; i < 20; i++) {
		FF0(a,b,c,d,e,i);
		REV(a,b,c,d,e,t);
	}

	for (; i < 40; i++) {
		FF1(a,b,c,d,e,i);
		REV(a,b,c,d,e,t);
	}

	for (; i < 60; i++) {
		FF2(a,b,c,d,e,i);
		REV(a,b,c,d,e,t);
	}

	for (; i < 80; i++) {
		FF3(a,b,c,d,e,i);
		REV(a,b,c,d,e,t);
	}

	md->sha1.state[0] += a;
	md->sha1.state[1] += b;
	md->sha1.state[2] += c;
	md->sha1.state[3] += d;
	md->sha1.state[4] += e;
}

void
sha1_init(union hash_state *md)
{
	md->sha1.length   = 0;
	md->sha1.state[0] = 0x67452301UL;
	md->sha1.state[1] = 0xefcdab89UL;
	md->sha1.state[2] = 0x98badcfeUL;
	md->sha1.state[3] = 0x10325476UL;
	md->sha1.state[4] = 0xc3d2e1f0UL;
}

void
sha1_process(union hash_state *md, uint8_t *in, unsigned long len)
{
	unsigned r;

	r = md->sha1.length % 64;
	md->sha1.length += len;
	if (r) {
		if (len < 64 - r) {
			memcpy(md->sha1.buf + r, in, len);
			return;
		}
		memcpy(md->sha1.buf + r, in, 64 - r);
		len -= 64 - r;
		in  += 64 - r;
		sha1_compress(md, md->sha1.buf);
	}
	for (; len >= 64; len -= 64, in += 64)
		sha1_compress(md, in);
	memcpy(md->sha1.buf, in, len);
}

void
sha1_done(union hash_state *md, uint8_t *out)
{
	int i;
	unsigned r;

	r = md->sha1.length % 64;
	md->sha1.buf[r++] = 0x80;
	if (r > 56) {
		memset(md->sha1.buf + r, 0, 64 - r);
		sha1_compress(md, md->sha1.buf);
		r = 0;
	}

	memset(md->sha1.buf + r, 0, 56 - r);
	md->sha1.length *= 8;
	STORE64H(md->sha1.length, md->sha1.buf + 56);
	sha1_compress(md, md->sha1.buf);

	for (i = 0; i < 5; i++)
		STORE32H(md->sha1.state[i], out + (4 * i));
}

