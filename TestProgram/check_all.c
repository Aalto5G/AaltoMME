/* This application was initially developed as a Final Project by
 *     Vicent Ferrer Guasch (vicent.ferrerguasch@aalto.fi)
 * under the supervision of,
 *     Jukka Manner (jukka.manner@aalto.fi)
 *     Jose Costa-Requena (jose.costa@aalto.fi)
 * in AALTO University and partially funded by EIT ICT labs.
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


