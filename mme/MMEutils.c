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
 * @file   MMEutils.c
 * @Author Vicent Ferrer
 * @date   August, 2015
 * @brief  
 * @modifiedby Jesus Llorente, Todor Ginchev
 * @lastmodified 9 October 2017
 * 
 */

#include <string.h>
#include "MMEutils.h"
#include "logmgr.h"
#include "NAS.h"

void plmn_FillFromTBCD(mme_PLMN *plmn, const guint8 *tbcd){
    memcpy(plmn->tbcd, tbcd, 3);
}

void plmn_FillFromMCCMNCs(mme_PLMN *plmn, const guint16 mcc, const guint16 mnc){
    guint8 *tmp = plmn->tbcd;
    tmp[0] = ((mcc%100)/10<<4) | mcc/100;
    tmp[1] = (mcc%10);
    if(mnc/100 == 0){
        tmp[1]|=0xf0;
        tmp[2] = (mnc/10) | (mnc%10)<<4;
    }else{
        tmp[1]|= (mnc%10)<<4;
        tmp[2] = ((mnc%100)/10<<4) | mnc/100;
    }
}

gboolean plmn_Equal(const mme_PLMN *a, const mme_PLMN *b){
	if (! memcmp(a->tbcd, b->tbcd, 3 * sizeof(guint8)))
		return TRUE;
	else
		return FALSE;
}

/*
 * This function receives as input a plmn char array and a PLMN code in tbcd format. 
 * It converts the PLMN tbcd encoding into a human readable null terminated string 
 * and stores it in the char array. 
 */
void plmn_FillPLMNFromTBCD(guint8 *plmn, const guint8 *tbcd){
    plmn[0] = 0x30 + (tbcd[0] & 0x0f);
    plmn[1] = 0x30 + ((tbcd[0] & 0xf0) >> 4);
    plmn[2] = 0x30 + (tbcd[1] & 0x0f);
    plmn[3] = 0x30 + ((tbcd[1] & 0xf0) >> 4);
    plmn[4] = 0x30 + (tbcd[2] & 0x0f);
    plmn[5] = 0x30 + ((tbcd[2] & 0xf0) >> 4);
    plmn[6] = 0x00; // equivalent to '\0' null termination

    /* Check if there is 'F' padding to figure our of MNC is 2 or 3 digits */
    if((tbcd[1] & 0xf0) == 0xf0){
        plmn[3] = plmn[4];
	plmn[4] = plmn[5];
        plmn[5] = 0x00; // equivalent to '\0' null termination
    }
}


guint plmn_Hash(const mme_PLMN *key){
	guint32 nk = (key->tbcd[2] *10000) & (key->tbcd[1] * 100) & key->tbcd[0];
	return g_int_hash(&nk);
}


mme_GlobaleNBid *globaleNBID_Fill(mme_GlobaleNBid *r, const Global_ENB_ID_t *gid){
	plmn_FillFromTBCD( &(r->plmn), gid->pLMNidentity->tbc.s);
	if (gid->eNBid->choice == 0){
		r->macroId = TRUE;
		r->eNBid = gid->eNBid->id.macroENB_ID<<4;
	}else{
		r->macroId = FALSE;
		r->eNBid = gid->eNBid->id.homeENB_ID<<12;
	}
}

mme_GlobaleNBid *globaleNBID_copy(mme_GlobaleNBid *in, mme_GlobaleNBid *out){
	plmn_FillFromTBCD( &(out->plmn), in->plmn.tbcd);
	out->macroId = in->macroId;
	out->eNBid = in->eNBid;
	return out;
}

const guint globaleNB_getMCC(const mme_GlobaleNBid *gid){
	return ((gid->plmn.tbcd[0]&0x0f))*100 +
		(gid->plmn.tbcd[0]>>4)*10 +
		((gid->plmn.tbcd[1]&0x0f));
}

const guint globaleNB_getMNC(const mme_GlobaleNBid *gid){
	return ((gid->plmn.tbcd[1]>>4)==0xf)?
		(gid->plmn.tbcd[2]&0x0f)*10 + (gid->plmn.tbcd[2]>>4):
		(gid->plmn.tbcd[1]>>4)*100 + (gid->plmn.tbcd[2]&0x0f)*10 + (gid->plmn.tbcd[2]>>4);
}

const guint globaleNB_getCI(const mme_GlobaleNBid *gid){
	return gid->eNBid;
}

gboolean globaleNBID_Equal(const mme_GlobaleNBid *a, const mme_GlobaleNBid *b){
	return plmn_Equal(&(a->plmn), &(b->plmn)) &&
		(a->macroId == b->macroId) &&
		(a->eNBid == b->eNBid);
}

guint globaleNBID_Hash(const mme_GlobaleNBid *k){
	guint id = k->eNBid;
	return plmn_Hash(&(k->plmn)) ^ g_int_hash(&id);
}
