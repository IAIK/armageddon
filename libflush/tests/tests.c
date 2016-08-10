/* See LICENSE file for license and copyright information */

#include <check.h>

#ifdef FIU_ENABLE
#include <fiu.h>
#endif

Suite* suite_session(void);
Suite* suite_timing(void);
Suite* suite_memory(void);
Suite* suite_eviction(void);
Suite* suite_prefetch(void);
Suite* suite_utils(void);

int main(void)
{
  /* initialize libfiu */
#ifdef FIU_ENABLE
  fiu_init(0);
#endif

  /* setup test suite */
  SRunner* suite_runner = srunner_create(NULL);
  srunner_set_fork_status(suite_runner, CK_NOFORK);

  srunner_add_suite(suite_runner, suite_session());
  srunner_add_suite(suite_runner, suite_timing());
  srunner_add_suite(suite_runner, suite_memory());
  srunner_add_suite(suite_runner, suite_eviction());
  srunner_add_suite(suite_runner, suite_prefetch());
  srunner_add_suite(suite_runner, suite_utils());

  int number_failed = 0;
  srunner_run_all(suite_runner, CK_ENV);
  number_failed += srunner_ntests_failed(suite_runner);
  srunner_free(suite_runner);

  return (number_failed == 0) ? 0 : 1;
}
