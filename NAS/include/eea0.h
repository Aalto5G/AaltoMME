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
