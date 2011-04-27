/* test-utils.c
 *
 * Utility functions for tests
 *
 * Created by: myshkin
 *             26 Apr 2011
 */

#include "h-basic.h"
#include "config.h"
#include "init.h"
#include "z-util.h"

/*
 * Call this function to simulate init_stuff() and populate the *_info arrays
 */
void read_edit_files(void) {
	char configpath[512], libpath[512], datapath[512];

	my_strcpy(configpath, DEFAULT_CONFIG_PATH, sizeof(configpath));
	my_strcpy(libpath,    DEFAULT_CONFIG_PATH, sizeof(libpath));
	my_strcpy(datapath,   DEFAULT_CONFIG_PATH, sizeof(datapath));

	configpath[511] = libpath[511] = datapath[511] = '\0';

	if (!suffix(configpath, PATH_SEP))
		my_strcat(configpath, PATH_SEP, sizeof(configpath));
	if (!suffix(libpath, PATH_SEP))
		my_strcat(libpath, PATH_SEP, sizeof(libpath));
	if (!suffix(datapath, PATH_SEP))
		my_strcat(datapath, PATH_SEP, sizeof(datapath));

	init_file_paths(configpath, libpath, datapath);
	init_arrays();
}
