/* This application was initially developed as a Final Project by
 *     Vicent Ferrer Guasch (vicent.ferrerguasch@aalto.fi)
 * under the supervision of,
 *     Jukka Manner (jukka.manner@aalto.fi)
 *     Jose Costa-Requena (jose.costa@aalto.fi)
 * in AALTO University and partially funded by EIT ICT labs.
 */

/**@file   check_libgtp.c
 * @author Vicent Ferrer
 * @date   March, 2013
 * @brief  Definition of Check tests
 *
 * Definition and implementation of test cases using check testing framework */

#include "check_libs1ap.h"

#include "S1AP.h"
#include "rt_per_bin.h"

/* strcmp*/
#include <string.h>
#include <stdlib.h>


START_TEST (dec_Global_ENB_ID_tc)
{
    Global_ENB_ID_t *id;
    S1AP_PROTOCOL_IES_t *ie;
    struct BinaryData bytes;
    uint8_t info[]={0x00, 0x42, 0xf4, 0x70, 0x00, 0x00, 0x00, 0x20};
    bytes.length = 8*8;
    bytes.pos=0;
    bytes.data=info;
    ie = newProtocolIE();
    ck_assert_msg(ie!=NULL, "IE not allocated");

    dec_Global_ENB_ID (ie, &bytes);
    id = (Global_ENB_ID_t*)ie->value;
    ck_assert_msg(id != NULL, "Value not decoded" );

    ck_assert_msg(id->eNBid->id.macroENB_ID == 2, "macroID incorrect %#x != 2", id->eNBid->id.macroENB_ID);

    //id->showIE(id);

    ie->freeIE(ie);
    //ck_assert_msg(id == NULL, "Value not deallocated" );
    //ck_assert_msg(ie == NULL, "IE not deallocated" );

}
END_TEST

START_TEST (dec_ENBname_tc)
{
    ENBname_t *name;
    S1AP_PROTOCOL_IES_t *ie;
    struct BinaryData bytes;
    uint8_t info[]={0x00, 0x80, 0x30, 0x32};
    bytes.length = 4*8;
    bytes.pos=0;
    bytes.data=info;
    ie = newProtocolIE();
    ck_assert_msg(ie!=NULL, "IE not allocated");

    dec_ENBname (ie, &bytes);
    name = (ENBname_t*)ie->value;
    ck_assert_msg(name != NULL, "Value not decoded" );

    ck_assert_msg(name->name[0] == '0' && name->name[1] == '2', "ENBname = %#x %#x != 0x30 0x32, %s", name->name[0], name->name[1], name->name);

    //id->showIE(id);

    ie->freeIE(ie);
    //ck_assert_msg(id == NULL, "Value not deallocated" );
    //ck_assert_msg(ie == NULL, "IE not deallocated" );

}
END_TEST


START_TEST (enc_MME_UE_S1AP_ID_tc)
{
    MME_UE_S1AP_ID_t *mmeUEId;
    S1AP_PROTOCOL_IES_t *ie;
    struct BinaryData bytes;
    uint8_t data[50];

    memset(data, 0, 50);
    bytes.data = data;
    bytes.length = 0;
    bytes.offset = data;
    bytes.pos = 0;

    ie = newProtocolIE();

    ck_assert_msg(ie != NULL, "S1AP: Coudn't allocate new Protocol IE structure");

    mmeUEId = new_MME_UE_S1AP_ID();

    ck_assert_msg(mmeUEId != NULL, "S1AP: Coudn't allocate mmeUEId");

    ie->value = (void*)mmeUEId;
    ie->showValue = mmeUEId->showIE;
    ie->freeValue = mmeUEId->freeIE;
    ie->id = id_MME_UE_S1AP_ID;
    ie->presence = mandatory;
    ie->criticality = reject;

    mmeUEId->mme_id = 256;

    enc_MME_UE_S1AP_ID(&bytes, ie);

    ck_assert_msg((data[0]&0xc0) == 0x40 , "IE value length incorrect %#x != 0x40 ", data[0]&0xc0);
    ck_assert_msg(data[1] == 0x01 && data[2] == 0x00, "IE value incorrect ", data[4]&0xc0);
    ck_assert_msg(bytes.length == 24, "Incorrect length encode %u != 24", bytes.length);
}
END_TEST

Suite *
s1p_suite (void)
{
    Suite *s = suite_create ("s1ap_lib");

    /* Version test case */
    TCase *tc_Global_ENB_ID = tcase_create ("Global_ENB_ID_tc");
    TCase *tc_ENBname = tcase_create ("ENBname_tc");
    TCase *tc_UE_MME_ID = tcase_create ("MME_UE_S1AP_ID_tc");



    tcase_add_test (tc_Global_ENB_ID, dec_Global_ENB_ID_tc);
    tcase_add_test (tc_ENBname, dec_ENBname_tc);
    tcase_add_test (tc_UE_MME_ID, enc_MME_UE_S1AP_ID_tc);

    suite_add_tcase (s, tc_Global_ENB_ID);
    suite_add_tcase (s, tc_ENBname);
    suite_add_tcase (s, tc_UE_MME_ID);



    return s;
}
