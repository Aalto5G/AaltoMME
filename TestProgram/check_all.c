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

/**@file   check_all.c
 * @author Vicent Ferrer
 * @date   March, 2013
 * @brief  Definition of Check tests
 *
 * Definition and implementation of test cases using check testing framework */

#include <check.h>

/* EXIT_SUCCESS , EXIT_FAILURE*/
#include <stdlib.h>

/*Test Suites*/
#include "check_libgtp.h"
#include "check_nas.h"
#include "check_rt_per.h"
#include "check_libs1ap.h"

int
main (void)
{
    int number_failed;

    SRunner *sr = srunner_create (gtplib_suite());
    srunner_add_suite (sr, nas_suite ());
    srunner_add_suite (sr, per_suite ());
    srunner_add_suite (sr, s1p_suite ());

    srunner_set_xml (sr, "check_tests.xml");
    srunner_run_all (sr, CK_NORMAL);
    number_failed = srunner_ntests_failed (sr);
    srunner_print(sr, CK_VERBOSE);
    srunner_free (sr);
    return (number_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}


