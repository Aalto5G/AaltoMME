/* This application was initially developed as a Final Project by
 *     Vicent Ferrer Guasch (vicent.ferrerguasch@aalto.fi)
 * under the supervision of,
 *     Jukka Manner (jukka.manner@aalto.fi)
 *     Jose Costa-Requena (jose.costa@aalto.fi)
 * in AALTO University and partially funded by EIT ICT labs.
 */

/**
 * @file   NAS_EMM.c
 * @Author Vicent Ferrer
 * @date   August, 2015
 * @brief  NAS EMM implementation
 *
 * This module implements the NAS EMM interface state machine on the MME EndPoint.
 */

#include <glib.h>
#include "NAS_EMM.h"
#include "NAS.h"
#include "logmgr.h"
#include "EMMCtx.h"


typedef struct{
    gpointer    emm;

}EMM_t;

gpointer emm_init(gpointer ecm){
    EMMCtx_t *self = emmCtx_init();
    emmConfigureFSM();
    emmChangeState(self, EMM_Deregistered);
    self->ecm = ecm;
    return self;
}

void emm_free(gpointer emm_h){
    EMMCtx_t *self = (EMMCtx_t*)emm_h;
    emmDestroyFSM();
    emmCtx_free(self);
}

void emm_processMsg(gpointer emm_h, gpointer buffer, size_t len){
    EMMCtx_t *self = (EMMCtx_t*)emm_h;
    GenericNASMsg_t msg;
    dec_NAS(&msg, buffer, len);

    log_msg(LOG_WARNING, 0, "Enter");

    self->state->processMsg(self, &msg);
}
