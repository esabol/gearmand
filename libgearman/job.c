/* Gearman server and library
 * Copyright (C) 2008 Brian Aker, Eric Day
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#include "common.h"

/* Initialize a job structure. */
gearman_job_st *gearman_job_create(gearman_st *gearman, gearman_job_st *job)
{
  if (job == NULL)
  {
    job= malloc(sizeof(gearman_job_st));
    if (job == NULL)
      return NULL;

    memset(job, 0, sizeof(gearman_job_st));
    job->options|= GEARMAN_JOB_ALLOCATED;
  }
  else
    memset(job, 0, sizeof(gearman_job_st));

  job->gearman= gearman;

  if (gearman->job_list)
    gearman->job_list->prev= job;
  job->next= gearman->job_list;
  gearman->job_list= job;
  gearman->job_count++;

  return job;
}

/* Free a job structure. */
void gearman_job_free(gearman_job_st *job)
{
  if (job->gearman->job_list == job)
    job->gearman->job_list= job->next;
  if (job->prev)
    job->prev->next= job->next;
  if (job->next)
    job->next->prev= job->prev;
  job->gearman->job_count--;

  if (job->options & GEARMAN_JOB_ALLOCATED)
    free(job);
}

/* Send result for a job. */
gearman_return gearman_job_send_result(gearman_job_st *job, char *result,
                                       size_t result_len)
{
  gearman_return ret;

  (void)gearman_packet_create(job->gearman, &(job->result));
  gearman_packet_set_header(&(job->result), GEARMAN_MAGIC_REQUEST,
                            GEARMAN_COMMAND_WORK_COMPLETE);

  /* Add job handle. */
  ret= gearman_packet_arg_add(&(job->result), job->packet.arg[0],
                              job->packet.arg_len[0]);
  if (ret != GEARMAN_SUCCESS)
    return ret;

  ret= gearman_packet_arg_add(&(job->result), result, result_len);
  if (ret != GEARMAN_SUCCESS)
    return ret;

  ret= gearman_packet_pack(&(job->result));
  if (ret != GEARMAN_SUCCESS)
    return ret;

  ret= gearman_con_send(job->con, &(job->result));
  if (ret != GEARMAN_SUCCESS)
    return ret;

  gearman_packet_free(&(job->result));

  return GEARMAN_SUCCESS;
}

/* Get job attributes. */
char *gearman_job_handle(gearman_job_st *job)
{
  return job->packet.arg[0];
}

char *gearman_job_function(gearman_job_st *job)
{
  return job->packet.arg[1];
}

char *gearman_job_arg(gearman_job_st *job)
{
  return job->packet.arg[2];
}

size_t gearman_job_arg_len(gearman_job_st *job)
{
  return job->packet.arg_len[2];
}
