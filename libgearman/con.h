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

#ifndef __GEARMAN_CON_H__
#define __GEARMAN_CON_H__

#ifdef __cplusplus
extern "C" {
#endif

/* Common usage, add a connection structure. */
gearman_con_st *gearman_con_add(gearman_st *gearman, gearman_con_st *con,
                                char *host, in_port_t port);

/* Initialize a connection structure. */
gearman_con_st *gearman_con_create(gearman_st *gearman, gearman_con_st *con);

/* Clone a connection structure. */
gearman_con_st *gearman_con_clone(gearman_st *gearman, gearman_con_st *con,
                                  gearman_con_st *from);

/* Free a connection structure. */
gearman_return gearman_con_free(gearman_con_st *con);

/* Set options for a connection. */
void gearman_con_set_host(gearman_con_st *con, char *host);
void gearman_con_set_port(gearman_con_st *con, in_port_t port);
void gearman_con_set_options(gearman_con_st *con, gearman_con_options options,
                             uint32_t data);

/* Set and get application data pointer for a connection. */
void gearman_con_set_data(gearman_con_st *con, void *data);
void *gearman_con_get_data(gearman_con_st *con);

/* Set connection to an already open file description. */
void gearman_con_set_fd(gearman_con_st *con, int fd);

/* Connect to server. */
gearman_return gearman_con_connect(gearman_con_st *con);

/* Close a connection. */
gearman_return gearman_con_close(gearman_con_st *con);

/* Clear address info, freeing structs if needed. */
void gearman_con_reset_addrinfo(gearman_con_st *con);

/* Send packet to a connection. */
gearman_return gearman_con_send(gearman_con_st *con, gearman_packet_st *packet);

/* Receive packet from a connection. */
gearman_packet_st *gearman_con_recv(gearman_con_st *con,
                                    gearman_packet_st *packet,
                                    gearman_return *ret);

/* State loop for gearman_con_st. */
gearman_return gearman_con_loop(gearman_con_st *con);

/* Data structures. */
struct gearman_con_st
{
  gearman_st *gearman;
  gearman_con_st *next;
  gearman_con_st *prev;
  gearman_con_state state;
  gearman_con_options options;
  void *data;
  char host[NI_MAXHOST];
  in_port_t port;
  struct addrinfo *addrinfo;
  struct addrinfo *addrinfo_next;
  int fd;
  short events;
  short revents;
  char buffer[GEARMAN_MAX_BUFFER_LENGTH];
  char *buffer_ptr;
  ssize_t buffer_len;
  gearman_packet_st *packet;
  size_t packet_size;
};

#ifdef __cplusplus
}
#endif

#endif /* __GEARMAN_CON_H__ */
