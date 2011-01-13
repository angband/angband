#include "buildid.h"

#ifdef BUILD_ID
# define STR(x) #x
# define XSTR(x) STR(x)
# define VERSION_STRING XSTR(BUILD_ID)
#else
# define VERSION_STRING "3.2.0"
#endif

const char *buildid = VERSION_NAME " " VERSION_STRING;
const char *buildver = VERSION_STRING;
