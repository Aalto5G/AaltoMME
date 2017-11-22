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
 * @file   S1Assoc.h
 * @Author Vicent Ferrer
 * @date   August, 2015
 * @brief  ECM logic
 *
 * This Module implements the ECM session logic, the user associated messages
 * of S1AP
 */

#ifndef S1ASSOC_HFILE
#define S1ASSOC_HFILE

#include <glib.h>

#include "MME.h"
#include "MME_S1.h"
#include "S1AP.h"
#include "MMEutils.h"

typedef gpointer S1Assoc;

//g_quark_from_string("S1AP assoc");
//G_DEFINE_QUARK(S1AP_ASSOC, s1Assoc);

/* API to MME_S1 */
S1Assoc s1Assoc_init(S1 s1);

void s1Assoc_free(gpointer h);

/**@brief Accept a new eNB sctp association
 * @param [in] h  S1 association handler
 * @param [in] ss SCTP Server Socket
 *
 * Accept a new SCTP connection towards a eNB and store the
 * information on the handler
 */
void s1Assoc_accept(S1Assoc h, int ss);

void s1Assoc_disconnect(S1Assoc h);

void s1Assoc_registerECMSession(S1Assoc h, gpointer ecm);

void s1Assoc_deregisterECMSession(S1Assoc h, gpointer ecm);


/* ************************************************** */
/*                      Accessors                     */
/* ************************************************** */

/**@brief Get pointer to file descriptor
 * @param [in] h  S1 association handler
 * @return file descriptor pointer
 *
 * Function used to get the internal address of the fd, used as a key
 */
int *s1Assoc_getfd_p(const S1Assoc h);


/**@brief Get file descriptor
 * @param [in] h  S1 association handler
 * @return file descriptor
 *
 * Function used to get the internal file descriptor
 */
const int s1Assoc_getfd(const S1Assoc h);

mme_GlobaleNBid *s1Assoc_getID_p(const S1Assoc h);

const char *s1Assoc_getName(const S1Assoc h);

mme_GlobaleNBid *s1Assoc_getID(const S1Assoc h, mme_GlobaleNBid *out);

void s1Assoc_paging(S1Assoc h, gpointer emm);

#endif /* S1ASSOC_HFILE */
