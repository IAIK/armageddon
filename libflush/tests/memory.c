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

START_TEST(test_flush) {
  int x;
  libflush_flush(libflush_session, &x);
} END_TEST

START_TEST(test_flush_time) {
  int x;
  libflush_flush_time(libflush_session, &x);
} END_TEST

#if HAVE_PAGEMAP_ACCESS == 1
START_TEST(test_evict) {
  int x;
  libflush_evict(libflush_session, &x);
} END_TEST
#endif

#if HAVE_PAGEMAP_ACCESS == 1
START_TEST(test_evict_time) {
  int x;
  libflush_evict_time(libflush_session, &x);
} END_TEST
#endif

START_TEST(test_access_memory) {
  int x;
  libflush_access_memory(&x);
} END_TEST

START_TEST(test_reload_address) {
  int x;
  libflush_reload_address(libflush_session, &x);
} END_TEST

START_TEST(test_reload_address_and_flush) {
  int x;
  libflush_reload_address_and_flush(libflush_session, &x);
} END_TEST

#if HAVE_PAGEMAP_ACCESS == 1
START_TEST(test_reload_address_and_evict) {
  int x;
  libflush_reload_address_and_evict(libflush_session, &x);
} END_TEST
#endif

START_TEST(test_memory_barrier) {
  libflush_memory_barrier();
} END_TEST

Suite*
suite_memory(void)
{
  TCase* tcase = NULL;
  Suite* suite = suite_create("memory");

  tcase = tcase_create("flush");
  tcase_add_checked_fixture(tcase, setup_session, teardown_session);
  tcase_add_test(tcase, test_flush);
  tcase_add_test(tcase, test_flush_time);
  suite_add_tcase(suite, tcase);

#if HAVE_PAGEMAP_ACCESS == 1
  tcase = tcase_create("evict");
  tcase_add_checked_fixture(tcase, setup_session, teardown_session);
  tcase_add_test(tcase, test_evict);
  tcase_add_test(tcase, test_evict_time);
  suite_add_tcase(suite, tcase);
#endif

  tcase = tcase_create("access");
  tcase_add_checked_fixture(tcase, setup_session, teardown_session);
  tcase_add_test(tcase, test_access_memory);
  suite_add_tcase(suite, tcase);

  tcase = tcase_create("reload");
  tcase_add_checked_fixture(tcase, setup_session, teardown_session);
  tcase_add_test(tcase, test_reload_address);
  tcase_add_test(tcase, test_reload_address_and_flush);
#if HAVE_PAGEMAP_ACCESS == 1
  tcase_add_test(tcase, test_reload_address_and_evict);
#endif
  suite_add_tcase(suite, tcase);

  tcase = tcase_create("barrier");
  tcase_add_checked_fixture(tcase, setup_session, teardown_session);
  tcase_add_test(tcase, test_memory_barrier);
  suite_add_tcase(suite, tcase);

  return suite;
}
