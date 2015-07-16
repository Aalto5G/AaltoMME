/* This application was initially developed as a Final Project by
 *     Vicent Ferrer Guasch (vicent.ferrerguasch@aalto.fi)
 * under the supervision of,
 *     Jukka Manner (jukka.manner@aalto.fi)
 *     Jose Costa-Requena (jose.costa@aalto.fi)
 * in AALTO University and partially funded by EIT ICT labs.
 */

/**
 * @file   EPS_Session.h
 * @Author Vicent Ferrer
 * @date   July, 2015
 * @brief  EPS_Session Information 
 *
 * EPS Session Interface
 */

#ifndef EPS_SESSION_IFACE_H
#define EPS_SESSION_IFACE_H

typedef void* EPS_Session;

/**
 * @brief EPS_Session Constructor
 * @return empty session
 *
 *  Allocates the EPS_Session handler
 */
EPS_Session emm_init();

/**
 * @brief Dealocates the EPS_Session Handler
 * @param [in]  s EPS_Session handler to be removed.
 */
void emm_free(EPS_Session s);

#endif /* EPS_SESSION_IFACE_H*/
