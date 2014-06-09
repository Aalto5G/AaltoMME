/* This application was initially developed as a Final Project by
 *     Vicent Ferrer Guasch (vicent.ferrerguasch@aalto.fi)
 * under the supervision of,
 *     Jukka Manner (jukka.manner@aalto.fi)
 *     Jose Costa-Requena (jose.costa@aalto.fi)
 * in AALTO University and partially funded by EIT ICT labs.
 */

/**
 * @file   NAS_Definitions.h
 * @Author Vicent Ferrer
 * @date   May, 2013
 * @brief  NAS FSM header
 *
 * NAS Definitions required outside the NAS State MAchine Scope
 */

#ifndef NAS_DEF_H
#define NAS_DEF_H

/* ======================================================================
 * NAS State Machines, States enum types
 * ====================================================================== */

/** EPS session management (ESM) states : 3FPP 24.301 clause 6.1.3.3
 * The ESM sublayer states for EPS bearer context handling in the network */
typedef enum ESM_State_c{
    ESM_Bearer_Context_Inactive = 0,
    ESM_Bearer_Context_Active_Pending,
    ESM_Bearer_Context_Active,
    ESM_Bearer_Context_Inactive_Pending,
    ESM_Bearer_Context_Modify_Pending,
}ESM_State_t;

/* EMM : EPS Mobility Management */
/* MM State  Mobility management state ECM-IDLE, ECM-CONNECTED, EMM-DEREGISTERED*/
/* ECM EPS Connection Manager
 * 3FPP 23.401 - clause 5.7.2*/
typedef enum EMM_State_c{
    EMM_Deregistered = 0,
    EMM_Registered,
    EMM_Specific_Procedure_Initiated,
    EMM_Common_Procedure_Initiated,
    EMM_Deregistered_Initiated,
}EMM_State_t;

/* ======================================================================
 * Other Type definitions
 * ====================================================================== */

/**@brief GUTI Type guti_t */
typedef struct guti_c{
    uint32_t    tbcd_plmn   :   24; /*< PLMN encoded with TBCD*/
    uint16_t    mmegi;              /*< MMEGI MME group Id*/
    uint8_t     mmec;               /*< MMEC MME Code*/
    uint32_t    mtmsi;              /*< M-TMSI */
}__attribute__((packed)) guti_t;

#endif /* NAS_DEF_H */
