/* implementation based on libtomcrypt */
#include <stdint.h>
#include <string.h>

#include "crypto.h"

void
sha224_init(union hash_state *md)
{
	md->sha256.length   = 0;
	md->sha256.state[0] = 0xc1059ed8UL;
	md->sha256.state[1] = 0x367cd507UL;
	md->sha256.state[2] = 0x3070dd17UL;
	md->sha256.state[3] = 0xf70e5939UL;
	md->sha256.state[4] = 0xffc00b31UL;
	md->sha256.state[5] = 0x68581511UL;
	md->sha256.state[6] = 0x64f98fa7UL;
	md->sha256.state[7] = 0xbefa4fa4UL;
}

void
sha224_done(union hash_state *md, uint8_t *out)
{
	uint8_t buf[32];

	sha256_done(md, buf);
	memcpy(out, buf, 28);
}
