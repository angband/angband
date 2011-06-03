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

/* Utility macro for executing and resetting a statement; assumes existence
 * of an integer err variable */

#define STATS_DB_STEP_RESET(s) \
	err = sqlite3_step(s);\
	if (err && err != SQLITE_DONE) return err;\
	err = sqlite3_reset(s);\
	if (err) return err;

/* Utility macro for finalizing a statement; assumes existence
 * of an integer err variable */

#define STATS_DB_FINALIZE(s) \
	err = sqlite3_finalize(s);\
	if (err) return err;

extern bool stats_db_open(void);
extern bool stats_db_close(void);
extern int stats_db_exec(char *sql_str);
extern int stats_db_stmt_prep(sqlite3_stmt **sql_stmt, char *sql_str);
extern int stats_db_bind_ints(sqlite3_stmt *sql_stmt, int num_cols, 
	int offset, ...);
extern int stats_db_bind_rv(sqlite3_stmt *sql_stmt, int col,
	random_value rv);

#endif /* STATS_DB_H */
