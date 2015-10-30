/* This application was initially developed as a Final Project by
 *     Vicent Ferrer Guasch (vicent.ferrerguasch@aalto.fi)
 * under the supervision of,
 *     Jukka Manner (jukka.manner@aalto.fi)
 *     Jose Costa-Requena (jose.costa@aalto.fi)
 * in AALTO University and partially funded by EIT ICT labs.
 */

/**
 * @file   S1Assoc_FSMConfig.h
 * @Author Vicent Ferrer
 * @date   July, 2015
 * @brief  Functions to modify S1Assoc FSM
 *
 */

#ifndef S1ASSOC_FSMCONFIG_HFILE
#define S1ASSOC_FSMCONFIG_HFILE

#include <glib.h>

/**
 * S1Assoc states
 */
typedef enum{
    S1_NotConfigured,
    S1_Active,
}S1AssocState;


static const char *S1AssocStateName[] = {"S1-NoConf",
                                         "S1-Active"};

void s1ConfigureFSM();

void s1DestroyFSM();


void s1ChangeState(gpointer self, S1AssocState s);

void s1notImplemented(gpointer self);

#endif /* S1Assoc_FSMCONFIG_HFILE */
