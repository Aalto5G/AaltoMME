/*
 * AES functions
 * Copyright (c) 2003-2006, Jouni Malinen <j@w1.fi>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * Alternatively, this software may be distributed under the terms of BSD
 * license.
 *
 * See README and COPYING for more details.
 */

#ifndef AES_H
#define AES_H

#include <stdint.h>

void * aes_encrypt_init(const uint8_t *key, size_t len);
void aes_encrypt(void *ctx, const uint8_t *plain, uint8_t *crypt);
void aes_encrypt_deinit(void *ctx);
void * aes_decrypt_init(const uint8_t *key, size_t len);
void aes_decrypt(void *ctx, const uint8_t *crypt, uint8_t *plain);
void aes_decrypt_deinit(void *ctx);

/* Wrapper*/
int aes_128_encrypt_block(const uint8_t *key, const uint8_t *in, uint8_t *out);

#endif /* AES_H */
