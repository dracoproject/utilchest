/* implementation based on libtomcrypt */
#include <stdint.h>
#include <string.h>

#include "crypto.h"

#define REV(a,b,c,d,e,f,g,h,t) t=h;h=g;g=f;f=e;e=d;d=c;c=b;b=a;a=t
#define Ch(a,b,c)  (c ^ (a & (b ^ c)))
#define Maj(a,b,c) ((a & b) | (c & (a | b)))
#define R(a,b)     (((a)&0xFFFFFFFF)>>(b))
#define G0(x)      (ror(x, 7)  ^ ror(x, 18) ^ R(x, 3))
#define G1(x)      (ror(x, 17) ^ ror(x, 19) ^ R(x, 10))
#define S0(x)      (ror(x, 2)  ^ ror(x, 13) ^ ror(x, 22))
#define S1(x)      (ror(x, 6)  ^ ror(x, 11) ^ ror(x, 25))

static const uint32_t K[64] = {
	0x428a2f98UL, 0x71374491UL, 0xb5c0fbcfUL, 0xe9b5dba5UL, 0x3956c25bUL,
	0x59f111f1UL, 0x923f82a4UL, 0xab1c5ed5UL, 0xd807aa98UL, 0x12835b01UL,
	0x243185beUL, 0x550c7dc3UL, 0x72be5d74UL, 0x80deb1feUL, 0x9bdc06a7UL,
	0xc19bf174UL, 0xe49b69c1UL, 0xefbe4786UL, 0x0fc19dc6UL, 0x240ca1ccUL,
	0x2de92c6fUL, 0x4a7484aaUL, 0x5cb0a9dcUL, 0x76f988daUL, 0x983e5152UL,
	0xa831c66dUL, 0xb00327c8UL, 0xbf597fc7UL, 0xc6e00bf3UL, 0xd5a79147UL,
	0x06ca6351UL, 0x14292967UL, 0x27b70a85UL, 0x2e1b2138UL, 0x4d2c6dfcUL,
	0x53380d13UL, 0x650a7354UL, 0x766a0abbUL, 0x81c2c92eUL, 0x92722c85UL,
	0xa2bfe8a1UL, 0xa81a664bUL, 0xc24b8b70UL, 0xc76c51a3UL, 0xd192e819UL,
	0xd6990624UL, 0xf40e3585UL, 0x106aa070UL, 0x19a4c116UL, 0x1e376c08UL,
	0x2748774cUL, 0x34b0bcb5UL, 0x391c0cb3UL, 0x4ed8aa4aUL, 0x5b9cca4fUL,
	0x682e6ff3UL, 0x748f82eeUL, 0x78a5636fUL, 0x84c87814UL, 0x8cc70208UL,
	0x90befffaUL, 0xa4506cebUL, 0xbef9a3f7UL, 0xc67178f2UL
};

static uint32_t
ror(uint32_t n, int k)
{
	return ((n >> k) | (n << (32 - k)));
}

static void
sha256_compress(union hash_state *md, uint8_t *buf)
{
	uint32_t W[64], a, b, c, d, e, f, g, h, t, t0, t1;
	int i;

	for (i = 0; i < 16; i++)
		LOAD32H(W[i], buf + (4 * i));

	for (i = 16; i < 64; i++)
		W[i] = G1(W[i - 2]) + W[i - 7] + G0(W[i - 15]) + W[i - 16];

	a = md->sha256.state[0];
	b = md->sha256.state[1];
	c = md->sha256.state[2];
	d = md->sha256.state[3];
	e = md->sha256.state[4];
	f = md->sha256.state[5];
	g = md->sha256.state[6];
	h = md->sha256.state[7];

	for (i = 0; i < 64; i++) {
		t0 = h + S1(e) + Ch(e, f, g) + K[i] + W[i];
		t1 = S0(a) + Maj(a, b, c);
		d += t0;
		h  = t0 + t1;
		REV(a, b, c, d, e, f, g, h, t);
	}

	md->sha256.state[0] += a;
	md->sha256.state[1] += b;
	md->sha256.state[2] += c;
	md->sha256.state[3] += d;
	md->sha256.state[4] += e;
	md->sha256.state[5] += f;
	md->sha256.state[6] += g;
	md->sha256.state[7] += h;
}

void
sha256_init(union hash_state *md)
{
	md->sha256.length   = 0;
	md->sha256.state[0] = 0x6A09E667UL;
	md->sha256.state[1] = 0xBB67AE85UL;
	md->sha256.state[2] = 0x3C6EF372UL;
	md->sha256.state[3] = 0xA54FF53AUL;
	md->sha256.state[4] = 0x510E527FUL;
	md->sha256.state[5] = 0x9B05688CUL;
	md->sha256.state[6] = 0x1F83D9ABUL;
	md->sha256.state[7] = 0x5BE0CD19UL;
}

void
sha256_process(union hash_state *md, uint8_t *in, unsigned long len)
{
	unsigned r;

	r = md->sha256.length % 64;
	md->sha256.length += len;
	if (r) {
		if (len < 64 - r) {
			memcpy(md->sha256.buf + r, in, len);
			return;
		}
		memcpy(md->sha256.buf + r, in, 64 - r);
		len -= 64 - r;
		in  += 64 - r;
		sha256_compress(md, md->sha256.buf);
	}
	for (; len >= 64; len -= 64, in += 64)
		sha256_compress(md, in);
	memcpy(md->sha256.buf, in, len);
}

void
sha256_done(union hash_state *md, uint8_t *out)
{
	int i;
	unsigned r;

	r = md->sha256.length % 64;
	md->sha256.buf[r++] = 0x80;
	if (r > 56) {
		memset(md->sha256.buf + r, 0, 64 - r);
		sha256_compress(md, md->sha256.buf);
		r = 0;
	}

	memset(md->sha256.buf + r, 0, 56 - r);
	md->sha256.length *= 8;
	STORE64H(md->sha256.length, md->sha256.buf + 56);
	sha256_compress(md, md->sha256.buf);

	for (i = 0; i < 8; i++)
		STORE32H(md->sha256.state[i], out + (4 * i));
}

