/* AaltoMME - Mobility Management Entity for LTE networks
 * Copyright (C) 2013 Vicent Ferrer Guash & Jesus Llorente Santos
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
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

gboolean plmn_Equal(const mme_PLMN *a, const mme_PLMN *b);

void plmn_FillPLMNFromTBCD(guint8 *plmn, const guint8 *tbcd);

guint plmn_Hash(const mme_PLMN *key);


mme_GlobaleNBid *globaleNBID_Fill(mme_GlobaleNBid *r, const Global_ENB_ID_t *gid);
mme_GlobaleNBid *globaleNBID_copy(mme_GlobaleNBid *in, mme_GlobaleNBid *out);

const guint globaleNB_getMCC(const mme_GlobaleNBid *gid);
const guint globaleNB_getMNC(const mme_GlobaleNBid *gid);
const guint globaleNB_getCI(const mme_GlobaleNBid *gid);

gboolean globaleNBID_Equal(const mme_GlobaleNBid *a, const mme_GlobaleNBid *b);

guint globaleNBID_Hash(const mme_GlobaleNBid *k);


#endif /* MMEUTILS_H*/
