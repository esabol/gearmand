/* Gearman server and library
 * Copyright (C) 2009 Cory Bennett
 * All rights reserved.
 *
 * Use and distribution licensed under the BSD license.  See
 * the COPYING file in the parent directory for full text.
 */

/**
 * @file
 * @brief libsqlite3 Queue Storage Declarations
 */

#pragma once

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @addtogroup gearman_queue_libsqlite3 libsqlite3 Queue Storage Declarations
 * @ingroup gearman_server_queue
 * @{
 */

/**
 * Get module configuration options.
 */
GEARMAN_API
gearmand_error_t gearman_server_queue_libsqlite3_conf(gearman_conf_st *conf);

/**
 * Initialize the queue.
 */
GEARMAN_API
gearmand_error_t gearman_server_queue_libsqlite3_init(gearman_server_st *server,
                                                      gearman_conf_st *conf);

/**
 * De-initialize the queue.
 */
GEARMAN_API
gearmand_error_t
gearman_server_queue_libsqlite3_deinit(gearman_server_st *server);

/**
 * Initialize the queue for a gearmand object.
 */
GEARMAN_API
gearmand_error_t gearmand_queue_libsqlite3_init(gearmand_st *gearmand,
                                                gearman_conf_st *conf);

/**
 * De-initialize the queue for a gearmand object.
 */
GEARMAN_API
gearmand_error_t gearmand_queue_libsqlite3_deinit(gearmand_st *gearmand);

/** @} */

#ifdef __cplusplus
}
#endif