/* This application was initially developed as a Final Project by
 *     Vicent Ferrer Guasch (vicent.ferrerguasch@aalto.fi)
 * under the supervision of,
 *     Jukka Manner (jukka.manner@aalto.fi)
 *     Jose Costa-Requena (jose.costa@aalto.fi)
 * in AALTO University and partially funded by EIT ICT labs.
 */

/**
 * @file   eea0.h
 * @Author Vicent Ferrer
 * @date   September, 2015
 * @brief  EPS Encryption Algorithm 0
 *
 * EPS Encryption Algorithm 0: null ciphering algorithm
 */

#ifndef EEA0_H
#define EEA0_H

#include <stdlib.h>
#include <stdint.h>

void eea0_dec(const void *k,
              const void *count, const uint8_t bearer, const uint8_t direction,
              const void* msg, const size_t mLen,
              void* plain, size_t *pLen);

void eea0_cyph(const void *k,
               const void *count, const uint8_t bearer, const uint8_t direction,
               void* msg, size_t *mLen,
               const void* plain, const size_t pLen);

#endif /* EEA0_H */
