/* undef.c: undefined routines */

#include "constant.h"
#include "config.h"
#include "types.h"
#include <strings.h>
#include <sys/file.h>

extern short log_index;

void init_files()
{
}

int _new_log()
{
  log_index = 0;
  return 1;
}
