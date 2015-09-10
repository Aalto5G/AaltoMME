/* This application was initially developed as a Final Project by
 *     Vicent Ferrer Guasch (vicent.ferrerguasch@aalto.fi)
 * under the supervision of,
 *     Jukka Manner (jukka.manner@aalto.fi)
 *     Jose Costa-Requena (jose.costa@aalto.fi)
 * in AALTO University and partially funded by EIT ICT labs.
 */

/**
 * @file   eia0.h
 * @Author Vicent Ferrer
 * @date   September, 2015
 * @brief  EPS Integrity Algorithm 0
 *
 * EPS Integrity Algorithm 0: null integrity protection algorithm
 */

#ifndef EIA0_H
#define EIA0_H

#include <stdlib.h>
#include <stdint.h>

void eia0(const void *k,
          const void *count, const uint8_t bearer, const uint8_t direction,
          const void* msg, const size_t mLen,
          void *digest);


#endif /* EIA0_H */
