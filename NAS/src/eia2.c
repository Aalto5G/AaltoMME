/* AaltoMME - Mobility Management Entity for LTE networks
 * Copyright (C) 2013 Vicent Ferrer Guash & Jesus Llorente Santos
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

/**
 * @file   eia2.c
 * @Author Vicent Ferrer
 * @date   September, 2015
 * @brief  EPS Integrity Algorithm 2
 */

#include "eia2.h"

#include <string.h>
#include <openssl/evp.h>
#include <openssl/evp.h>

#define M_EVP_CIPHER_CTX_block_size(e)	((e)->cipher->block_size)
#define M_EVP_CIPHER_CTX_cipher(e)	((e)->cipher)
#define M_EVP_EncryptInit_ex(ctx,ciph,impl,key,iv) \
	(EVP_CipherInit_ex(ctx,ciph,impl,key,iv,1))

typedef struct {
	/* Cipher context to use */
	EVP_CIPHER_CTX cctx;
	/* Keys k1 and k2 */
	unsigned char k1[EVP_MAX_BLOCK_LENGTH];
	unsigned char k2[EVP_MAX_BLOCK_LENGTH];
	/* Temporary block */
	unsigned char tbl[EVP_MAX_BLOCK_LENGTH];
	/* Last (possibly partial) block */
	unsigned char last_block[EVP_MAX_BLOCK_LENGTH];
	/* Number of bytes in last block: -1 means context not initialised */
	int nlast_block;
}NAS_CMAC_CTX;


/* Make temporary keys K1 and K2 */
static void make_kn(unsigned char *k1, const unsigned char *l, int bl)
{
	int i;
	unsigned char c = l[0], carry = c >> 7, cnext;
	/* Shift block to left, including carry */
	for (i = 0; i < bl - 1; i++, c = cnext)
		k1[i] = (c << 1) | ((cnext = l[i + 1]) >> 7);
	/* If MSB set fixup with R */
	k1[i] = (c << 1) ^ ((0 - carry) & (bl == 16 ? 0x87 : 0x1b));
}

NAS_CMAC_CTX *NAS_CMAC_CTX_new(void)
{
	NAS_CMAC_CTX *ctx;
	ctx = OPENSSL_malloc(sizeof(*ctx));
	if (!ctx)
		return NULL;
	EVP_CIPHER_CTX_init(&ctx->cctx);
	ctx->nlast_block = -1;
	return ctx;
}

void NAS_CMAC_CTX_cleanup(NAS_CMAC_CTX *ctx)
{
	EVP_CIPHER_CTX_cleanup(&ctx->cctx);
	OPENSSL_cleanse(ctx->tbl, EVP_MAX_BLOCK_LENGTH);
	OPENSSL_cleanse(ctx->k1, EVP_MAX_BLOCK_LENGTH);
	OPENSSL_cleanse(ctx->k2, EVP_MAX_BLOCK_LENGTH);
	OPENSSL_cleanse(ctx->last_block, EVP_MAX_BLOCK_LENGTH);
	ctx->nlast_block = -1;
}

void NAS_CMAC_CTX_free(NAS_CMAC_CTX *ctx)
{
	if (!ctx)
		return;
	NAS_CMAC_CTX_cleanup(ctx);
	OPENSSL_free(ctx);
}


int NAS_CMAC_Init(NAS_CMAC_CTX *ctx, const void *key, size_t keylen,
              const EVP_CIPHER *cipher, ENGINE *impl)
{
	static const unsigned char zero_iv[EVP_MAX_BLOCK_LENGTH] = { 0 };
	/* All zeros means restart */
	if (!key && !cipher && !impl && keylen == 0) {
		/* Not initialised */
		if (ctx->nlast_block == -1)
			return 0;
		if (!M_EVP_EncryptInit_ex(&ctx->cctx, NULL, NULL, NULL, zero_iv))
			return 0;
		memset(ctx->tbl, 0, M_EVP_CIPHER_CTX_block_size(&ctx->cctx));
		ctx->nlast_block = 0;
		return 1;
	}
	/* Initialiase context */
	if (cipher && !M_EVP_EncryptInit_ex(&ctx->cctx, cipher, impl, NULL, NULL))
		return 0;
	/* Non-NULL key means initialisation complete */
	if (key) {
		int bl;
		if (!M_EVP_CIPHER_CTX_cipher(&ctx->cctx))
			return 0;
		if (!EVP_CIPHER_CTX_set_key_length(&ctx->cctx, keylen))
			return 0;
		if (!M_EVP_EncryptInit_ex(&ctx->cctx, NULL, NULL, key, zero_iv))
			return 0;
		bl = M_EVP_CIPHER_CTX_block_size(&ctx->cctx);
		if (!EVP_Cipher(&ctx->cctx, ctx->tbl, zero_iv, bl))
			return 0;
		make_kn(ctx->k1, ctx->tbl, bl);
		make_kn(ctx->k2, ctx->k1, bl);
		OPENSSL_cleanse(ctx->tbl, bl);
		/* Reset context again ready for first data block */
		if (!M_EVP_EncryptInit_ex(&ctx->cctx, NULL, NULL, NULL, zero_iv))
			return 0;
		/* Zero tbl so resume works */
		memset(ctx->tbl, 0, bl);
		ctx->nlast_block = 0;
	}
	return 1;
}

int NAS_CMAC_Update(NAS_CMAC_CTX *ctx, const void *in, size_t dlen)
{
	const unsigned char *data = in;
	size_t bl;
	if (ctx->nlast_block == -1)
		return 0;
	if (dlen == 0)
		return 1;
	bl = M_EVP_CIPHER_CTX_block_size(&ctx->cctx);
	/* Copy into partial block if we need to */
	if (ctx->nlast_block > 0) {
		size_t nleft;
		nleft = bl*8 - ctx->nlast_block;
		if (dlen < nleft)
			nleft = dlen;
		/* TODO, align data*/
		memcpy(ctx->last_block + ctx->nlast_block, data, nleft/8 + (nleft%8?1:0));
		dlen -= nleft;
		ctx->nlast_block += nleft;
		/* If no more to process return */
		if (dlen == 0)
			return 1;
		data += nleft/8;
		/* Else not final block so encrypt it */
		if (!EVP_Cipher(&ctx->cctx, ctx->tbl, ctx->last_block, bl))
			return 0;
	}
	/* Encrypt all but one of the complete blocks left */
	while (dlen > bl*8) {
		if (!EVP_Cipher(&ctx->cctx, ctx->tbl, data, bl))
			return 0;
		dlen -= bl*8;
		data += bl;
	}
	/* Copy any data left to last block buffer */
	memcpy(ctx->last_block, data, dlen/8 + (dlen%8?1:0));
	ctx->nlast_block = dlen;
	return 1;
}

int NAS_CMAC_Final(NAS_CMAC_CTX *ctx, unsigned char *out, size_t *poutlen)
{
	int i, bl, lb, lastbyte;
	if (ctx->nlast_block == -1)
		return 0;
	bl = M_EVP_CIPHER_CTX_block_size(&ctx->cctx);
	*poutlen = (size_t)bl;
	if (!out)
		return 1;
	lb = ctx->nlast_block;
	/* Is last block complete? */
	if (lb == bl*8) {
		for (i = 0; i < bl; i++)
			out[i] = ctx->last_block[i] ^ ctx->k1[i];
	} else if (lb%8 == 0){
		lastbyte = lb/8;
		ctx->last_block[lastbyte] = 0x80;
		if (bl - lastbyte > 1)
			memset(ctx->last_block + lastbyte + 1, 0, bl - lastbyte - 1);
		for (i = 0; i < bl; i++)
			out[i] = ctx->last_block[i] ^ ctx->k2[i];
	} else {
		lastbyte = lb/8;
		ctx->last_block[lastbyte] = (ctx->last_block[lastbyte] | (1 << (7-lb%8))) & 0xFF<<(7-lb%8);
		if (bl - lastbyte > 1)
			memset(ctx->last_block + lastbyte + 1, 0, bl - lastbyte - 1);
		for (i = 0; i < bl; i++)
			out[i] = ctx->last_block[i] ^ ctx->k2[i];
	}
	if (!EVP_Cipher(&ctx->cctx, out, out, bl)) {
		OPENSSL_cleanse(out, bl);
		return 0;
	}
	return 1;
}

void eia2(const void *k,
          const void *count, const uint8_t bearer, const uint8_t direction,
          const void* msg, const size_t mLen,
          void *digest){

	uint8_t *s;
	uint8_t mact[16] = {0};
	size_t sLen, mactlen;
	size_t len = 64 + mLen;

    NAS_CMAC_CTX *ctx = NAS_CMAC_CTX_new();

    
    /* Creation of S */
    sLen = 8 + mLen/8 + (mLen%8?1:0);
    s = (uint8_t *)malloc(sLen);
    memset(s, 0, sLen);
    memcpy(s, count, 4);
    s[4] = bearer<<3 | direction << 2;
    memcpy(s+8, msg, sLen-8);

    NAS_CMAC_Init(ctx, k, 16, EVP_aes_128_cbc(), NULL);
    NAS_CMAC_Update(ctx, s, len);

    NAS_CMAC_Final(ctx, mact, &mactlen);
    
    /*Copy output*/
    memcpy(digest, mact, 4);
    
    NAS_CMAC_CTX_free(ctx);
    free(s);
}
