/*
 * File: stats/db.h
 * Purpose: interface to SQLite3 database manipulation
 *
 * Copyright (c) 2011 Robert Au
 *
 * This work is free software; you can redistribute it and/or modify it
 * under the terms of either:
 *
 * a) the GNU General Public License as published by the Free Software
 *    Foundation, version 2, or
 *
 * b) the "Angband licence":
 *    This software may be copied and distributed for educational,
 *    research,
 *    and not for profit purposes provided that this copyright and
 *    statement
 *    are included in all such copies.  Other copyrights may also apply.
 */

#ifndef STATS_DB_H
#define STATS_DB_H

#include <sqlite3.h>

extern bool stats_db_open(void);
extern bool stats_db_close(void);
extern int stats_db_exec(char *sql_str);
extern int stats_db_stmt_prep(sqlite3_stmt **sql_stmt, char *sql_str);
extern int stats_db_bind_ints(sqlite3_stmt *sql_stmt, int num_cols, ...);

#endif /* STATS_DB_H */
