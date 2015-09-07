/* This application was initially developed as a Final Project by
 *     Vicent Ferrer Guasch (vicent.ferrerguasch@aalto.fi)
 * under the supervision of,
 *     Jukka Manner (jukka.manner@aalto.fi)
 *     Jose Costa-Requena (jose.costa@aalto.fi)
 * in AALTO University and partially funded by EIT ICT labs.
 */

/**
 * @file   EMM_SpecificProcedureInitiated.c
 * @Author Vicent Ferrer
 * @date   August, 2015
 * @brief  EMM State
 *
 */

#include "EMM_SpecificProcedureInitiated.h"
#include "logmgr.h"
#include "EMM_FSMConfig.h"
#include "NAS.h"
#include "NAS.h"
#include <string.h>

static void emmProcessMsg(gpointer emm_h, GenericNASMsg_t* msg){
    log_msg(LOG_ERR, 0, "Not Implemented");
}

static void emmAttachAccept(gpointer emm_h, gpointer esm_msg, gsize msgLen, GList *bearers){
	EMMCtx_t *emm = (EMMCtx_t*)emm_h;
	guint8 *pointer, buffer[150], count, t3412, guti_b[11];
	guti_t guti;
	guint32 mac;
	NAS_tai_list_t tAIl;
	gsize len;

	memset(buffer, 0, 150);
    pointer = buffer;

	if (emm->attachStarted == TRUE){
		emm->attachStarted = FALSE;
		/* Build Attach Accept*/
		newNASMsg_EMM(&pointer, EPSMobilityManagementMessages,
		              IntegrityProtectedAndCiphered);
		mac = 0;
		nasIe_v_t3(&pointer, (uint8_t *)&mac, 4);
		count = emm->nasDlCount % 0xff;
		nasIe_v_t3(&pointer, (uint8_t*)&count, 1);
		emm->nasDlCount++;

		newNASMsg_EMM(&pointer, EPSMobilityManagementMessages, PlainNAS);

		encaps_EMM(&pointer, AttachAccept);

		/* EPS attach result */
		nasIe_v_t1_l(&pointer, 1); /* EPS only */
		pointer++; /*Spare half octet*/
		/* T3412 value */
		t3412 = 0x23;
		nasIe_v_t3(&pointer, &t3412, 1); /* EPS only */
		/* TAI list */
		ecmSession_getTAIlist(emm->ecm, &tAIl, &len);
		nasIe_lv_t4(&pointer, (uint8_t*)&tAIl, len);
		/* ESM message container */
		nasIe_lv_t6(&pointer, esm_msg, msgLen);

		/* GUTI */
		emmCtx_newGUTI(emm, &guti);
		guti_b[0]=0xF6;   /*1111 0 110 - spare, odd/even , GUTI id*/
		memcpy(guti_b+1, &guti, 10);
		nasIe_tlv_t4(&pointer, 0x50, guti_b, 11);
	}else{
		
	}
	ecm_sendCtxtSUReq(emm->ecm, buffer, pointer-buffer, bearers);
	emmChangeState(emm, EMM_Registered);
}

void linkEMMSpecificProcedureInitiated(EMM_State* s){
    s->processMsg = emmProcessMsg;
    /* s->authInfoAvailable = emmAuthInfoAvailable; */
    s->attachAccept = emmAttachAccept;
}


