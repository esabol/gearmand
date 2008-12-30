/* Gearman server and library
 * Copyright (C) 2008 Brian Aker, Eric Day
 * All rights reserved.
 *
 * Use and distribution licensed under the BSD license.  See
 * the COPYING file in the parent directory for full text.
 */

/**
 * @file
 * @brief Server connection declarations
 */

#ifndef __GEARMAN_SERVER_CON_H__
#define __GEARMAN_SERVER_CON_H__

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @addtogroup gearman_server_con Server Connection Handling
 * This is a low level interface for gearman server connections. This is used
 * internally by the server interface, so you probably want to look there first.
 * @{
 */

/**
 * Initialize a server connection structure.
 */
gearman_server_con_st *gearman_server_con_create(gearman_server_st *server,
                                             gearman_server_con_st *server_con);

/**
 * Free a server connection structure.
 */
void gearman_server_con_free(gearman_server_con_st *server_con);

/**
 * Get application data pointer.
 */
void *gearman_server_con_data(gearman_server_con_st *server_con);

/**
 * Set application data pointer.
 */
void gearman_server_con_set_data(gearman_server_con_st *server_con, void *data);

/**
 * Add a server packet structure to a connection.
 */
gearman_server_packet_st *gearman_server_con_packet_add(gearman_server_con_st *server_con);

/**
 * Remove the first server packet structure from a connection.
 */
void gearman_server_con_packet_remove(gearman_server_con_st *server_con);

/**
 * Free server worker struction with name for a server connection.
 */
void gearman_server_con_free_worker(gearman_server_con_st *server_con,
                                    char *function_name,
                                    size_t function_name_size);

/**
 * Free all server worker structures for a server connection.
 */
void gearman_server_con_free_workers(gearman_server_con_st *server_con);

/** @} */

#ifdef __cplusplus
}
#endif

#endif /* __GEARMAN_SERVER_CON_H__ */