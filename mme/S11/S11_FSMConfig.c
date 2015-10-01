/* This application was initially developed as a Final Project by
 *     Vicent Ferrer Guasch (vicent.ferrerguasch@aalto.fi)
 * under the supervision of,
 *     Jukka Manner (jukka.manner@aalto.fi)
 *     Jose Costa-Requena (jose.costa@aalto.fi)
 * in AALTO University and partially funded by EIT ICT labs.
 */

/**
 * @file   S11_FSMConfig.c
 * @Author Vicent Ferrer
 * @date   June, 2015
 * @brief  Functions to modify S11 FSM
 *
 */


//#include "MME_S11.c"
#include "S11_FSMConfig.h"
#include "S11_User.h"

#include "S11_NoCtx.h"
#include "S11_WCtxRsp.h"
#include "S11_UlCtx.h"
#include "S11_WModBearerRsp.h"
#include "S11_Ctx.h"
#include "S11_WDel.h"

S11_State *s11_states;

void s11ConfigureFSM(){
    s11_states = g_new(S11_State, 6);

    linkNoCtx(&s11_states[0]);
    linkWCtxRsp(&s11_states[1]);
    linkUlCtx(&s11_states[2]);
    linkWModBearerRsp(&s11_states[3]);
    linkCtx(&s11_states[4]);
    linkWDel(&s11_states[5]);
}

void s11DestroyFSM(){
    g_free(s11_states);
}


void s11changeState(gpointer session, S11State s){
    s11u_setState(session, &(s11_states[s]));
}
