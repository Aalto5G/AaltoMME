/* This application was initially developed as a Final Project by
 *     Vicent Ferrer Guasch (vicent.ferrerguasch@aalto.fi)
 * under the supervision of,
 *     Jukka Manner (jukka.manner@aalto.fi)
 *     Jose Costa-Requena (jose.costa@aalto.fi)
 * in AALTO University and partially funded by EIT ICT labs.
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
