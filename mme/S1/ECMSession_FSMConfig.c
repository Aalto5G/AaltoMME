/* This application was initially developed as a Final Project by
 *     Vicent Ferrer Guasch (vicent.ferrerguasch@aalto.fi)
 * under the supervision of,
 *     Jukka Manner (jukka.manner@aalto.fi)
 *     Jose Costa-Requena (jose.costa@aalto.fi)
 * in AALTO University and partially funded by EIT ICT labs.
 */

/**
 * @file   ECMSession_FSMConfig.c
 * @Author Vicent Ferrer
 * @date   August, 2015
 * @brief  Functions to modify ECMSession FSM
 *
 */

#include "S1Assoc.h"
#include "ECMSession_FSMConfig.h"
#include "ECMSession_priv.h"
/* Include States*/
#include "ECMSession_Idle.h"
#include "ECMSession_Connected.h"

#include "EMM_FSMConfig.h"

#include "logmgr.h"

ECMSession_State *ecm_states;


void ecm_ConfigureFSM(){
    ecm_states = g_new(ECMSession_State, 2);

    emmConfigureFSM();

    linkECMSessionIdle(&ecm_states[0]);
    linkECMSessionConnected(&ecm_states[1]);
}

void ecm_DestroyFSM(){
    emmDestroyFSM();
    g_free(ecm_states);
}


void ecm_ChangeState(gpointer ecm, ECMSessionState s){
    ecmSession_setState(ecm, &(ecm_states[s]), s);
}

void ecm_notImplemented(gpointer self){
    log_msg(LOG_ERR, 0, "Not Implemented");
}
