/* undef.c: undefined routines */

#include "constant.h"
#include "config.h"
#include "types.h"

#ifdef MSDOS
#include <string.h>
#else
#include <strings.h>
#include <sys/file.h>
#endif

extern short log_index;

void init_files()
{
}

_new_log()
{
  log_index = 0;
  return 1;
}
