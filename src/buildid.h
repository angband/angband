#ifndef BUILDID
#define BUILDID

#define VERSION_NAME	"Angband"

#ifdef BUILD_ID
# define STR(x) #x
# define XSTR(x) STR(x)
# define VERSION_STRING XSTR(BUILD_ID)
#else
# define VERSION_STRING "3.5-dev"
#endif

extern const char *buildid;
extern const char *buildver;

#endif /* BUILDID */
