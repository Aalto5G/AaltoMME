/* This application was initially developed as a Final Project by
 *     Vicent Ferrer Guasch (vicent.ferrerguasch@aalto.fi)
 * under the supervision of,
 *     Jukka Manner (jukka.manner@aalto.fi)
 *     Jose Costa-Requena (jose.costa@aalto.fi)
 * in AALTO University and partially funded by EIT ICT labs.
 */

/**
 * @file   NAS_ESM_priv.h
 * @Author Vicent Ferrer
 * @date   September, 2015
 * @brief  ESM logic
 *
 */

#ifndef NAS_ESM_PRIV_HFILE
#define NAS_ESM_PRIV_HFILE

#include <glib.h>
#include "NAS_ESM.h"

typedef struct{
	gpointer    emm;
	gpointer    s11_iface;
	GHashTable* bearers;
	GHashTable* sessions;
	uint8_t     next_ebi;
	void        (*cb)(gpointer);
    gpointer    args;
}ESM_t;

/* API to EPS session and bearers */

/**@brief 
 * @param [in] em_h     ESM handler
 * @return 
 *
 * */
gpointer esm_getS11iface(ESM esm_h);

uint32_t esm_getDNSsrv(ESM esm_h);


#endif /* NAS_ESM_PRIV_HFILE */
