/*
 * UMTS AKA - Milenage algorithm (3GPP TS 35.205, .206, .207, .208)
 * Copyright (c) 2006 <j@w1.fi>
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

#ifndef MILENAGE_H
#define MILENAGE_H

#include <stdint.h>

void getOPC(const uint8_t *op, const uint8_t *k, uint8_t *opc);

void milenage_generate(const uint8_t *opc, const uint8_t *amf, const uint8_t *k,
		       const uint8_t *sqn, const uint8_t *_rand, uint8_t *autn, uint8_t *ik,
		       uint8_t *ck, uint8_t *res, size_t *res_len);
int milenage_auts(const uint8_t *opc, const uint8_t *k, const uint8_t *_rand, const uint8_t *auts,
		  uint8_t *sqn);
void gsm_milenage(const uint8_t *opc, const uint8_t *k, const uint8_t *_rand, uint8_t *sres,
		  uint8_t *kc);

#endif /* MILENAGE_H */
