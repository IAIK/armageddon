/* See LICENSE file for license and copyright information */

#include <check.h>

#include <libflush.h>

libflush_session_t* libflush_session;

static void setup_session(void) {
  fail_unless(libflush_init(&libflush_session, NULL) == true);
  fail_unless(libflush_session != NULL);
}

static void teardown_session(void) {
  fail_unless(libflush_terminate(libflush_session) == true);
  libflush_session = NULL;
}

START_TEST(test_prefetch) {
  int x;
  libflush_prefetch(libflush_session, &x);
} END_TEST

START_TEST(test_prefetch_time) {
  int x;
  libflush_prefetch_time(libflush_session, &x);
} END_TEST

Suite*
suite_prefetch(void)
{
  TCase* tcase = NULL;
  Suite* suite = suite_create("prefetch");

  tcase = tcase_create("basic");
  tcase_add_checked_fixture(tcase, setup_session, teardown_session);
  tcase_add_test(tcase, test_prefetch);
  tcase_add_test(tcase, test_prefetch_time);
  suite_add_tcase(suite, tcase);

  return suite;
}
