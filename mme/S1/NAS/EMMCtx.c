/* This application was initially developed as a Final Project by
 *     Vicent Ferrer Guasch (vicent.ferrerguasch@aalto.fi)
 * under the supervision of,
 *     Jukka Manner (jukka.manner@aalto.fi)
 *     Jose Costa-Requena (jose.costa@aalto.fi)
 * in AALTO University and partially funded by EIT ICT labs.
 */

/**
 * @file   EMMCtx.c
 * @Author Vicent Ferrer
 * @date   July, 2015
 * @brief  EMMCtx Information
 *
 * This module implements the EMMCtx Information
 */

#include <string.h>
#include "EMMCtx.h"
#include "logmgr.h"

EMMCtx emmCtx_init(){
    EMMCtx_t *self = g_new0(EMMCtx_t, 1);

    self->subs = subs_init();

    self->authQuadrs = g_ptr_array_new_full (5, g_free);
    self->authQuints = g_ptr_array_new_full (5, g_free);

    return self;
}

void emmCtx_free(EMMCtx s){
    EMMCtx_t *self = (EMMCtx_t*)s;
    g_ptr_array_free (self->authQuadrs, TRUE);
    g_ptr_array_free (self->authQuints, TRUE);

    subs_free(self->subs);
    g_free(self);
}

void emm_setState(EMMCtx emm_h, EMM_State *s){
    EMMCtx_t *self = (EMMCtx_t*)emm_h;
    self->state = s;
}
