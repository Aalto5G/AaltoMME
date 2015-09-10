/* This application was initially developed as a Final Project by
 *     Vicent Ferrer Guasch (vicent.ferrerguasch@aalto.fi)
 * under the supervision of,
 *     Jukka Manner (jukka.manner@aalto.fi)
 *     Jose Costa-Requena (jose.costa@aalto.fi)
 * in AALTO University and partially funded by EIT ICT labs.
 */

/**
 * @file   eia0.c
 * @Author Vicent Ferrer
 * @date   September, 2015
 * @brief  EPS Integrity Algorithm 0
 */

#include "eia0.h"

#include <string.h>

void eia0(const void *k,
          const void *count, const uint8_t bearer, const uint8_t direction,
          const void* msg, const size_t mLen,
          void *digest){

	memset(digest, 0, 4);
	return;
}
