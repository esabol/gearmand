/*  vim:expandtab:shiftwidth=2:tabstop=2:smarttab:
 *
 *  Gearmand client and server library.
 *
 *  Copyright (C) 2026 The gearmand Project Contributors
 *  All rights reserved.
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions are
 *  met:
 *
 *      * Redistributions of source code must retain the above copyright
 *  notice, this list of conditions and the following disclaimer.
 *
 *      * Redistributions in binary form must reproduce the above
 *  copyright notice, this list of conditions and the following disclaimer
 *  in the documentation and/or other materials provided with the
 *  distribution.
 *
 *      * The names of its contributors may not be used to endorse or
 *  promote products derived from this software without specific prior
 *  written permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 *  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 *  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 *  A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 *  OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 *  SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 *  LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 *  DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 *  THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 *  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 *  OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "gear_config.h"
#include <libtest/test.hpp>

using namespace libtest;

#include <cerrno>
#include <csignal>
#include <cstring>
#if !defined(WIN32)
#include <sys/wait.h>
#include <unistd.h>
#endif

#include <libgearman/gearman.h>
#include <libgearman-1.0/client.h>
#include <libgearman-1.0/worker.h>

#define GEARMAN_CORE
#define private public
#include "libgearman/common.h"
#include "libgearman/connection.hpp"
#undef private
#include "libgearman/packet.hpp"
#include "libgearman/universal.hpp"

static volatile sig_atomic_t signal_seen= 0;

static void signal_wait_handler(int)
{
  signal_seen= 1;
}

static gearman_connection_st *create_wait_test_connection(gearman_universal_st& universal,
                                                          int socket_fd)
{
  gearman_connection_st *connection=
    new (std::nothrow) gearman_connection_st(universal, nullptr, GEARMAN_DEFAULT_TCP_PORT_STRING);
  ASSERT_TRUE(connection);
  connection->fd= socket_fd;
  connection->state= GEARMAN_CON_UNIVERSAL_CONNECTED;
  connection->set_events(POLLIN);
  return connection;
}

static gearman_return_t wait_with_signal(bool stop_wait_on_signal)
{
#if defined(WIN32)
  return GEARMAN_NOT_FLUSHING;
#else
  gearman_universal_st universal;
  int sockets[2];
  ASSERT_EQ(0, socketpair(AF_UNIX, SOCK_STREAM, 0, sockets));

  gearman_connection_st *connection= create_wait_test_connection(universal, sockets[0]);
  ASSERT_TRUE(connection);

  gearman_universal_set_timeout(universal, 2000);
  gearman_universal_set_option(universal, GEARMAN_UNIVERSAL_STOP_WAIT_ON_SIGNAL, stop_wait_on_signal);

  struct sigaction new_action;
  struct sigaction old_action;
  memset(&new_action, 0, sizeof(new_action));
  memset(&old_action, 0, sizeof(old_action));
  new_action.sa_handler= signal_wait_handler;
  sigemptyset(&new_action.sa_mask);
  new_action.sa_flags= 0;
  ASSERT_EQ(0, sigaction(SIGUSR1, &new_action, &old_action));

  signal_seen= 0;

  pid_t pid= fork();
  ASSERT_NEQ(-1, pid);
  if (pid == 0)
  {
    usleep(100000);
    kill(getppid(), SIGUSR1);
    usleep(100000);
    ssize_t write_length= write(sockets[1], "1", 1);
    (void)write_length;
    _exit(0);
  }

  gearman_return_t ret= gearman_wait(universal);

  ASSERT_EQ(0, sigaction(SIGUSR1, &old_action, nullptr));
  ASSERT_EQ(pid, waitpid(pid, nullptr, 0));

  close(sockets[1]);
  delete connection;
  gearman_universal_free(universal);

  ASSERT_TRUE(signal_seen);

  return ret;
#endif
}

static test_return_t client_stop_wait_on_signal_option_test(void *)
{
  gearman_client_st *client= gearman_client_create(nullptr);
  ASSERT_TRUE(client);
  ASSERT_FALSE(client->impl()->universal.is_stop_wait_on_signal());
  ASSERT_FALSE(gearman_client_has_option(client, GEARMAN_CLIENT_STOP_WAIT_ON_SIGNAL));
  ASSERT_EQ(0, gearman_client_options(client) & GEARMAN_CLIENT_STOP_WAIT_ON_SIGNAL);

  gearman_client_add_options(client, GEARMAN_CLIENT_STOP_WAIT_ON_SIGNAL);
  ASSERT_TRUE(client->impl()->universal.is_stop_wait_on_signal());
  ASSERT_TRUE(gearman_client_has_option(client, GEARMAN_CLIENT_STOP_WAIT_ON_SIGNAL));
  ASSERT_TRUE(gearman_client_options(client) & GEARMAN_CLIENT_STOP_WAIT_ON_SIGNAL);

  gearman_client_remove_options(client, GEARMAN_CLIENT_STOP_WAIT_ON_SIGNAL);
  ASSERT_FALSE(client->impl()->universal.is_stop_wait_on_signal());
  ASSERT_FALSE(gearman_client_has_option(client, GEARMAN_CLIENT_STOP_WAIT_ON_SIGNAL));
  ASSERT_EQ(0, gearman_client_options(client) & GEARMAN_CLIENT_STOP_WAIT_ON_SIGNAL);

  gearman_client_set_options(client, GEARMAN_CLIENT_STOP_WAIT_ON_SIGNAL);
  ASSERT_TRUE(client->impl()->universal.is_stop_wait_on_signal());
  ASSERT_TRUE(gearman_client_options(client) & GEARMAN_CLIENT_STOP_WAIT_ON_SIGNAL);

  gearman_client_set_options(client, gearman_client_options_t());
  ASSERT_FALSE(client->impl()->universal.is_stop_wait_on_signal());
  ASSERT_EQ(0, gearman_client_options(client) & GEARMAN_CLIENT_STOP_WAIT_ON_SIGNAL);

  gearman_client_free(client);

  return TEST_SUCCESS;
}

static test_return_t worker_stop_wait_on_signal_option_test(void *)
{
  gearman_worker_st *worker= gearman_worker_create(nullptr);
  ASSERT_TRUE(worker);
  ASSERT_FALSE(worker->impl()->universal.is_stop_wait_on_signal());
  ASSERT_EQ(0, gearman_worker_options(worker) & GEARMAN_WORKER_STOP_WAIT_ON_SIGNAL);

  gearman_worker_add_options(worker, GEARMAN_WORKER_STOP_WAIT_ON_SIGNAL);
  ASSERT_TRUE(worker->impl()->universal.is_stop_wait_on_signal());
  ASSERT_TRUE(gearman_worker_options(worker) & GEARMAN_WORKER_STOP_WAIT_ON_SIGNAL);

  gearman_worker_remove_options(worker, GEARMAN_WORKER_STOP_WAIT_ON_SIGNAL);
  ASSERT_FALSE(worker->impl()->universal.is_stop_wait_on_signal());
  ASSERT_EQ(0, gearman_worker_options(worker) & GEARMAN_WORKER_STOP_WAIT_ON_SIGNAL);

  gearman_worker_set_options(worker, GEARMAN_WORKER_STOP_WAIT_ON_SIGNAL);
  ASSERT_TRUE(worker->impl()->universal.is_stop_wait_on_signal());
  ASSERT_TRUE(gearman_worker_options(worker) & GEARMAN_WORKER_STOP_WAIT_ON_SIGNAL);

  gearman_worker_set_options(worker, gearman_worker_options_t());
  ASSERT_FALSE(worker->impl()->universal.is_stop_wait_on_signal());
  ASSERT_EQ(0, gearman_worker_options(worker) & GEARMAN_WORKER_STOP_WAIT_ON_SIGNAL);

  gearman_worker_free(worker);

  return TEST_SUCCESS;
}

static test_return_t gearman_wait_ignores_signal_by_default_test(void *)
{
#if defined(WIN32)
  return TEST_SKIPPED;
#else
  ASSERT_EQ(GEARMAN_SUCCESS, wait_with_signal(false));
  return TEST_SUCCESS;
#endif
}

static test_return_t gearman_wait_stops_on_signal_test(void *)
{
#if defined(WIN32)
  return TEST_SKIPPED;
#else
  ASSERT_EQ(GEARMAN_IO_WAIT, wait_with_signal(true));
  return TEST_SUCCESS;
#endif
}

test_st signal_wait_test[]= {
  {"client option plumbing", 0, client_stop_wait_on_signal_option_test },
  {"worker option plumbing", 0, worker_stop_wait_on_signal_option_test },
  {"wait ignores signal by default", 0, gearman_wait_ignores_signal_by_default_test },
  {"wait returns io_wait when enabled", 0, gearman_wait_stops_on_signal_test },
  {0, 0, 0}
};

collection_st collection[]= {
  {"signal_wait", 0, 0, signal_wait_test},
  {0, 0, 0, 0}
};

void get_world(libtest::Framework *world)
{
  world->collections(collection);
}
