/* Gearman server and library
 * Copyright (C) 2008 Brian Aker, Eric Day
 * All rights reserved.
 *
 * Use and distribution licensed under the BSD license.  See
 * the COPYING file in the parent directory for full text.
 */

#include "config.h"

#if defined(NDEBUG)
# undef NDEBUG
#endif

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <libgearman/gearman.h>

#include "test.h"
#include "test_gearmand.h"
#include "test_worker.h"

#define CLIENT_TEST_PORT 32123

static test_return_t init_test(void *not_used __attribute__((unused)))
{
  gearman_state_st gear;
  gearman_state_st *gear_ptr;

  gear_ptr= gearman_state_create(&gear, NULL);
  test_truth(gear.options.allocated == false);
  test_truth(gear_ptr->options.allocated == false);
  test_truth(gear.options.dont_track_packets == false);
  test_truth(gear.options.non_blocking == false);
  test_truth(gear.options.stored_non_blocking == false);
  test_truth(gear_ptr == &gear);

  gearman_state_free(&gear);
  test_truth(gear.options.allocated == false);

  return TEST_SUCCESS;
}

static test_return_t allocation_test(void *not_used __attribute__((unused)))
{
  gearman_state_st *gear_ptr;

  gear_ptr= gearman_state_create(NULL, NULL);

  test_truth(gear_ptr->options.allocated == true);
  test_truth(gear_ptr->options.dont_track_packets == false);
  test_truth(gear_ptr->options.non_blocking == false);
  test_truth(gear_ptr->options.stored_non_blocking == false);

  gearman_state_free(gear_ptr);

  return TEST_SUCCESS;
}


static test_return_t clone_test(void *not_used __attribute__((unused)))
{
  gearman_state_st gear;
  gearman_state_st *gear_ptr;

  gear_ptr= gearman_state_create(&gear, NULL);
  test_truth(gear_ptr);
  test_truth(gear_ptr == &gear);

  /* All null? */
  {
    gearman_state_st *gear_clone;
    gear_clone= gearman_state_clone(NULL, NULL);
    test_truth(gear_clone);
    test_truth(gear_clone->options.allocated);
    gearman_state_free(gear_clone);
  }

  /* Can we init from null? */
  {
    gearman_state_st *gear_clone;
    gear_clone= gearman_state_clone(NULL, &gear);
    test_truth(gear_clone);
    test_truth(gear_clone->options.allocated);

    { // Test all of the flags
      test_truth(gear_clone->options.dont_track_packets == gear_ptr->options.dont_track_packets);
      test_truth(gear_clone->options.non_blocking == gear_ptr->options.non_blocking);
      test_truth(gear_clone->options.stored_non_blocking == gear_ptr->options.stored_non_blocking);
    }
    test_truth(gear_clone->verbose == gear_ptr->verbose);
    test_truth(gear_clone->con_count == gear_ptr->con_count);
    test_truth(gear_clone->packet_count == gear_ptr->packet_count);
    test_truth(gear_clone->pfds_size == gear_ptr->pfds_size);
    test_truth(gear_clone->last_errno == gear_ptr->last_errno);
    test_truth(gear_clone->timeout == gear_ptr->timeout);
    test_truth(gear_clone->con_list == gear_ptr->con_list);
    test_truth(gear_clone->packet_list == gear_ptr->packet_list);
    test_truth(gear_clone->pfds == gear_ptr->pfds);
    test_truth(gear_clone->log_fn == gear_ptr->log_fn);
    test_truth(gear_clone->log_context == gear_ptr->log_context);
    test_truth(gear_clone->event_watch_fn == gear_ptr->event_watch_fn);
    test_truth(gear_clone->event_watch_context == gear_ptr->event_watch_context);
    test_truth(gear_clone->workload_malloc_fn == gear_ptr->workload_malloc_fn);
    test_truth(gear_clone->workload_malloc_context == gear_ptr->workload_malloc_context);
    test_truth(gear_clone->workload_free_fn == gear_ptr->workload_free_fn);
    test_truth(gear_clone->workload_free_context == gear_ptr->workload_free_context);

    gearman_state_free(gear_clone);
  }

  /* Can we init from struct? */
  {
    gearman_state_st declared_clone;
    gearman_state_st *gear_clone;
    memset(&declared_clone, 0 , sizeof(gearman_state_st));
    gear_clone= gearman_state_clone(&declared_clone, NULL);
    test_truth(gear_clone);
    test_truth(gear_clone->options.allocated == false);
    gearman_state_free(gear_clone);
  }

  /* Can we init from struct? */
  {
    gearman_state_st declared_clone;
    gearman_state_st *gear_clone;
    memset(&declared_clone, 0 , sizeof(gearman_state_st));
    gear_clone= gearman_state_clone(&declared_clone, &gear);
    test_truth(gear_clone);
    test_truth(gear_clone->options.allocated == false);
    gearman_state_free(gear_clone);
  }

  return TEST_SUCCESS;
}

static test_return_t set_timout_test(void *not_used __attribute__((unused)))
{
  gearman_state_st gear;
  gearman_state_st *gear_ptr;
  int time_data;

  gear_ptr= gearman_state_create(&gear, NULL);
  test_truth(gear_ptr);
  test_truth(gear_ptr == &gear);
  test_truth(gear_ptr->options.allocated == false);

  time_data= gearman_timeout(gear_ptr);
  test_truth (time_data == -1); // Current default

  gearman_set_timeout(gear_ptr, 20);
  time_data= gearman_timeout(gear_ptr);
  test_truth (time_data == 20); // Current default

  gearman_set_timeout(gear_ptr, 10);
  time_data= gearman_timeout(gear_ptr);
  test_truth (time_data == 10); // Current default

  test_truth(gear_ptr == &gear); // Make sure noting got slipped in :)
  gearman_state_free(gear_ptr);

  return TEST_SUCCESS;
}

static test_return_t basic_error_test(void *not_used __attribute__((unused)))
{
  gearman_state_st gear;
  gearman_state_st *gear_ptr;
  const char *error;
  int error_number;

  gear_ptr= gearman_state_create(&gear, NULL);
  test_truth(gear_ptr);
  test_truth(gear_ptr == &gear);
  test_truth(gear_ptr->options.allocated == false);

  error= gearman_state_error(gear_ptr);
  test_truth(error == NULL);

  error_number= gearman_state_errno(gear_ptr);
  test_truth(error_number == 0);
  
  test_truth(gear_ptr == &gear); // Make sure noting got slipped in :)
  gearman_state_free(gear_ptr);

  return TEST_SUCCESS;
}

test_st state_test[] ={
  {"init", 0, init_test },
  {"allocation", 0, allocation_test },
  {"clone_test", 0, clone_test },
  {"set_timeout", 0, set_timout_test },
  {"basic_error", 0, basic_error_test },
  {0, 0, 0}
};


collection_st collection[] ={
  {"state", 0, 0, state_test},
  {0, 0, 0, 0}
};

void get_world(world_st *world)
{
  world->collections= collection;
}