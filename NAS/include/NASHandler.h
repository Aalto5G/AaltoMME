/* This application was initially developed as a Final Project by
 *     Vicent Ferrer Guasch (vicent.ferrerguasch@aalto.fi)
 * under the supervision of,
 *     Jukka Manner (jukka.manner@aalto.fi)
 *     Jose Costa-Requena (jose.costa@aalto.fi)
 * in AALTO University and partially funded by EIT ICT labs.
 */

/**
 * @file   NAS.h
 * @Author Vicent Ferrer
 * @date   September, 2015
 * @brief  Internal NAS handler
 */

#ifndef _NAS_HANDLER_H
#define _NAS_HANDLER_H

typedef struct{
	NAS_EIA i;
	NAS_EEA e;
	uint8_t key[16];
}NASHandler;

#endif  /* !_NAS_HANDLER_H */
