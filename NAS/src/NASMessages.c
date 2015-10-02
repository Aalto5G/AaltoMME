/* This application was initially developed as a Final Project by
 *     Vicent Ferrer Guasch (vicent.ferrerguasch@aalto.fi)
 * under the supervision of,
 *     Jukka Manner (jukka.manner@aalto.fi)
 *     Jose Costa-Requena (jose.costa@aalto.fi)
 * in AALTO University and partially funded by EIT ICT labs.
 */

/**
 * @file   Messages.c
 * @Author Vicent Ferrer
 * @date   May, 2013
 * @brief  NAS messages decoder/encoder
 */

#include "NASMessages.h"
#include "NASlog.h"
#include <string.h>
#include <netinet/in.h>


void dec_nonImperativeIE(union nAS_ie_member **optionals, uint8_t *buffer, uint32_t size);


void dec_NASOpt_tlv(union nAS_ie_member *optionals, uint8_t **buffer, uint32_t *size,
                    uint8_t *optIndex, uint32_t type);

void dec_NASOpt_tv_t3(union nAS_ie_member *optionals, uint8_t **buffer, uint32_t *size,
                      uint8_t *optIndex, uint32_t type, uint32_t len);

void dec_NASOpt_tv_t1(union nAS_ie_member *optionals, uint8_t **buffer, uint32_t *size,
                      uint8_t *optIndex, uint32_t type, uint32_t valueMask);


/* ********************** EMM ********************** */
void dec_IdentityResponse(IdentityResponse_t *msg, uint8_t *buffer, uint32_t size) {
    /*EPSMobileId*/
    msg->mobileId.l = *buffer;
    memcpy(msg->mobileId.v, ++buffer, msg->mobileId.l);
    buffer+=msg->mobileId.l;
    nas_msg(NAS_DEBUG, 0, "DEC : ePSMobileId len = %u, v = %#x%x", msg->mobileId.l, msg->mobileId.v[0], msg->mobileId.v[1]);
}

void dec_AuthenticationResponse(AuthenticationResponse_t *msg, uint8_t *buffer, uint32_t size){
    /* AuthParam*/
    msg->authParam.l = *buffer;
    memcpy(msg->authParam.v, ++buffer, msg->authParam.l);
    buffer+=msg->authParam.l;
    nas_msg(NAS_DEBUG, 0, "DEC : authParam len = %u, v = %#x%x", msg->authParam.l, msg->authParam.v[0], msg->authParam.v[1]);
}

void dec_AuthenticationFailure(AuthenticationFailure_t *msg, uint8_t *buffer, uint32_t size){
    uint8_t  opT, numOp=0;
    ie_tlv_t4_t *temp;

    /*eMMCause*/
    memcpy(&(msg->eMMCause), buffer,1);
    buffer++;
    nas_msg(NAS_DEBUG, 0, "DEC : eMMCause = %#x", msg->eMMCause);


    size--;
    if(size == 0)
        return;

    /*Optionals*/
    opT = *buffer;
    if(opT == 0x30){
        /*Protocol configuration options*/
        temp = (ie_tlv_t4_t *)buffer;
        memcpy( msg->optionals +numOp, temp, temp->l+2);

        buffer += temp->l;
        size -= 2;
        size -= temp->l;
        numOp++;
        if(size == 0)
            return;
        opT = *buffer;
    }
}


void dec_AttachAccept(AttachAccept_t *msg, uint8_t *buffer, uint32_t size){

    /*EPSAttachType and NASKeySetId*/
    msg->ePSAttachResult.v = (*buffer)&0x0F;
    /*Spare*/
    buffer++;
    nas_msg(NAS_DEBUG, 0, "DEC : ePSAttachResult = %#x", msg->ePSAttachResult.v);

    /*t3412*/
    memcpy(msg->t3412, buffer,1);
    buffer++;
    nas_msg(NAS_DEBUG, 0, "DEC : t3412 = %#x", msg->t3412[0]);

    /*UENetworkCapability*/
    msg->tAI_list.l = *buffer;
    buffer++;
    memcpy(msg->tAI_list.v, buffer, msg->tAI_list.l);
    buffer+=msg->tAI_list.l;
    nas_msg(NAS_DEBUG, 0, "DEC : tAI_list len = %u, v = %#x%x", msg->tAI_list.l, msg->tAI_list.v[0], msg->tAI_list.v[1]);

    /*ESM_MessageContainer*/
    memcpy(&(msg->eSM_MessageContainer.l), buffer, 2);
    msg->eSM_MessageContainer.l = htons(msg->eSM_MessageContainer.l);
    buffer+=2;
    memcpy(msg->eSM_MessageContainer.v, buffer, msg->eSM_MessageContainer.l);
    buffer+=msg->eSM_MessageContainer.l;
    nas_msg(NAS_DEBUG, 0, "DEC : eSM_MessageContainer len = %u, v = %#x", msg->eSM_MessageContainer.l, msg->eSM_MessageContainer.v[0]);

    /*Currently not working. The TV IE's are common on the NAS specifications and are not considered on 3gpp 24.007, Clause 11.2.4*/
    /*dec_nonImperativeIE(msg->optionals, buffer, size);*/
    /*union nAS_ie_member *optionals[17];*/
}

void dec_AttachComplete(AttachComplete_t *msg, uint8_t *buffer, uint32_t size){

    /*ESM_MessageContainer*/
    memcpy(&(msg->eSM_MessageContainer.l), buffer, 2);
    msg->eSM_MessageContainer.l = htons(msg->eSM_MessageContainer.l);
    buffer+=2;
    memcpy(msg->eSM_MessageContainer.v, buffer, msg->eSM_MessageContainer.l);
    buffer+=msg->eSM_MessageContainer.l;
    nas_msg(NAS_DEBUG, 0, "DEC : eSM_MessageContainer len = %u, v = %#x", msg->eSM_MessageContainer.l, msg->eSM_MessageContainer.v[0]);

    /*Currently not working. The TV IE's are common on the NAS specifications and are not considered on 3gpp 24.007, Clause 11.2.4*/
    /*dec_nonImperativeIE(msg->optionals, buffer, size);*/
    /*union nAS_ie_member *optionals[17];*/
}

void dec_AttachReject(AttachReject_t *msg, uint8_t *buffer, uint32_t size){

    /*eMMCause*/
    memcpy(&(msg->eMMCause), buffer,1);
    buffer++;
    nas_msg(NAS_DEBUG, 0, "DEC : eMMCause = %#x", msg->eMMCause);

}

void dec_AttachRequest(AttachRequest_t *msg, uint8_t *buffer, uint32_t size){
    uint8_t numOp=0;
    nas_msg(NAS_DEBUG, 0, "Enter");

    /*EPSAttachType and NASKeySetId*/
    msg->ePSAttachType.v = (*buffer)&0x0F;
    msg->nASKeySetId.v = ((*buffer)&0xF0)>>4;
    buffer++;
    size--;
    nas_msg(NAS_DEBUG, 0, "DEC : ePSAttachType = %#x, DEC : nASKeySetId = %#x", msg->ePSAttachType.v, msg->nASKeySetId.v);

    /*EPSMobileId*/
    msg->ePSMobileId.l = *buffer;
    memcpy(msg->ePSMobileId.v, ++buffer, msg->ePSMobileId.l);
    buffer+=msg->ePSMobileId.l;
    size -= (msg->ePSMobileId.l+1);
    nas_msg(NAS_DEBUG, 0, "DEC : ePSMobileId len = %u, v = %#x%x", msg->ePSMobileId.l, msg->ePSMobileId.v[0], msg->ePSMobileId.v[1]);

    /*UENetworkCapability*/
    msg->uENetworkCapability.l = *buffer;
    memcpy(msg->uENetworkCapability.v, ++buffer, msg->uENetworkCapability.l);
    buffer+=msg->uENetworkCapability.l;
    size -= (msg->uENetworkCapability.l+1);
    nas_msg(NAS_DEBUG, 0, "DEC : uENetworkCapability len = %u, v = %#x", msg->uENetworkCapability.l, msg->uENetworkCapability.v[0]);

    /*ESM_MessageContainer*/
    memcpy(&(msg->eSM_MessageContainer.l), buffer, 2);
    msg->eSM_MessageContainer.l = ntohs(msg->eSM_MessageContainer.l);
    buffer+=2;
    size-=2;
    memcpy(msg->eSM_MessageContainer.v, buffer, msg->eSM_MessageContainer.l);
    buffer+=msg->eSM_MessageContainer.l;
    size-=msg->eSM_MessageContainer.l;
    nas_msg(NAS_DEBUG, 0, "DEC : eSM_MessageContainer len = %u, v = %#x", msg->eSM_MessageContainer.l, msg->eSM_MessageContainer.v[0]);

    /*Currently not working. The TV IE's are common on the NAS specifications and are not considered on 3gpp 24.007, Clause 11.2.4*/
    /*dec_nonImperativeIE(msg->optionals, buffer, size);*/
    /*union nAS_ie_member *optionals[17];*/

    if(size == 0)
        return;

    /*Optionals*/
    /* Old P-TMSI signature */
    dec_NASOpt_tv_t3(msg->optionals, &buffer, &size, &numOp, 0x19, 4);
    /* Additional GUTI */
    dec_NASOpt_tlv(msg->optionals, &buffer, &size, &numOp, 0x50);
    /* Last visited registered TAI */
    dec_NASOpt_tv_t3(msg->optionals, &buffer, &size, &numOp, 0x52, 6);
    /* DRX Parameter */
    dec_NASOpt_tv_t3(msg->optionals, &buffer, &size, &numOp, 0x5C, 3);
    /* MS Network Capability */
    dec_NASOpt_tlv(msg->optionals, &buffer, &size, &numOp, 0x31);
    /* Old LAI */
    dec_NASOpt_tv_t3(msg->optionals, &buffer, &size, &numOp, 0x13, 6);
    /* TMSI status */
    dec_NASOpt_tv_t1(msg->optionals, &buffer, &size, &numOp, 0x9, 0x1);
    /* MS Classmark 2 */
    dec_NASOpt_tlv(msg->optionals, &buffer, &size, &numOp, 0x11);
    /* MS Classmark 3 */
    dec_NASOpt_tlv(msg->optionals, &buffer, &size, &numOp, 0x20);
    /* Supported Codecs */
    dec_NASOpt_tlv(msg->optionals, &buffer, &size, &numOp, 0x40);
    /* Additional update type */
    dec_NASOpt_tv_t1(msg->optionals, &buffer, &size, &numOp, 0xF, 0x1);
    /* Voice Domain Preference & UE's usage setting*/
    dec_NASOpt_tlv(msg->optionals, &buffer, &size, &numOp, 0x5D);
    /* Device properties */
    dec_NASOpt_tv_t1(msg->optionals, &buffer, &size, &numOp, 0xD, 0x1);
    /* Old GUTI type */
    dec_NASOpt_tv_t1(msg->optionals, &buffer, &size, &numOp, 0xE, 0x1);
    /* MS network feature support */
    dec_NASOpt_tv_t1(msg->optionals, &buffer, &size, &numOp, 0xC, 0x1);
    /* TMSI based NRI container*/
    dec_NASOpt_tlv(msg->optionals, &buffer, &size, &numOp, 0x10);
    /* T3324 value*/
    dec_NASOpt_tlv(msg->optionals, &buffer, &size, &numOp, 0x6A);
    /* T3412 extended value*/
    dec_NASOpt_tlv(msg->optionals, &buffer, &size, &numOp, 0x5E);
}

void dec_DetachRequestUEOrig(DetachRequestUEOrig_t *msg, uint8_t *buffer, uint32_t size){
    /*detachType and NASKeySetId*/
    msg->detachType.v = (*buffer)&0x0F;
    msg->nASKeySetId.v = ((*buffer)&0xF0)>>4;
    buffer++;
    size--;
    nas_msg(NAS_DEBUG, 0, "DEC : detachType = %#x", msg->detachType.v);

    /*EPSMobileId*/
    msg->ePSMobileId.l = *buffer;
    memcpy(msg->ePSMobileId.v, ++buffer, msg->ePSMobileId.l);
    size-=(msg->ePSMobileId.l+1);
    buffer+=msg->ePSMobileId.l;
    nas_msg(NAS_DEBUG, 0, "DEC : ePSMobileId len = %u, v = %#x%x", msg->ePSMobileId.l, msg->ePSMobileId.v[0], msg->ePSMobileId.v[1]);

}

void dec_DetachRequestUETerm(DetachRequestUETerm_t *msg, uint8_t *buffer, uint32_t size){
    /*detachType and spare*/
    msg->detachType.v = (*buffer)&0x0F;
    buffer++;
    size--;
    nas_msg(NAS_DEBUG, 0, "DEC : detachType = %#x", msg->detachType.v);
}

void dec_TrackingAreaUpdateAccept(TrackingAreaUpdateAccept_t *msg, uint8_t *buffer, uint32_t size){
    /*detachType and spare*/
    msg->ePSUpdateResult.v = (*buffer)&0x0F;
    buffer++;
    size--;
    nas_msg(NAS_DEBUG, 0, "DEC : ePSUpdateResult = %#x", msg->ePSUpdateResult.v);
}

void dec_TrackingAreaUpdateReject(TrackingAreaUpdateReject_t *msg, uint8_t *buffer, uint32_t size){
    /*eMMCause*/
    memcpy(&(msg->eMMCause), buffer,1);
    buffer++;
    size--;
    nas_msg(NAS_DEBUG, 0, "DEC : eMMCause = %#x", msg->eMMCause);
}

void dec_TrackingAreaUpdateRequest(TrackingAreaUpdateRequest_t *msg, uint8_t *buffer, uint32_t size){
    uint8_t numOp=0;
    /*ePSUpdateType and NASKeySetId*/
    msg->ePSUpdateType.v = (*buffer)&0x0F;
    msg->nASKeySetId.v = ((*buffer)&0xF0)>>4;
    buffer++;
    size--;
    nas_msg(NAS_DEBUG, 0, "DEC : ePSUpdateType = %#x, DEC : nASKeySetId = %#x",
            msg->ePSUpdateType.v, msg->nASKeySetId.v);

    /*oldGUTI - EPSMobileId*/
    msg->oldGUTI.l = *buffer;
    memcpy(msg->oldGUTI.v, ++buffer, msg->oldGUTI.l);
    size-=(msg->oldGUTI.l+1);
    buffer+=msg->oldGUTI.l;
    nas_msg(NAS_DEBUG, 0, "DEC : oldGUTI len = %u, v = %#x%x",
            msg->oldGUTI.l, msg->oldGUTI.v[0], msg->oldGUTI.v[1]);

    if(size == 0)
        return;

    /*Optionals*/
    /* Non-current native NAS key set identifier */
    dec_NASOpt_tv_t1(msg->optionals, &buffer, &size, &numOp, 0xB, 0xF);
    /* GPRS ciphering key sequence number*/
    dec_NASOpt_tv_t1(msg->optionals, &buffer, &size, &numOp, 0x8, 0x7);
    /* Old P-TMSI signature*/
    dec_NASOpt_tv_t3(msg->optionals, &buffer, &size, &numOp, 0x19, 4);
    /* Additional GUTI */
    dec_NASOpt_tlv(msg->optionals, &buffer, &size, &numOp, 0x50);
    /* Nonce UE*/
    dec_NASOpt_tv_t3(msg->optionals, &buffer, &size, &numOp, 0x55, 5);
    /* UE network Capability*/
    dec_NASOpt_tlv(msg->optionals, &buffer, &size, &numOp, 0x58);
    /* Last visited registered TAI */
    dec_NASOpt_tv_t3(msg->optionals, &buffer, &size, &numOp, 0x52, 6);
    /* DRX Parameter */
    dec_NASOpt_tv_t3(msg->optionals, &buffer, &size, &numOp, 0x5C, 3);
    /* UE radio capability information update needed */
    dec_NASOpt_tv_t1(msg->optionals, &buffer, &size, &numOp, 0xA, 0x1);
    /* EPS bearer context status*/
    dec_NASOpt_tlv(msg->optionals, &buffer, &size, &numOp, 0x57);
    /* MS Network Capability */
    dec_NASOpt_tlv(msg->optionals, &buffer, &size, &numOp, 0x31);
    /* Old LAI */
    dec_NASOpt_tv_t3(msg->optionals, &buffer, &size, &numOp, 0x13, 6);
    /* TMSI status */
    dec_NASOpt_tv_t1(msg->optionals, &buffer, &size, &numOp, 0x9, 0x1);
    /* MS Classmark 2 */
    dec_NASOpt_tlv(msg->optionals, &buffer, &size, &numOp, 0x11);
    /* MS Classmark 3 */
    dec_NASOpt_tlv(msg->optionals, &buffer, &size, &numOp, 0x20);
    /* Supported Codecs */
    dec_NASOpt_tlv(msg->optionals, &buffer, &size, &numOp, 0x40);
    /* Additional update type */
    dec_NASOpt_tv_t1(msg->optionals, &buffer, &size, &numOp, 0xF, 0x1);
    /* Voice Domain Preference & UE's usage setting*/
    dec_NASOpt_tlv(msg->optionals, &buffer, &size, &numOp, 0x5D);
    /* Old GUTI type */
    dec_NASOpt_tv_t1(msg->optionals, &buffer, &size, &numOp, 0xE, 0x1);
    /* Device properties */
    dec_NASOpt_tv_t1(msg->optionals, &buffer, &size, &numOp, 0xD, 0x1);
    /* MS network feature support */
    dec_NASOpt_tv_t1(msg->optionals, &buffer, &size, &numOp, 0xC, 0x1);
    /* TMSI based NRI container*/
    dec_NASOpt_tlv(msg->optionals, &buffer, &size, &numOp, 0x10);
    /* T3324 value*/
    dec_NASOpt_tlv(msg->optionals, &buffer, &size, &numOp, 0x6A);
    /* T3412 extended value*/
    dec_NASOpt_tlv(msg->optionals, &buffer, &size, &numOp, 0x5E);
}

/* ******************** end EMM ******************** */

/* ********************** ESM ********************** */

void dec_ActivateDefaultEPSBearerContextAccept(ActivateDefaultEPSBearerContextAccept_t *msg, uint8_t *buffer, uint32_t size){

}

void dec_ActivateDefaultEPSBearerContextReject(ActivateDefaultEPSBearerContextReject_t *msg, uint8_t *buffer, uint32_t size){
    /*eSMCause*/
    memcpy(&(msg->eSMCause), buffer,1);
    buffer++;
    size--;
    nas_msg(NAS_DEBUG, 0, "DEC : eSMCause = %#x", msg->eSMCause);
}

void dec_ActivateDefaultEPSBearerContextRequest(ActivateDefaultEPSBearerContextRequest_t *msg, uint8_t *buffer, uint32_t size){
    /*ePSQoS*/
    msg->ePSQoS.l = *buffer;
    memcpy(msg->ePSQoS.v, ++buffer, msg->ePSQoS.l);
    size-=(msg->ePSQoS.l+1);
    buffer+=msg->ePSQoS.l;
    nas_msg(NAS_DEBUG, 0, "DEC : ePSQoS len = %u, v = %#x%x", msg->ePSQoS.l, msg->ePSQoS.v[0], msg->ePSQoS.v[1]);

    /*accessPointName*/
    msg->accessPointName.l = *buffer;
    memcpy(msg->ePSQoS.v, ++buffer, msg->accessPointName.l);
    size-=(msg->accessPointName.l+1);
    buffer+=msg->accessPointName.l;
    nas_msg(NAS_DEBUG, 0, "DEC : accessPointName len = %u, v = %#x%x", msg->accessPointName.l, msg->accessPointName.v[0], msg->accessPointName.v[1]);

    /*ePSQoS*/
    msg->pDNAddress.l = *buffer;
    memcpy(msg->pDNAddress.v, ++buffer, msg->pDNAddress.l);
    size-=(msg->ePSQoS.l+1);
    buffer+=msg->ePSQoS.l;
    nas_msg(NAS_DEBUG, 0, "DEC : pDNAddress len = %u, v = %#x%x", msg->pDNAddress.l, msg->pDNAddress.v[0], msg->pDNAddress.v[1]);

}


void dec_ActivateDedicatedEPSBearerContextAccept(ActivateDedicatedEPSBearerContextAccept_t *msg, uint8_t *buffer, uint32_t size){

}

void dec_ActivateDedicatedEPSBearerContextReject(ActivateDedicatedEPSBearerContextReject_t *msg, uint8_t *buffer, uint32_t size){
    /*eSMCause*/
    memcpy(&(msg->eSMCause), buffer,1);
    buffer++;
    size--;
    nas_msg(NAS_DEBUG, 0, "DEC : eSMCause = %#x", msg->eSMCause);
}

void dec_ActivateDedicatedEPSBearerContextRequest(ActivateDedicatedEPSBearerContextRequest_t *msg, uint8_t *buffer, uint32_t size){

    /*linkedEPSIdentity*/
    msg->linkedEPSId.v = (*buffer)&0x0F;
    buffer++;
    size--;
    nas_msg(NAS_DEBUG, 0, "DEC : linkedEPSId = %#x", msg->linkedEPSId.v);

    /*ePSQoS*/
    msg->ePSQoS.l = *buffer;
    memcpy(msg->ePSQoS.v, ++buffer, msg->ePSQoS.l);
    buffer-=(msg->ePSQoS.l+1);
    buffer+=msg->ePSQoS.l;
    nas_msg(NAS_DEBUG, 0, "DEC : ePSQoS len = %u, v = %#x%x", msg->ePSQoS.l, msg->ePSQoS.v[0], msg->ePSQoS.v[1]);

    /*TFT*/
    msg->tFT.l = *buffer;
    memcpy(msg->tFT.v, ++buffer, msg->tFT.l);
    size-=(msg->tFT.l+1);
    buffer+=msg->tFT.l;
    nas_msg(NAS_DEBUG, 0, "DEC : TFT len = %u, v = %#x%x", msg->tFT.l, msg->tFT.v[0], msg->tFT.v[1]);
}


void dec_ModifyEPSBearerContextAccept(ModifyEPSBearerContextAccept_t *msg, uint8_t *buffer, uint32_t size){

}

void dec_ModifyEPSBearerContextReject(ModifyEPSBearerContextReject_t *msg, uint8_t *buffer, uint32_t size){
    /*eSMCause*/
    memcpy(&(msg->eSMCause), buffer,1);
    buffer++;
    size--;
    nas_msg(NAS_DEBUG, 0, "DEC : eSMCause = %#x", msg->eSMCause);
}

void dec_ModifyEPSBearerContextRequest(ModifyEPSBearerContextRequest_t *msg, uint8_t *buffer, uint32_t size){

}


void dec_DeactivateEPSBearerContextAccept(DeactivateEPSBearerContextAccept_t *msg, uint8_t *buffer, uint32_t size){

}

void dec_DeactivateEPSBearerContextRequest(DeactivateEPSBearerContextRequest_t *msg, uint8_t *buffer, uint32_t size){
    /*eSMCause*/
    memcpy(&(msg->eSMCause), buffer,1);
    buffer++;
    size--;
    nas_msg(NAS_DEBUG, 0, "DEC : eSMCause = %#x", msg->eSMCause);
}


void dec_PDNConnectivityReject(PDNConnectivityReject_t *msg, uint8_t *buffer, uint32_t size){
    /*eSMCause*/
    memcpy(&(msg->eSMCause), buffer,1);
    buffer++;
    size--;
    nas_msg(NAS_DEBUG, 0, "DEC : eSMCause = %#x", msg->eSMCause);
}

void dec_PDNConnectivityRequest(PDNConnectivityRequest_t *msg, uint8_t *buffer, uint32_t size){
    uint8_t numOp=0;
    /*Request type*/
    msg->requestType.v = (*buffer)&0x0F;
    /* PDN type*/
    msg->PDNType.v = ((*buffer)&0xF0)>>4;
    buffer++;
    size--;
    nas_msg(NAS_DEBUG, 0, "DEC : requestType = %#x, DEC : PDNType = %#x",
            msg->requestType.v, msg->PDNType.v);

    if(size == 0)
        return;

    /*Optionals*/
    /*ESM information transfer flag*/
    dec_NASOpt_tv_t1(msg->optionals, &buffer, &size, &numOp, 0xD, 0x1);
    /*Access point name*/
    dec_NASOpt_tlv(msg->optionals, &buffer, &size, &numOp, 0x28);
    /*Protocol configuration options*/
    dec_NASOpt_tlv(msg->optionals, &buffer, &size, &numOp, 0x27);
    /* Device properties */
    dec_NASOpt_tv_t1(msg->optionals, &buffer, &size, &numOp, 0xC, 0x1);
}


void dec_PDNDisconnectReject(PDNDisconnectReject_t *msg, uint8_t *buffer, uint32_t size){
    /*eSMCause*/
    memcpy(&(msg->eSMCause), buffer,1);
    buffer++;
    size--;
    nas_msg(NAS_DEBUG, 0, "DEC : eSMCause = %#x", msg->eSMCause);
}

void dec_PDNDisconnectRequest(PDNDisconnectRequest_t *msg, uint8_t *buffer, uint32_t size){
    /*linkedEPSIdentity*/
    msg->linkedEPSId.v = (*buffer)&0x0F;
    buffer++;
    size--;
    nas_msg(NAS_DEBUG, 0, "DEC : linkedEPSId = %#x", msg->linkedEPSId.v);
}

void dec_BearerResourceAllocationReject(BearerResourceAllocationReject_t *msg, uint8_t *buffer, uint32_t size){
    /*eSMCause*/
    memcpy(&(msg->eSMCause), buffer,1);
    buffer++;
    size--;
    nas_msg(NAS_DEBUG, 0, "DEC : eSMCause = %#x", msg->eSMCause);
}

void dec_BearerResourceAllocationRequest(BearerResourceAllocationRequest_t *msg, uint8_t *buffer, uint32_t size){
    /*linkedEPSIdentity*/
    msg->linkedEPSId.v = (*buffer)&0x0F;
    buffer++;
    size--;
    nas_msg(NAS_DEBUG, 0, "DEC : linkedEPSId = %#x", msg->linkedEPSId.v);

    /*trafficFlowAggregate*/
    msg->trafficFlowAggregate.l = *buffer;
    memcpy(msg->trafficFlowAggregate.v, ++buffer, msg->trafficFlowAggregate.l);
    size-=(msg->trafficFlowAggregate.l+1);
    buffer+=msg->trafficFlowAggregate.l;
    nas_msg(NAS_DEBUG, 0, "DEC : trafficFlowAggregate len = %u, v = %#x%x", msg->trafficFlowAggregate.l, msg->trafficFlowAggregate.v[0], msg->trafficFlowAggregate.v[1]);

    /*requiredTrafficFlowQoS*/
    msg->requiredTrafficFlowQoS.l = *buffer;
    memcpy(msg->requiredTrafficFlowQoS.v, ++buffer, msg->requiredTrafficFlowQoS.l);
    size-=(msg->requiredTrafficFlowQoS.l+1);
    buffer+=msg->requiredTrafficFlowQoS.l;
    nas_msg(NAS_DEBUG, 0, "DEC : requiredTrafficFlowQoS len = %u, v = %#x%x",
            msg->requiredTrafficFlowQoS.l, msg->requiredTrafficFlowQoS.v[0], msg->requiredTrafficFlowQoS.v[1]);

}


void dec_BearerResourceModificationReject(BearerResourceModificationReject_t *msg, uint8_t *buffer, uint32_t size){
    /*eSMCause*/
    memcpy(&(msg->eSMCause), buffer,1);
    buffer++;
    size--;
    nas_msg(NAS_DEBUG, 0, "DEC : eSMCause = %#x", msg->eSMCause);
}

void dec_BearerResourceModificationRequest(BearerResourceModificationRequest_t *msg, uint8_t *buffer, uint32_t size){
    /*EPSBearerIdForPacketFilter*/
    msg->EPSBearerIdForPacketFilter.v = (*buffer)&0x0F;
    buffer++;
    size--;
    nas_msg(NAS_DEBUG, 0, "DEC : linkedEPSId = %#x", msg->EPSBearerIdForPacketFilter.v);

    /*trafficFlowAggregate*/
    msg->trafficFlowAggregate.l = *buffer;
    memcpy(msg->trafficFlowAggregate.v, ++buffer, msg->trafficFlowAggregate.l);
    size-=(msg->trafficFlowAggregate.l+1);
    buffer+=msg->trafficFlowAggregate.l;
    nas_msg(NAS_DEBUG, 0, "DEC : trafficFlowAggregate len = %u, v = %#x%x", msg->trafficFlowAggregate.l, msg->trafficFlowAggregate.v[0], msg->trafficFlowAggregate.v[1]);
}

/* ******************** end ESM ******************** */

/*@brief Decode Non Imperative IE's (not working)
 *
 * 3gpp 24.007, Clause 11.2.4
 * Currently not working. The TV IE's are common on the NAS specifications and are not considered on 3gpp 24.007, Clause 11.2.4*/
void dec_nonImperativeIE(union nAS_ie_member ** optionals, uint8_t *buffer, uint32_t size){
    uint32_t i=0;
    uint16_t len;
    uint8_t *end, iei;
    end = buffer + size;
    while(buffer<end){
        iei = *buffer;
        nas_msg(NAS_DEBUG, 0, "test iei = %#x &buffer = %p, end = %p\n", iei, &buffer, &end);
        optionals[i] = (union nAS_ie_member *)buffer;
        buffer++;
        if((iei &0x80) == 0){
            if((iei &0x78) == 0x78){
                /* TLV-E formatted type 6*/
                memcpy(&len, buffer, 2);
                len = htons(len);
                buffer+=(len+2);
            }else{
                /* TLV*/
                len = *buffer;
                buffer+=(len+1);
            }
        }
        /*Else Single byte IE*/
        i++;
    }

};

void dec_NASOpt_tlv(union nAS_ie_member *optionals, uint8_t **buffer, uint32_t *size,
                    uint8_t *optIndex, uint32_t type){
    ie_tlv_t4_t *temp;
    uint8_t opT;
    if(*size==0){
        return;
    }
    opT = **buffer;
    if(opT == type){
        temp = (ie_tlv_t4_t *)*buffer;
        memcpy(optionals + *optIndex, temp, temp->l+2);
        (*buffer) += (temp->l+2);
        (*size)   -= (temp->l+2);
        (*optIndex)++;
    }
}

void dec_NASOpt_tv_t3(union nAS_ie_member *optionals, uint8_t **buffer, uint32_t *size,
                      uint8_t *optIndex, uint32_t type, uint32_t len){
    uint8_t opT;
    if(*size==0){
        return;
    }
    opT = **buffer;
    if(opT == type){
        memcpy(optionals + *optIndex, *buffer, len);
        (*buffer)+=len;
        (*size)-=len;
        (*optIndex)++;
    }
}

void dec_NASOpt_tv_t1(union nAS_ie_member *optionals, uint8_t **buffer, uint32_t *size,
                      uint8_t *optIndex, uint32_t type, uint32_t valueMask){
    uint8_t opT;
    if(*size==0){
        return;
    }
    opT = **buffer;
    if((opT&0xF0) == (type<<4)){
        optionals[*optIndex].iei = opT&(0xF0|valueMask);
        (*buffer)++;
        (*size)--;
        (*optIndex)++;
    }
}
