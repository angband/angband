#include <wchar.h>
#include <wctype.h>

#ifndef iswprint
extern int iswprint(wint_t);
#endif
#ifndef wcschr
extern wchar_t * wcschr(const wchar_t *, wchar_t);
#endif
#ifndef wcslen
extern size_t wcslen(const wchar_t *);
#endif
