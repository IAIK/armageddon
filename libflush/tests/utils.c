/* See LICENSE file for license and copyright information */

#ifdef FIU_ENABLE
#include <fiu.h>
#include <fiu-control.h>
#endif

#include <check.h>
#include <signal.h>

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

#if HAVE_PAGEMAP_ACCESS == 1
START_TEST(test_get_physical_address) {
  int x;
  libflush_get_physical_address(libflush_session, (uintptr_t) &x);
} END_TEST

#ifdef FIU_ENABLE
START_TEST(test_get_physical_address_fault_injection) {
  int x;

  libflush_get_physical_address(libflush_session, (uintptr_t) &x);
} END_TEST
#endif

START_TEST(test_get_pagemap_entry) {
  int x;
  libflush_get_pagemap_entry(libflush_session, (uintptr_t) &x);
} END_TEST
#endif

START_TEST(test_bind_to_cpu) {
  libflush_bind_to_cpu(0);
} END_TEST

Suite*
suite_utils(void)
{
  TCase* tcase = NULL;
  Suite* suite = suite_create("utils");

  tcase = tcase_create("basic");
  tcase_add_checked_fixture(tcase, setup_session, teardown_session);
#if HAVE_PAGEMAP_ACCESS == 1
  tcase_add_test(tcase, test_get_physical_address);
#ifdef FIU_ENABLE
  fiu_enable("posix/io/rw/pread", 1, NULL, 0);
  tcase_add_test_raise_signal(tcase, test_get_physical_address_fault_injection, SIGABRT);
  fiu_disable("posix/io/rw/pread");
#endif
  tcase_add_test(tcase, test_get_pagemap_entry);
#endif
  tcase_add_test(tcase, test_bind_to_cpu);
  suite_add_tcase(suite, tcase);

  return suite;
}
