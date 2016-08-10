/* See LICENSE file for license and copyright information */

#include <check.h>

#include <libflush.h>
#include <eviction/eviction.h>
#include <eviction/configuration.h>

libflush_session_t* libflush_session;

static void setup_session(void) {
  fail_unless(libflush_init(&libflush_session, NULL) == true);
  fail_unless(libflush_session != NULL);
}

static void teardown_session(void) {
  fail_unless(libflush_terminate(libflush_session) == true);
  libflush_session = NULL;
}

START_TEST(test_eviction_init) {
  /* Invalid arguments */
  fail_unless(libflush_eviction_init(NULL, NULL) == false);
} END_TEST

START_TEST(test_eviction_terminate) {
  /* Invalid arguments */
  fail_unless(libflush_eviction_terminate(NULL) == false);
} END_TEST

#if HAVE_PAGEMAP_ACCESS == 1
START_TEST(test_eviction_evict) {
  int x;
  libflush_eviction_evict(libflush_session, &x);
} END_TEST

START_TEST(test_eviction_evict_cached) {
  int x;
  libflush_eviction_evict(libflush_session, &x);
  libflush_eviction_evict(libflush_session, &x);
} END_TEST

START_TEST(test_eviction_evict_two) {
  int x;
  int y;
  libflush_eviction_evict(libflush_session, &x);
  libflush_eviction_evict(libflush_session, &y);
} END_TEST

START_TEST(test_eviction_evict_exhaust) {
  size_t number_of_addresses = ADDRESS_CACHE_SIZE + 1;
  int addresses[number_of_addresses];
  for (size_t i = 0; i < number_of_addresses; i++) {
    libflush_eviction_evict(libflush_session, &(addresses[i]));
  }
} END_TEST
#endif

Suite*
suite_eviction(void)
{
  TCase* tcase = NULL;
  Suite* suite = suite_create("eviction");

  tcase = tcase_create("basic");
  tcase_add_checked_fixture(tcase, setup_session, teardown_session);
  tcase_add_test(tcase, test_eviction_init);
  tcase_add_test(tcase, test_eviction_terminate);
  suite_add_tcase(suite, tcase);

#if HAVE_PAGEMAP_ACCESS == 1
  tcase = tcase_create("evict");
  tcase_add_checked_fixture(tcase, setup_session, teardown_session);
  tcase_add_test(tcase, test_eviction_evict);
  tcase_add_test(tcase, test_eviction_evict_cached);
  tcase_add_test(tcase, test_eviction_evict_two);
  tcase_add_test(tcase, test_eviction_evict_exhaust);
  suite_add_tcase(suite, tcase);
#endif

  return suite;
}
