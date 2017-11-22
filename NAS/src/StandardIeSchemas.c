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
 * @file   StandardIeSchemas.c
 * @Author Vicent Ferrer
 * @date   May, 2013
 * @brief  IE Encapsulate Functions
 */

#include "StandardIeSchemas.h"
#include "NASlog.h"

#include <arpa/inet.h>

void nasIe_v_t1_l(uint8_t** p, uint8_t v){
    ((ie_v_t1_l_t *)*p)->v = v;
}

void nasIe_v_t1_h(uint8_t** p, uint8_t v){
    ((ie_v_t1_h_t *)*p)->v = v;
    (*p)++;
}

void nasIe_tv_t1(uint8_t** p, const uint8_t t, const uint8_t v){
    ((ie_tv_t1_t *)*p)->t = t;
    ((ie_tv_t1_t *)*p)->v = v;
    (*p)++;
}

void nasIe_t_t2(uint8_t** p, const uint8_t t){
    *(ie_t_t2_t *)*p = t;
    (*p)++;
}

void nasIe_v_t3(uint8_t** p, const uint8_t *v, const uint8_t vsize){
    uint8_t i;
    for(i=0; i<vsize; i++){
        (*p)[i] = v[i];
    }
    (*p)+=vsize;
}

void nasIe_tv_t3(uint8_t** p, const uint8_t t, const uint8_t *v, const uint8_t vsize){
    uint8_t i;

    ((ie_tv_t3_t *)*p)->t = t;
    (*p)++;

    for(i=0; i<vsize; i++){
        (*p)[i] = v[i];
    }
    (*p)+=vsize;
}

void nasIe_lv_t4(uint8_t** p, const uint8_t *v, const uint8_t len){
    uint8_t i;

    ((ie_lv_t4_t *)*p)->l = len;
    (*p)++;

    for(i=0; i<len; i++){
        (*p)[i] = v[i];
    }
    (*p)+=len;
}

void nasIe_tlv_t4(uint8_t** p, const uint8_t t, const uint8_t *v, const uint8_t len){
    uint8_t i;

    ((ie_tlv_t4_t *)*p)->t = t;

    ((ie_tlv_t4_t *)*p)->l = len;
    (*p)+=2;

    for(i=0; i<len; i++){
        (*p)[i] = v[i];
    }
    (*p)+=len;
}

void nasIe_lv_t6(uint8_t** p, const uint8_t *v, const uint16_t len){
    uint16_t i;
    ((ie_lv_t6_t *)*p)->l = htons(len);
    (*p)+=2;

    for(i=0; i<len; i++){
        (*p)[i] = v[i];
    }
    (*p)+=len;
}

void nasIe_tlv_t6(uint8_t** p, const uint8_t t, const uint8_t *v, const uint16_t len){
    uint16_t i;

    ((ie_tlv_t6_t *)*p)->t = t;

    ((ie_tlv_t6_t *)*p)->l = htons(len);
    (*p)+=3;

    for(i=0; i<len; i++){
        (*p)[i] = v[i];
    }
    (*p)+=len;
}
