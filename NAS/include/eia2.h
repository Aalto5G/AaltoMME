/* This application was initially developed as a Final Project by
 *     Vicent Ferrer Guasch (vicent.ferrerguasch@aalto.fi)
 * under the supervision of,
 *     Jukka Manner (jukka.manner@aalto.fi)
 *     Jose Costa-Requena (jose.costa@aalto.fi)
 * in AALTO University and partially funded by EIT ICT labs.
 */

/**
 * @file   eia2.h
 * @Author Vicent Ferrer
 * @date   September, 2015
 * @brief  EPS Integrity Algorithm 2
 *
 * EPS Integrity Algorithm 2: CMAC, AES 128 CBC mode
 */

#ifndef EIA2_H
#define EIA2_H

#include <stdlib.h>
#include <stdint.h>

void eia2(const void *k,
          const void *count, const uint8_t bearer, const uint8_t direction,
          const void* msg, const size_t mLen,
          void *digest);


#endif /* EIA2_H */
