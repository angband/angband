/*
 * File: stats/db.c
 * Purpose: SQLite3 database storage functions
 *
 * Copyright (c) 2011 Robert Au <myshkin+angband@durak.net>
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

#include <sqlite3.h>
#include <time.h>
#include <sys/stat.h>

#include "angband.h"

/* Module state variables */
static sqlite3 *db;
static char *ANGBAND_DIR_STATS;
static char *db_filename;

/* Utility functions */
static bool stats_make_output_dir(void) {
	size_t size = strlen(ANGBAND_DIR_USER) + strlen(PATH_SEP) + 6;
	ANGBAND_DIR_STATS = mem_alloc(size * sizeof(char));
	path_build(ANGBAND_DIR_STATS, size, ANGBAND_DIR_USER, "stats");

	if (dir_create(ANGBAND_DIR_STATS)) {
		return true;
	} else {
		return false;
	}
}

/* Interface functions */

/**
 * Call stats_db_open first to create the database file and set up a 
 * database connection. Returns true on success, false on failure.
 */
bool stats_db_open(void) {
	size_t size;
	char filename_buf[20];
	int result;
	time_t now_time = time(NULL);
	struct tm *now = localtime(&now_time);

	if (!stats_make_output_dir()) {
		return false;
	}

	size = strlen(ANGBAND_DIR_STATS) + strlen(PATH_SEP) + 20;
	db_filename = mem_alloc(size * sizeof(char));
	strnfmt(filename_buf, 20, "%4d-%02d-%02dT%02d:%02d.db",
		now->tm_year + 1900, now->tm_mon + 1, now->tm_mday,
		now->tm_hour, now->tm_min);
	path_build(db_filename, size, ANGBAND_DIR_STATS, filename_buf);

	if (file_exists(db_filename)) {
		return false;
	}

	result = sqlite3_open(db_filename, &db);
	if (result) {
		sqlite3_close(db);
		return false;
	}
	
	return true;	
}

/**
 * Call stats_close_db to close the database connection and free 
 * module variables.
 */
bool stats_db_close(void) {
	sqlite3_close(db);
	mem_free(ANGBAND_DIR_STATS);
	mem_free(db_filename);
	return true;
}

/**
 * Evaluate a sqlite3 SQL statement on the previously opened database.
 * The argument sql_str should contain the SQL statement, encoded as UTF-8.
 * The return value is zero on success or a sqlite3 error code on failure.
 * This function is a wrapper around sqlite3_exec and is for statements
 * that don't expect output. Use stats_db_stmt_prep(), sqlite3_bind_*(), and 
 * sqlite3_step() for more complex statements.
 */

int stats_db_exec(char *sql_str) {
	assert(strlen(sql_str) > 0);
	return sqlite3_exec(db, sql_str, NULL, NULL, NULL);
}

/**
 * Prepare a sqlite3 SQL statement for evaluation. sql_stmt should be a
 * non-NULL, unallocated pointer. The sqlite3 library will allocate memory
 * appropriately. The caller should later delete this statement with 
 * sqlite3_finalize(). sql_str should contain the SQL statement, encoded
 * as UTF-8. The function returns 0 on success or a sqlite3 error code on
 * failure.
 */

int stats_db_stmt_prep(sqlite3_stmt **sql_stmt, char *sql_str) {
	assert(strlen(sql_str) > 0);
	assert(sql_stmt != NULL);
	return sqlite3_prepare_v2(db, sql_str, strlen(sql_str),
		sql_stmt, NULL);
}

/**
 * Utility function for binding many ints at once. The offset argument 
 * should be the number of columns to skip before starting to bind. 
 * Arguments after num_cols should be a series of ints to be bound to
 * parameters in sql_stmt, in order.
 */

int stats_db_bind_ints(sqlite3_stmt *sql_stmt, int num_cols, int offset, ...) {
	va_list vp;
	int err = SQLITE_OK;
	int col;

	va_start(vp, offset);
	for (col = offset + 1; col <= offset + num_cols; col++) {
		u32b value = va_arg(vp, u32b);
		err = sqlite3_bind_int(sql_stmt, col, value);
		if (err) return err;
	}
	va_end(vp);

	return err;
}

/**
 * Utility function for binding a random_value to a parameter as TEXT.
 * Note optimization for storing values without randomness.
 */
int stats_db_bind_rv(sqlite3_stmt *sql_stmt, int col, random_value rv) {
	char sql_buf[256];

	if (rv.dice || rv.sides || rv.m_bonus) {
		strnfmt(sql_buf, 256, "%d+%dd%dM%d", rv.base, rv.dice, 
			rv.sides, rv.m_bonus);
	} else {
		strnfmt(sql_buf, 256, "%d", rv.base);
	}
	return sqlite3_bind_text(sql_stmt, col, sql_buf, strlen(sql_buf),
		SQLITE_STATIC);
}

/**
 * I have chosen not to wrap the other sqlite3 core interfaces, since
 * they do not require access to the database connection object db.
 * The key functions are
 *
 * int sqlite3_bind_<type>(sqlite3_stmt *, int, ...)
 *     Given a previously prepared statement, bind a parameter to a value
 *     of a given type: blob, double, int, int64, null, text, text16,
 *     value, zeroblob. The second argument is the index of the parameter.
 *
 * int sqlite3_step(sqlite3_stmt *)
 *     Evaluate the given statement. Returns SQLITE_DONE on successful
 *     execution completion. For statements that return data, returns
 *     SQLITE_ROW if a new row of data is ready for processing. Call the
 *     function multiple times to retrieve all the rows. It may also return
 *     other sqlite3 error codes.
 *
 * <C type> sqlite3_column_<sqlite3 type>(sqlite3_stmt *, int)
 *     Retrieve information about a column of the current result row of a
 *     query. The second argument is the index of the column. Available
 *     types are blob, double, int, int64, text, text16, and value.
 *     Routines attempt unit conversion where appropriate. Use 
 *     sqlite3_column_type() to determine the datatype code of a column,
 *     and sqlite3_column_bytes() or sqlite3_column_bytes16() to determikne
 *     the number of bytes in a blob or string.
 *
 * int sqlite3_reset(sqlite3_stmt *)
 *     Reset the given statement to its initial state, so that it may be
 *     re-executed. Does not alter bindings.
 *
 * int sqlite3_clear_bindings(sqlite3_stmt *)
 *     Reset all parameters for the given statement to NULL.
 *
 * int sqlite3_finalize(sqlite3_stmt *)
 *     Delete the given prepared statement.
 */
