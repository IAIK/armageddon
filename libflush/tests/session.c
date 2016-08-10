/* See LICENSE file for license and copyright information */

#ifdef FIU_ENABLE
#include <fiu.h>
#include <fiu-control.h>
#endif

#include <check.h>

#include <libflush.h>

START_TEST(test_session_init) {
  libflush_session_t* libflush_session;

  /* Invalid arguments */
  fail_unless(libflush_init(NULL, NULL) == false);

  /* Valid arguments */
  fail_unless(libflush_init(&libflush_session, NULL) == true);
  fail_unless(libflush_session != NULL);
  fail_unless(libflush_terminate(libflush_session) == true);

  /* Fault injection */
#ifdef FIU_ENABLE
  fiu_enable("libc/mm/calloc", 1, NULL, 0);
  fail_unless(libflush_init(&libflush_session, NULL) == false);
  fiu_disable("libc/mm/calloc");

#if HAVE_PAGEMAP_ACCESS == 1
  fiu_enable("posix/io/oc/open", 1, NULL, 0);
  fail_unless(libflush_init(&libflush_session, NULL) == false);
  fiu_disable("posix/io/oc/open");
#endif
#endif
} END_TEST

START_TEST(test_session_terminate) {
  libflush_session_t* libflush_session;

  /* Invalid arguments */
  fail_unless(libflush_terminate(NULL) == false);

  /* Valid arguments */
  fail_unless(libflush_init(&libflush_session, NULL) == true);
  fail_unless(libflush_session != NULL);
  fail_unless(libflush_terminate(libflush_session) == true);
} END_TEST

Suite*
suite_session(void)
{
  TCase* tcase = NULL;
  Suite* suite = suite_create("session");

  tcase = tcase_create("basic");
  tcase_add_test(tcase, test_session_init);
  tcase_add_test(tcase, test_session_terminate);
  suite_add_tcase(suite, tcase);

  return suite;
}
