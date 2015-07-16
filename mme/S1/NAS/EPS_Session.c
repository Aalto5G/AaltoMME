/* This application was initially developed as a Final Project by
 *     Vicent Ferrer Guasch (vicent.ferrerguasch@aalto.fi)
 * under the supervision of,
 *     Jukka Manner (jukka.manner@aalto.fi)
 *     Jose Costa-Requena (jose.costa@aalto.fi)
 * in AALTO University and partially funded by EIT ICT labs.
 */

/**
 * @file   EPS_Session.c
 * @Author Vicent Ferrer
 * @date   July, 2015
 * @brief  NAS ESM implementation
 * @brief  EPS_Session Information
 *
 * This module implements the EPS_Session Information retrived from the HSS
 */

#include <string.h>
#include "EPS_Session.h"
#include "logmgr.h"

EPS_Session session_init(){
    EPS_Session_t *self = g_new0(EPS_Session_t, 1);
    self->bearers = g_hash_table_new_full (g_int_hash,
                                           g_int_equal,
                                           NULL,
                                           g_free);
    return self;
}

void mm_free(EPS_Session s){
    EPS_Session_t *self = (EPS_Session_t*)s;
    if(self->APN){
        g_string_free(self->apn_io_replacement, TRUE);
    }
    if(self->subscribedAPN){
        g_string_free(self->apn_io_replacement, TRUE);
    }
    if(self->apn_io_replacement){
        g_string_free(self->apn_io_replacement, TRUE);
    }
    g_hash_table_destroy(self->bearers);
    g_free(self);
}
