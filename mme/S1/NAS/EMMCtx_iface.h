/* This application was initially developed as a Final Project by
 *     Vicent Ferrer Guasch (vicent.ferrerguasch@aalto.fi)
 * under the supervision of,
 *     Jukka Manner (jukka.manner@aalto.fi)
 *     Jose Costa-Requena (jose.costa@aalto.fi)
 * in AALTO University and partially funded by EIT ICT labs.
 */

/**
 * @file   EMMCtx_iface.h
 * @Author Vicent Ferrer
 * @date   July, 2015
 * @brief  EMMCtx Information Interface
 *
 * This module implements the EMMCtx
 */

#ifndef EMM_CTX_IFACE_H
#define EMM_CTX_IFACE_H

typedef void* EMMCtx;

/**
 * @brief EMMCtx Constructor
 * @return empty subscription
 *
 *  Allocates the EMMCtx handler
 */
EMMCtx emm_init();

/**
 * @brief Dealocates the EMMCtx Handler
 * @param [in]  s EMMCtx handler to be removed.
 */
void emm_free(EMMCtx s);

#endif /* EMM_CTX_IFACE_H*/
