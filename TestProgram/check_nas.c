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

#include "check_nas.h"

#include "NAS.h"

/* strcmp*/
#include <string.h>

/**/
START_TEST (test_IE_lv_t6_enc)
{
    uint8_t *pointer, *p, esm[300], res[300];

    p=res;
    memset(res, 0, 300);

    pointer=esm;
    *pointer=0x12;
    pointer++;
    *pointer=0x34;
    pointer++;
    *pointer=0x56;
    pointer++;
    *pointer=0x78;
    pointer++;
    ck_assert_msg(pointer-esm == 4, "len not correct");
    ck_assert_msg( &p != res && p == res, "The pointer to the array is incorrect");
    nasIe_lv_t6(&p, esm, pointer-esm);
    //printf("test &res %#x, res %#x, esm = %#x, len = %u\n", &res, res, esm, pointer-esm);
    //printf("test\n");
    ck_assert_msg(((ie_lv_t6_t*)res)->l == htons(4), "lenght not stored correctly: %#.4x",((ie_lv_t6_t*)res)->l);
    //printf("test\n");

    //printf("%x %x %x %x %x %x %x\n", res[0], res[1], res[2], res[3], res[4], res[5], res[6]);
}
END_TEST

void encodeAPN(uint8_t *res, uint8_t *name){
    uint8_t i, aPN_len, lable_len=0, *tmp;
    aPN_len = strlen(name);
    tmp = name;
    for(i=0; i<aPN_len; i++){
        if(name[i]=='.'){
            *res = lable_len;
            res++;
            memcpy(res, tmp, lable_len);
            res +=lable_len;
            tmp +=(lable_len+1);
            lable_len = 0;
        }else{
            lable_len++;
        }
    }
    *res = lable_len;
    res++;
    memcpy(res, tmp, lable_len);
}

START_TEST (temp)
{
    uint8_t res[100], aPN[]="test.apn";
    encodeAPN(res, aPN);
    ck_assert_msg(res[0]==4, "Error on 1st len");
    ck_assert_msg(res[1]=='t', "1Error on 1st character");
    ck_assert_msg(res[2]=='e', "1Error on 2nd character");
    ck_assert_msg(res[3]=='s', "1Error on 3rd character");
    ck_assert_msg(res[4]=='t', "1Error on 4th character");
    ck_assert_msg(res[5]==3, "Error on 2nd len value");
    ck_assert_msg(res[6]=='a', "2Error on 1st character");
    ck_assert_msg(res[7]=='p', "2Error on 2nd character");
    ck_assert_msg(res[8]=='n', "2Error en 3rd character");

    //printf("test %s", res);

}
END_TEST


Suite *
nas_suite (void)
{
    Suite *s = suite_create ("nas_lib_Messages");

    /* Version test case */
    TCase *tc_IE_lv_t6 = tcase_create ("IE lv type 6");
    TCase *tc_tmp = tcase_create ("temp");
    tcase_add_test (tc_IE_lv_t6, test_IE_lv_t6_enc);
    tcase_add_test (tc_tmp, temp);
    suite_add_tcase (s, tc_IE_lv_t6);
    suite_add_tcase (s, tc_tmp);


    return s;
}
