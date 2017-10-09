/* This application was initially developed as a Final Project by
 *     Vicent Ferrer Guasch (vicent.ferrerguasch@aalto.fi)
 * under the supervision of,
 *     Jukka Manner (jukka.manner@aalto.fi)
 *     Jose Costa-Requena (jose.costa@aalto.fi)
 * in AALTO University and partially funded by EIT ICT labs.
 */

/**
 * @file   MMEutils.h
 * @Author Vicent Ferrer
 * @date   August, 2015
 * @brief  
 * @modifiedby Jesus Llorente, Todor Ginchev
 * @lastmodified 10 October 2017
 * 
 */

#ifndef MMEUTILS_H
#define MMEUTILS_H

#include <glib.h>
#include "S1AP.h"

typedef struct{
	guint8 tbcd[3];
}mme_PLMN;

typedef struct{
	mme_PLMN    plmn;
	gboolean    macroId;
	guint32     eNBid:28;
}mme_GlobaleNBid;

/**
 * @brief fill the PLMN struct
 * @param [out] plmn struct to be filled
 * @param [in] tbcd input data
 *
 * The format of the input is TBCD
 */
void plmn_FillFromTBCD(mme_PLMN *plmn, const guint8 *tbcd);

/**
 * @brief fill the PLMN struct
 * @param [out] plmn struct to be filled
 * @param [in] mcc input data
 * @param [in] mnc input data
 *
 * The format of the input is MCC, MNC
 */
void plmn_FillFromMCCMNCs(mme_PLMN *plmn, const guint16 mcc, const guint16 mnc);

void plmn_FillPLMNFromTBCD(guint8 *plmn, const guint8 *tbcd);

gboolean plmn_Equal(const mme_PLMN *a, const mme_PLMN *b);

guint plmn_Hash(const mme_PLMN *key);


mme_GlobaleNBid *globaleNBID_Fill(mme_GlobaleNBid *r, const Global_ENB_ID_t *gid);
mme_GlobaleNBid *globaleNBID_copy(mme_GlobaleNBid *in, mme_GlobaleNBid *out);

const guint globaleNB_getMCC(const mme_GlobaleNBid *gid);
const guint globaleNB_getMNC(const mme_GlobaleNBid *gid);
const guint globaleNB_getCI(const mme_GlobaleNBid *gid);

gboolean globaleNBID_Equal(const mme_GlobaleNBid *a, const mme_GlobaleNBid *b);

guint globaleNBID_Hash(const mme_GlobaleNBid *k);


#endif /* MMEUTILS_H*/
