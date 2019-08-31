#include <libc/unconst.h>
#include "dos-compat.h"

int iswprint(wint_t wc)
{
	int retval;
	retval = (wc < 0x100);
	return retval;
}

wchar_t * wcschr(const wchar_t *s, wchar_t c) {
	wchar_t cc = c;

	while (*s) {
		if (*s == cc) {
			return unconst(s, wchar_t *);
		}
		s++;
	}

	if (cc == 0) {
		return unconst(s, wchar_t *);
	}

	return 0;
}

size_t wcslen(const wchar_t *str) {
	const wchar_t *s;

	if (str == 0) {
		return 0;
	}

	for (s = str; *s; ++s);

	return s - str;
}
