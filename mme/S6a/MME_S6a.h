/* This application was initially developed as a Final Project by
 *     Vicent Ferrer Guasch (vicent.ferrerguasch@aalto.fi)
 * under the supervision of,
 *     Jukka Manner (jukka.manner@aalto.fi)
 *     Jose Costa-Requena (jose.costa@aalto.fi)
 * in AALTO University and partially funded by EIT ICT labs.
 */

/**
 * @file   MME_S6a.h
 * @Author Vicent Ferrer
 * @date   June, 2013
 * @brief  MME S6a interface protocol state machine.
 *
 * This module implements the S6a interface state machine.
 * It is currently only and emulator
 */

#ifndef MME_S6a_HFILE
#define MME_S6a_HFILE

#include <glib.h>

#include "MME.h"
#include "S6a.h"
#include "EMMCtx_iface.h"

#define MME_S6a mme_S6a_quark()

GQuark mme_S6a_quark();

typedef enum{
    S6a_UNKNOWN_EPS_SUBSCRIPTION,
    S6a_UNKNOWN_ERROR,
}S6aCause;

/**************************************************/
/* Interface to MME.c                             */
/**************************************************/

gpointer s6a_init(gpointer mme);

void s6a_free(gpointer s6a);


/**************************************************/
/* Interface to NAS_FSM.c                         */
/**************************************************/

void s6a_GetAuthInformation(gpointer s6a_h, EMMCtx emm,
                            void(*cb)(gpointer),
                            void(*error_cb)(gpointer, GError *err),
                            gpointer args);

void s6a_SynchAuthVector(gpointer s6a_h,  EMMCtx emm, uint8_t *auts,
                         void(*cb)(gpointer), gpointer args);

void s6a_UpdateLocation(gpointer s6a_h, EMMCtx emm,
                        void(*cb)(gpointer), gpointer args);


#endif /* MME_S6a_HFILE */
