/* This application was initially developed as a Final Project by
 *     Vicent Ferrer Guasch (vicent.ferrerguasch@aalto.fi)
 * under the supervision of,
 *     Jukka Manner (jukka.manner@aalto.fi)
 *     Jose Costa-Requena (jose.costa@aalto.fi)
 * in AALTO University and partially funded by EIT ICT labs.
 */

/**
 * @file   EPS_Session.c
 * @Author Vicent Ferrer
 * @date   July, 2015
 * @brief  NAS ESM implementation
 * @brief  EPS_Session Information
 *
 * This module implements the EPS_Session Information retrived from the HSS
 */

#include <string.h>
#include "EPS_Session_priv.h"
#include "NAS_ESM_priv.h"
#include "NAS_EMM_priv.h"
#include "logmgr.h"
#include "MME_S11.h"
#include "S1AP.h"

EPS_Session ePSsession_init(ESM esm, Subscription _subs, ESM_BearerContext b){
    EPS_Session_t *self = g_new0(EPS_Session_t, 1);
    self->esm = esm;
    self->subs = _subs;
    self->defaultBearer = b;
    self->bearers = g_hash_table_new_full (g_int_hash,
                                           g_int_equal,
                                           NULL,
                                           esm_bc_free);
    self->current_pti = 0;
    return self;
}

void ePSsession_free(EPS_Session s){
    EPS_Session_t *self = (EPS_Session_t*)s;
    if(self->APN){
        g_string_free(self->apn_io_replacement, TRUE);
    }
    if(self->subscribedAPN){
        g_string_free(self->apn_io_replacement, TRUE);
    }
    if(self->apn_io_replacement){
        g_string_free(self->apn_io_replacement, TRUE);
    }
    g_hash_table_destroy(self->bearers);
    g_free(self);
}

void ePSsession_parsePDNConnectivityRequest(EPS_Session s, ESM_Message_t *msg,
                                            gboolean *infoTxRequired){
    uint8_t  *pointer, numOp=0;
    ie_tlv_t4_t *temp;
    EPS_Session_t *self = (EPS_Session_t*)s;
    PDNConnectivityRequest_t *pdnReq = (PDNConnectivityRequest_t*)msg;
    union nAS_ie_member const *optIE=NULL;

    *infoTxRequired = FALSE;
    self->current_pti = msg->procedureTransactionIdentity;
    /*Optionals*/
    /* ESM information transfer flag: 0xD*/
    nas_NASOpt_lookup(pdnReq->optionals, 4, 0xD, &optIE);
    if(optIE){
        *infoTxRequired = (gboolean)optIE->v_t1_l.v;
    }
    /*Access point name: 0x28*/
    /*Protocol configuration options: 0x27*/
    nas_NASOpt_lookup(pdnReq->optionals, 4, 0x27, &optIE);
    if(optIE){
        ePSsession_setPCO(self, &(optIE->tlv_t4), optIE->tlv_t4.l+2);
    }
    /*Device properties: 0xC*/
}

void ePSsession_sendActivateDefaultEPSBearerCtxtReq(EPS_Session s){
    EPS_Session_t *self = (EPS_Session_t*)s;

    guint8 buffer[150], *pointer, apn[150], pco[30];
    gsize len;

    struct qos_t qos;
    struct PAA_t paa;

    guint32 dns;

    memset(buffer, 0, 150);
    pointer = buffer;

    /*Activate default*/
    newNASMsg_ESM(&pointer,
                  EPSSessionManagementMessages,
                  esm_bc_getEBI(self->defaultBearer));
    encaps_ESM(&pointer, self->current_pti,
               ActivateDefaultEPSBearerContextRequest);

    /* /\* EPS QoS *\/ */
    subs_cpyQoS_GTP(self->subs, &qos);
    nasIe_lv_t4(&pointer, &(qos.qci), 1);

    /* Access point name*/
    len=0;
    subs_getEncodedAPN(self->subs, apn, 150, &len);
    nasIe_lv_t4(&pointer,
                apn,
                len);

    /* /\* PDN address *\/ */
    memset(&paa, 0, sizeof(struct PAA_t));
    paa.type = self->pdn_addr_type;
    switch(self->pdn_addr_type) {
    case 1: /*IPv4*/
        memcpy(&(paa.addr.ipv4), self->pdn_addr, 4);
        len=5;
        break;
    case 2: /*IPv6*/
        memcpy(&(paa.addr.ipv6), self->pdn_addr, 16);
        len=17;
        break;
    case 3: /* IPv4v6*/
        memcpy(&(paa.addr.both.ipv4), self->pdn_addr, 4);
        memcpy(&(paa.addr.both.ipv6), self->pdn_addr+4, 16);
        len=21;
        break;
    default:
        g_error("Wrong PDN address type %u", self->pdn_addr_type);
    }
    nasIe_lv_t4(&pointer, (uint8_t*)&paa, len);

    /*Optionals */
    ePSsession_getPCO(self, pco);
    /* Protocol Configuration Options*/
    if(pco[0]==0x27){
        dns = esm_getDNSsrv(self->esm);
        if(dns==0){
            log_msg(LOG_DEBUG, 0, "Writting PCO IE. Lenght: %d, first byte %#x",
                    pco[1]+2, pco+2);
            nasIe_tlv_t4(&pointer, 0x27, pco+2, pco[1]);
            *(pointer-1-pco[1]) = pco[1];
        }else{
            /* pco[0]=0x80; */
            /* pco[1]=0x80; // */
            /* pco[2]=0x21; */
            /* pco[3]=0x0a; //lenght */
            /* pco[4]=0x01; */
            /* pco[5]=0x00; */
            /* pco[6]=0x00; */
            /* pco[7]=0x0a; */
            /* pco[8]=0x81; */
            /* pco[9]=0x06; */
            uint8_t pco_r[] = {0x80,
                               0x80, 0x21, 0x10, 0x03, 0x01, 0x00, 0x10,
                                     0x81, 0x06, 0, 0, 0, 0,
                                     0x83, 0x06, 0, 0, 0, 0,
                               0x00, 0x0d, 0x04, 0, 0, 0, 0,};
            //0x00, 0x0a, 0x04, 0, 0, 0, 0};
            memcpy(pco_r+10, &(dns), 4);
            memcpy(pco_r+16, &(dns), 4);
            memcpy(pco_r+23, &(dns), 4);
            //memcpy(pco_r+30, self->pdn_addr, 4);

            nasIe_tlv_t4(&pointer, 0x27, pco_r, 27);
            //*(pointer-15) = 14;
        }
    }
    emm_attachAccept(self->esm->emm, buffer, pointer-buffer,
                     g_list_append(NULL, self));
}

void ePSsession_activateDefault(EPS_Session s, gboolean infoTxRequired){
    EPS_Session_t *self = (EPS_Session_t*)s;
    guint8 *pointer, buf[100];
    if(!infoTxRequired){
        self->s11 = S11_newUserAttach(esm_getS11iface(self->esm),
                                      self->esm->emm, self,
                                      ePSsession_sendActivateDefaultEPSBearerCtxtReq,
                                      self);
    }else{
        pointer = buf;
        newNASMsg_ESM(&pointer,
                      EPSSessionManagementMessages,
                      0);
        encaps_ESM(&pointer,
                   self->current_pti,
                   ESMInformationRequest);
        emm_sendESM(self->esm->emm, buf, pointer-buf, NULL);
    }
}

gboolean ePSsession_getPCO(const EPS_Session s, gpointer pco){
    EPS_Session_t *self = (EPS_Session_t*)s;

    if(self->pco[0]==0x27){
        memcpy(pco, self->pco, self->pco[1]+2);
        return TRUE;
    }
    return FALSE;
}

void ePSsession_setPCO(EPS_Session s, gconstpointer pco, gsize len){
    EPS_Session_t *self = (EPS_Session_t*)s;
    memcpy(self->pco, pco, len);
}

ESM_BearerContext ePSsession_getDefaultBearer(EPS_Session s){
    EPS_Session_t *self = (EPS_Session_t*)s;
    return self->defaultBearer;
}

void ePSsession_setPDNAddress(EPS_Session s, gpointer paa, gsize len){
    EPS_Session_t *self = (EPS_Session_t*)s;
    guint8 *p;
    p = (guint8*)paa;
    self->pdn_addr_type = p[0]&0x07;
    memcpy(self->pdn_addr, p+1, len-1);
}

const char* ePSsession_getPDNAddrStr(EPS_Session s, gpointer str, gsize maxlen){
    EPS_Session_t *self = (EPS_Session_t*)s;
    struct in_addr ipv4;
    struct in6_addr ipv6;

    switch(self->pdn_addr_type) {
    case 1: /*IPv4*/
        memcpy(&(ipv4.s_addr), self->pdn_addr, 4);
        return inet_ntop(AF_INET, &ipv4, str, maxlen);
    case 2: /*IPv6*/
        memcpy(&(ipv6.s6_addr), self->pdn_addr, 16);
        return inet_ntop(AF_INET6, &ipv6, str, maxlen);
    case 3: /* IPv4v6*/
        memcpy(&(ipv4.s_addr), self->pdn_addr, 4);
        return inet_ntop(AF_INET, &ipv4, str, maxlen);
    default:
        /* strncpy(s, "Unknown AF", maxlen); */
        return "Not valid";
    }
}


void ePSsession_getPDNAddr(const EPS_Session s, TransportLayerAddress_t* addr){
    EPS_Session_t *self = (EPS_Session_t*)s;

    switch(self->pdn_addr_type) {
    case 1: /*IPv4*/
        memcpy(addr->addr, self->pdn_addr, 4);
        addr->len = 32;
        break;
    case 2: /*IPv6*/
        memcpy(addr->addr, self->pdn_addr, 16);
        addr->len = 128;
        break;
    case 3: /* IPv4v6*/
        memcpy(addr->addr, self->pdn_addr, 20);
        addr->len = 160;
        break;
    default:
        /* strncpy(s, "Unknown AF", maxlen); */
        g_error("PDN Addr not valid in EPS Session");
    }
}

void ePSsession_test(EPS_Session s){
    log_msg(LOG_DEBUG, 0, "Activate Default EPS Bearer Context Accept");
}

void ePSsession_modifyE_RABList(EPS_Session s, E_RABsToBeModified_t* l,
                                void (*cb)(gpointer), gpointer args){
    ESM_BearerContext bearer;
    E_RABSetupItemCtxtSURes_t *item;
    struct fteid_t fteid;
    EPS_Session_t *self = (EPS_Session_t*)s;

    bearer = ePSsession_getDefaultBearer(self);

    item = l->item[0]->value;
    if (esm_bc_getEBI(bearer) != item->eRAB_ID.id){
        return;
    }

    memcpy(&(fteid.teid), &(item->gTP_TEID.teid), 4);
    if (item->transportLayerAddress->len == 32){
        fteid.ipv4=1;
        memcpy(&(fteid.addr.addrv4), &(item->transportLayerAddress->addr), 4);
    }else{
        log_msg(LOG_ERR, 0, "Only IPv4 implemented, len %u, %x",
                item->transportLayerAddress->len);
    }
    fteid.iface = hton8(S1U_eNB);
    esm_bc_setS1ueNBfteid(bearer, &fteid);

    S11_Attach_ModifyBearerReq(self->s11,
                               (void(*)(gpointer)) cb,
                               (gpointer)args);
}

void ePSsession_UEContextReleaseReq(EPS_Session s,
                                    void (*cb)(gpointer), gpointer args){
    EPS_Session_t *self = (EPS_Session_t*)s;

    S11_ReleaseAccessBearers(self->s11, cb, args);
}

void ePSsession_detach(EPS_Session s, void(*cb)(gpointer), gpointer args){
    EPS_Session_t *self = (EPS_Session_t*)s;
    S11_dettach(self->s11, cb, args);
}
