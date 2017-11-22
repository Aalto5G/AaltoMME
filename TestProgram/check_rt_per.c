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

/**@file   check_libgtp.c
 * @author Vicent Ferrer
 * @date   March, 2013
 * @brief  Definition of Check tests
 *
 * Definition and implementation of test cases using check testing framework */

#include "check_rt_per.h"

#include "rt_per_bin.h"

/* strcmp*/
#include <string.h>
#include <stdlib.h>


START_TEST (decode_octet_string_tc)
{
    struct BinaryData bytes;
    uint8_t data[3];
    uint8_t s[2];
    memset(s, 0, 2);
    bytes.data=data;
    bytes.length=24;
    bytes.pos=2;
    data[0]=0x00;
    data[1]=0x0f;
    data[2]=0x40;
    decode_octet_string(s, &bytes, 2);
    ck_assert_msg(s[0] == 0x00, "First byte not decoded correctly %x != 0x00", s[0]);
    ck_assert_msg(s[1] == 0x3d, "Second byte not decoded correctly %x != 0x3d", s[1]);

}
END_TEST


START_TEST (decode_getbits_num16_pos2_length18_tc)
{
    struct BinaryData bytes, res;
    uint8_t data[3];

    res.data = malloc(2);
    memset(res.data, 0, 2);

    bytes.data=data;
    bytes.length=18;
    bytes.pos=2;
    data[0]=0x00;
    data[1]=0x0f;
    data[2]=0x40;
    getbits(&res, &bytes, 16);
    ck_assert_msg(res.data[0] == 0x00, "First byte not decoded correctly %x != 0x00", res.data[0]);
    ck_assert_msg(res.data[1] == 0x3d, "Second byte not decoded correctly %x != 0x3d", res.data[1]);

    free(res.data);

}
END_TEST

START_TEST (decode_getbits_num2_pos1_length18_tc)
{
    struct BinaryData bytes, res;
    uint8_t data[3];

    res.length=0;
    res.pos=0;
    res.data = malloc(2);
    memset(res.data, 0, 2);

    bytes.data=data;
    bytes.length=18;
    bytes.pos=1;
    data[0]=0x20;

    getbits(&res, &bytes, 2);
    ck_assert_msg(res.data[0] == 0x40, "Not decoded correctly %x != 0x01", res.data[0]);

    free(res.data);

}
END_TEST

START_TEST (decode_getbits_pos0len24num20_tc)
{
    uint32_t res=0;
    struct BinaryData bytes, bitstr;
    uint8_t data[3], buffer[4];;
    data[0]=0x12;
    data[1]=0x34;
    data[2]=0xf6;
    bytes.pos=0;
    bytes.length=24;
    bytes.data=data;

    bitstr.data=buffer;

    getbits(&bitstr, &bytes, 20);

    ck_assert_msg(buffer[0]==0x12 && buffer[1] == 0x34 && buffer[2]==0xf0, "buffer %.2x%.2x%.2x != 123450\n", buffer[0], buffer[1], buffer[2]);

    //ck_assert_msg(res == 0x00002000, "%#.8x != 0x00002000", res);

}
END_TEST

START_TEST (decode_getbits_num4_pos7_length12_tc)
{
    uint32_t res=0;
    struct BinaryData bytes, bitstr;
    uint8_t data[2], buffer[4];
    data[0]=0x78;
    data[1]=0xa1;
    bytes.pos=7;
    bytes.length=12;
    bytes.data=data;

    bitstr.data=buffer;

    getbits(&bitstr, &bytes, 4);

    ck_assert_msg(buffer[0]==0x50 , "buffer %.2#x != 0x50", buffer[0]);

    //ck_assert_msg(res == 0x00002000, "%#.8x != 0x00002000", res);

}
END_TEST

START_TEST (decode_getbits_pos1len16num8_tc)
{
    uint32_t res=0;
    struct BinaryData bytes, bitstr;
    uint8_t data[2] = {0x00, 0x80};
    uint8_t buffer[4];
    bytes.pos=1;
    bytes.length=16;
    bytes.data=data;

    bitstr.data=buffer;

    /*printf("decode_getbits_pos1len16num8_tc\n");*/
    getbits(&bitstr, &bytes, 8);

    ck_assert_msg(buffer[0]==0x01 , "buffer %#.2x != 0x80", buffer[0]);


    //ck_assert_msg(res == 0x00002000, "%#.8x != 0x00002000", res);

}
END_TEST

START_TEST (decode_getbits_pos2len24num16_tc)
{
    uint32_t res=0;
    struct BinaryData bytes, bitstr;
    uint8_t data[3] = {0x00, 0x0f, 0x40};
    uint8_t buffer[4];
    bytes.pos=2;
    bytes.length=24;
    bytes.data=data;

    bitstr.data=buffer;

    /*printf("decode_getbits_pos2len24num16_tc\n");*/
    getbits(&bitstr, &bytes, 16);

    ck_assert_msg(buffer[0]==0x00 && buffer[1]==0x3d, "(0x%.2x, 0x%.2x)!=(0x00, 0x3d)", buffer[0], buffer[1]);


    //ck_assert_msg(res == 0x00002000, "%#.8x != 0x00002000", res);

}
END_TEST

START_TEST (decode_getbits_pos4len8num4_tc)
{
    uint32_t res=0;
    struct BinaryData bytes, bitstr;
    uint8_t data[1] = {0x05};
    uint8_t buffer[1];
    bytes.pos=4;
    bytes.length=8;
    bytes.data=data;

    bitstr.data=buffer;

    /*printf("decode_getbits_pos2len24num16_tc\n");*/
    getbits(&bitstr, &bytes, 4);

    ck_assert_msg(buffer[0]==0x50, "0x%.2x!=0x50", buffer[0]);


    //ck_assert_msg(res == 0x00002000, "%#.8x != 0x00002000", res);

}
END_TEST


START_TEST (decode_constrained_number1_tc)
{
    struct BinaryData bytes;
    uint8_t data[1];
    uint64_t c;
    data[0]=0x20;
    bytes.pos=1;
    bytes.length=3;
    bytes.data=data;
    c = decode_constrained_number(&bytes, 0, 2 );
    ck_assert_msg(c == 1, "%u != 1", c);

}
END_TEST

START_TEST (decode_constrained_number2_tc)
{
    struct BinaryData bytes;
    uint8_t data[2]={0x78, 0xa1};
    uint64_t c;
    bytes.pos=7;
    bytes.length=12;
    bytes.data=data;
    c = decode_constrained_number(&bytes, 0, 15);

    ck_assert_msg(c == 5, "%u != 5", c);

}
END_TEST

START_TEST (decode_bit_string_tc)
{
    uint32_t res=0;
    struct BinaryData bytes;
    uint8_t data[3] = {0x12, 0x34, 0x56};

    bytes.pos=0;
    bytes.length=24;
    bytes.data=data;

    res = decode_bit_string(&bytes, 20);

    ck_assert_msg(res == 0x00012345, "%#.8x != 0x00012345", res);

}
END_TEST

Suite *
per_suite (void)
{
    Suite *s = suite_create ("per_lib_Messages");

    /* Version test case */
    TCase *tc_string = tcase_create ("decode_octet_string_tc");
    TCase *tc_getbits = tcase_create ("getbits_tc");
    TCase *tc_decode_constrained_number = tcase_create ("decode_constrained_number_tc");
    TCase *tc_decode_bit_string = tcase_create ("decode_bit_string_tc");

    tcase_add_test (tc_string, decode_octet_string_tc);

    tcase_add_test (tc_getbits, decode_getbits_num16_pos2_length18_tc);
    tcase_add_test (tc_getbits, decode_getbits_num2_pos1_length18_tc);
    tcase_add_test (tc_getbits, decode_getbits_pos0len24num20_tc);
    tcase_add_test (tc_getbits, decode_getbits_num4_pos7_length12_tc);
    tcase_add_test (tc_getbits, decode_getbits_pos1len16num8_tc);
    tcase_add_test (tc_getbits, decode_getbits_pos2len24num16_tc);
    tcase_add_test (tc_getbits, decode_getbits_pos4len8num4_tc);

    tcase_add_test (tc_decode_constrained_number, decode_constrained_number1_tc);
    tcase_add_test (tc_decode_constrained_number, decode_constrained_number2_tc);
    tcase_add_test (tc_decode_bit_string, decode_bit_string_tc);

    suite_add_tcase (s, tc_string);
    suite_add_tcase (s, tc_getbits);
    suite_add_tcase (s, tc_decode_constrained_number);
    suite_add_tcase (s, tc_decode_bit_string);


    return s;
}
