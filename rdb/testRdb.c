/*
 * See file ../COPYRIGHT for copying and redistribution conditions.
 */
#include "ldmconfig.h"

#ifndef _XOPEN_SOURCE
#   define _XOPEN_SOURCE 500
#endif

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <CUnit/CUnit.h>
#include <CUnit/Basic.h>
#include <syslog.h>
#include <sys/stat.h>
#include <ulog.h>
#include <log.h>

#include "rdblib.h"

#define TESTDIR_PATH    "/tmp/testRdb"


static int
setup(void)
{
    int         status = -1;    /* failure */

    status = 0;

    return status;
}


static int
teardown(void)
{
    return 0;
}


static void
test_rdbOpen(void)
{
    RdbStatus   status;

    status = rdbOpen(TESTDIR_PATH, 1);
    if (status)
        log_log(LOG_ERR);
    CU_ASSERT_EQUAL_FATAL(status, 0);
    status = rdbClose();
    CU_ASSERT_EQUAL(status, 0);

    status = rdbOpen(TESTDIR_PATH, 0);
    if (status)
        log_log(LOG_ERR);
    CU_ASSERT_EQUAL(status, 0);
    status = rdbClose();
    CU_ASSERT_EQUAL(status, 0);

    status = rdbOpen("/foo", 0);
    log_clear();
    CU_ASSERT_EQUAL(status, RDB_DBERR);

    status = rdbOpen("/foo", 1);
    log_clear();
    CU_ASSERT_EQUAL(status, RDB_DBERR);
}

static void
test_rdbString(void)
{
    RdbStatus   status;
    char*       value;

    status = rdbOpen(TESTDIR_PATH, 1);
    if (status)
        log_log(LOG_ERR);
    CU_ASSERT_EQUAL_FATAL(status, 0);

    status = rdbPutString("foo key", "foo value 0");
    if (status)
        log_log(LOG_ERR);
    CU_ASSERT_EQUAL(status, 0);

    status = rdbPutString("foo key", "foo value");
    if (status)
        log_log(LOG_ERR);
    CU_ASSERT_EQUAL(status, 0);

    status = rdbGetString("foo key", &value, "overridden default");
    if (status)
        log_log(LOG_ERR);
    CU_ASSERT_EQUAL(status, 0);
    CU_ASSERT_STRING_EQUAL(value, "foo value");
    free(value);

    status = rdbGetString("foo key/sub key", &value, "overridden default");
    if (status)
        log_log(LOG_ERR);
    CU_ASSERT_EQUAL(status, 0);
    CU_ASSERT_STRING_EQUAL(value, "foo value");
    free(value);

    status = rdbGetString("bar key", &value, "default bar value");
    if (status)
        log_log(LOG_ERR);
    CU_ASSERT_EQUAL(status, 0);
    CU_ASSERT_STRING_EQUAL(value, "default bar value");
    free(value);

    status = rdbGetString("bof key", &value, NULL);
    if (status)
        log_log(LOG_ERR);
    CU_ASSERT_EQUAL(status, 0);
    CU_ASSERT_PTR_NULL(value);

    status = rdbClose();
    if (status)
        log_log(LOG_ERR);
    CU_ASSERT_EQUAL(status, 0);
}

static void
test_rdbInt(void)
{
    RdbStatus   status;
    int         value;

    status = rdbOpen(TESTDIR_PATH, 1);
    if (status)
        log_log(LOG_ERR);
    CU_ASSERT_EQUAL_FATAL(status, 0);

    status = rdbPutInt("fooInt key", 1);
    if (status)
        log_log(LOG_ERR);
    CU_ASSERT_EQUAL(status, 0);

    status = rdbPutInt("fooInt key", 2);
    if (status)
        log_log(LOG_ERR);
    CU_ASSERT_EQUAL(status, 0);

    status = rdbGetInt("fooInt key", &value, 3);
    if (status)
        log_log(LOG_ERR);
    CU_ASSERT_EQUAL(status, 0);
    CU_ASSERT_EQUAL(value, 2);

    status = rdbGetInt("fooInt key/subkey", &value, 3);
    if (status)
        log_log(LOG_ERR);
    CU_ASSERT_EQUAL(status, 0);
    CU_ASSERT_EQUAL(value, 2);

    status = rdbGetInt("barInt key", &value, 4);
    if (status)
        log_log(LOG_ERR);
    CU_ASSERT_EQUAL(status, 0);
    CU_ASSERT_EQUAL(value, 4);

    status = rdbClose();
    if (status)
        log_log(LOG_ERR);
    CU_ASSERT_EQUAL(status, 0);
}


static void
test_rdbDelete(void)
{
    RdbStatus   status;
    char*       string;
    int         value;

    status = rdbOpen(TESTDIR_PATH, 1);
    if (status)
        log_log(LOG_ERR);
    CU_ASSERT_EQUAL_FATAL(status, 0);

    status = rdbPutString("string key", "string value");
    if (status)
        log_log(LOG_ERR);
    CU_ASSERT_EQUAL(status, 0);

    status = rdbGetString("string key", &string, NULL);
    if (status)
        log_log(LOG_ERR);
    CU_ASSERT_EQUAL(status, 0);
    CU_ASSERT_STRING_EQUAL(string, "string value");
    free(string);

    status = rdbDelete("string key");
    if (status)
        log_log(LOG_ERR);
    CU_ASSERT_EQUAL(status, 0);

    status = rdbGetString("string key", &string, "default string value");
    if (status && RDB_NOENTRY != status)
        log_log(LOG_ERR);
    CU_ASSERT_STRING_EQUAL(string, "default string value");
    free(string);

    status = rdbPutInt("int key", -1);
    if (status)
        log_log(LOG_ERR);
    CU_ASSERT_EQUAL(status, 0);

    status = rdbGetInt("int key", &value, 4);
    if (status)
        log_log(LOG_ERR);
    CU_ASSERT_EQUAL(status, 0);
    CU_ASSERT_EQUAL(value, -1);

    status = rdbDelete("int key");
    if (status)
        log_log(LOG_ERR);
    CU_ASSERT_EQUAL(status, 0);

    status = rdbGetInt("int key", &value, 4);
    if (status)
        log_log(LOG_ERR);
    CU_ASSERT_EQUAL(status, 0);
    CU_ASSERT_EQUAL(value, 4);

    status = rdbDelete("nosuch key");
    if (status && RDB_NOENTRY != status)
        log_log(LOG_ERR);
    CU_ASSERT_EQUAL(status, RDB_NOENTRY);

    status = rdbClose();
    if (status)
        log_log(LOG_ERR);
    CU_ASSERT_EQUAL(status, 0);
}


static void
test_rdbRemove(void)
{
    RdbStatus   status;

    status = rdbRemove(TESTDIR_PATH);
    if (status)
        log_log(LOG_ERR);
    CU_ASSERT_EQUAL(status, 0);
}

/*
    CU_ASSERT_EQUAL(ut_map_unit_to_name(unit, "name", UT_ASCII), UT_SUCCESS);
    CU_ASSERT_PTR_NOT_NULL_FATAL(unit);
    CU_ASSERT_PTR_NULL(ut_new_base_unit(NULL));
    CU_ASSERT_STRING_EQUAL(buf, "1");
    CU_ASSERT_TRUE_FATAL(nchar > 0);
    CU_ASSERT_TRUE(n > 0);
*/

int
main(
    const int           argc,
    const char* const*  argv)
{
    int exitCode = EXIT_FAILURE;

    if (-1 == mkdir(TESTDIR_PATH, S_IRWXU)) {
        (void)fprintf(stderr, "Couldn't create test directory \"%s\": %s\n",
            TESTDIR_PATH, strerror(errno));
    }
    else {
        if (CUE_SUCCESS == CU_initialize_registry()) {
            CU_Suite*       testSuite = CU_add_suite(__FILE__, setup, teardown);

            if (NULL != testSuite) {
                CU_ADD_TEST(testSuite, test_rdbOpen);
                CU_ADD_TEST(testSuite, test_rdbString);
                CU_ADD_TEST(testSuite, test_rdbInt);
                CU_ADD_TEST(testSuite, test_rdbDelete);
                CU_ADD_TEST(testSuite, test_rdbRemove);

                if (-1 == openulog(argv[0], 0, LOG_LOCAL0, "-")) {
                    (void)fprintf(stderr, "Couldn't open logging system\n");
                }
                else {
                    if (CU_basic_run_tests() == CUE_SUCCESS) {
                        if (0 == CU_get_number_of_failures())
                            exitCode = EXIT_SUCCESS;
                    }
                }
            }

            CU_cleanup_registry();
        }                               /* CUnit registery allocated */

        if (-1 == rmdir(TESTDIR_PATH))
            (void)fprintf(stderr, "Couldn't delete test directory \"%s\": %s\n",
                TESTDIR_PATH, strerror(errno));
    }                                   /* test-directory created */

    return exitCode;
}
