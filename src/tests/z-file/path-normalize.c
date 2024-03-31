/* z-file/path-normalize.c */

#include "unit-test.h"
#include "z-file.h"
#include "z-form.h"
#ifdef UNIX
#include "z-rand.h"
#endif
#ifdef WINDOWS
#include <windows.h>
#else
#include <unistd.h> /* getcwd() */
#endif

int setup_tests(void **state) {
#ifdef UNIX
	Rand_init();
#endif
	return 0;
}

NOTEARDOWN

static int test_unchanged(void *state) {
	struct {
		const char *path;
		size_t exp_root_sz;
	} unchanged_cases[] = {
#ifdef WINDOWS
		{ "C:\\", 3 },
		{ "C:\\Windows", 3 },
		{ "C:\\Windows\\test.ini", 3 },
		{ "C:\\Windows\\.hidden", 3 },
		{ "C:\\Windows\\..hidden2", 3 },
		{ "\\\\rephial.org\\angband\\", 22 },
		{ "\\\\rephial.org\\angband\\src", 22 },
		{ "\\\\rephial.org\\angband\\src\\Makefile", 22 },
		{ "\\\\rephial.org\\angband\\src\\.hidden", 22 },
		{ "\\\\rephial.org\\angband\\src\\..hidden2", 22 },
		{ "\\\\.\\C:", 4 },
		{ "\\\\?\\C:", 4 },
#else
		{ "/", 1 },
		{ "/var", 1 },
		{ "/etc/fstab", 1 },
		{ "/etc/.hidden", 1 },
		{ "/etc/..hidden2", 1 },
		{ "/etc/hello.txt~", 1 },
#endif
	};
	char buf[256];
	size_t lo, lreq, lroot;
	int i, j, result, adj;

	for (i = 0; i < (int) N_ELEMENTS(unchanged_cases); ++i) {
		lo = strlen(unchanged_cases[i].path);

		(void) memset(buf, '*', sizeof(buf));
		result = path_normalize(buf + 10, sizeof(buf) - 10,
			unchanged_cases[i].path, false, &lreq, &lroot);
		eq(result, 0);
		require(streq(buf + 10, unchanged_cases[i].path));
		eq(lreq, lo + 1);
		eq(lroot, unchanged_cases[i].exp_root_sz);
		for (j = 0; j < 10; ++j) {
			eq(buf[j], '*');
		}
		for (j = (int) lo + 11; j < (int) sizeof(buf); ++j) {
			eq(buf[j], '*');
		}

		(void) memset(buf, '*', sizeof(buf));
		result = path_normalize(buf + 10, sizeof(buf) - 10,
			unchanged_cases[i].path, true, NULL, NULL);
		eq(result, 0);
		if (unchanged_cases[i].path[lo - 1] == PATH_SEPC) {
			require(streq(buf + 10, unchanged_cases[i].path));
			adj = 0;
		} else {
			require(prefix(buf + 10, unchanged_cases[i].path)
				&& strlen(buf + 10) == lo + 1
				&& buf[10 + lo] == PATH_SEPC);
			adj = 1;
		}
		for (j = 0; j < 10; ++j) {
			eq(buf[j], '*');
		}
		for (j = (int) lo + adj + 11; j < (int) sizeof(buf); ++j) {
			eq(buf[j], '*');
		}

		result = path_normalize(NULL, 0, unchanged_cases[i].path,
			false, &lreq, NULL);
		eq(result, 1);
		eq(lreq, lo + 1);

		if (lo > 5) {
			(void) memset(buf, '*', sizeof(buf));
			result = path_normalize(buf + 10, lo - 5,
				unchanged_cases[i].path, true, NULL, &lroot);
			eq(result, 1);
			eq(lroot, unchanged_cases[i].exp_root_sz);
			for (j = 0; j < 10; ++j) {
				eq(buf[j], '*');
			}
			for (j = 10; j < (int) lo + 4; ++j) {
				eq(buf[j], unchanged_cases[i].path[j - 10]);
			}
			eq(buf[lo + 4], '\0');
			for (j = (int) lo + 5; j < (int) sizeof(buf); ++j) {
				eq(buf[j], '*');
			}
		}
	}
	ok;
}

static int test_relative_parts(void *state) {
	struct {
		const char *path, *path_out;
		size_t exp_req_sz, exp_trunc_req_sz, exp_root_sz;
	} relative_cases[] = {
#ifdef WINDOWS
		{ "C:\\Windows\\.", "C:\\Windows", 11, 11, 3 },
		{ "C:\\Windows\\.\\", "C:\\Windows\\", 12, 12, 3 },
		{ "C:\\Windows\\..", "C:\\", 11, 11, 3 },
		{ "C:\\Windows\\..\\", "C:\\", 11, 12, 3 },
		{ "C:\\Windows\\temp\\..", "C:\\Windows", 16, 16, 3 },
		{ "C:\\Windows\\temp\\..\\", "C:\\Windows\\", 16, 17, 3 },
		{ "C:\\.", "C:\\", 4, 4, 3 },
		{ "C:\\..", "C:\\", 4, 4, 3 },
		{ "C:\\Windows\\.\\temp", "C:\\Windows\\temp", 16, 16, 3 },
		{ "C:\\Windows\\..\\temp", "C:\\temp", 11, 16, 3 },
		{ "\\\\rephial.org\\angband\\src\\.",
			"\\\\rephial.org\\angband\\src", 26, 26, 22 },
		{ "\\\\rephial.org\\angband\\src\\.\\",
			"\\\\rephial.org\\angband\\src\\", 27, 27, 22 },
		{ "\\\\rephial.org\\angband\\src\\..",
			"\\\\rephial.org\\angband\\", 26, 26, 22 },
		{ "\\\\rephial.org\\angband\\src\\tests\\..\\",
			"\\\\rephial.org\\angband\\src\\", 32, 33, 22 },
		{ "\\\\rephial.org\\angband\\.",
			"\\\\rephial.org\\angband\\", 23, 23, 22 },
		{ "\\\\rephial.org\\angband\\..",
			"\\\\rephial.org\\angband\\", 23, 23, 22 },
		{ "\\\\rephial.org\\angband\\src\\.\\test",
			"\\\\rephial.org\\angband\\src\\test", 31, 31, 22 },
		{ "\\\\rephial.org\\angband\\src\\..\\lib",
			"\\\\rephial.org\\angband\\lib", 26, 30, 22 },
#else
		{ "/var/.", "/var", 5, 5, 1 },
		{ "/var/./", "/var/", 6, 6, 1 },
		{ "/var/..", "/", 5, 5, 1 },
		{ "/var/../", "/", 5, 6, 1 },
		{ "/var/tmp/../", "/var/", 9, 10, 1},
		{ "/.", "/", 2, 2, 1 },
		{ "/..", "/", 2, 2, 1 },
		{ "/var/./tmp", "/var/tmp", 9, 9, 1 },
		{ "/var/../vmlinuz", "/vmlinuz", 9, 13, 1 },
#endif
	};
	char buf[256];
	size_t lo, lreq, lroot;
	int i, j, result, adj;

	for (i = 0; i < (int) N_ELEMENTS(relative_cases); ++i) {
		lo = strlen(relative_cases[i].path_out);

		(void) memset(buf, '*', sizeof(buf));
		result = path_normalize(buf + 10, sizeof(buf) - 10,
			relative_cases[i].path, false, &lreq, &lroot);
		eq(result, 0);
		require(streq(buf + 10, relative_cases[i].path_out));
		eq(lreq, relative_cases[i].exp_req_sz);
		eq(lroot, relative_cases[i].exp_root_sz);
		for (j = 0; j < 10; ++j) {
			eq(buf[j], '*');
		}
		for (j = (int) lreq + 10; j < (int) sizeof(buf); ++j) {
			eq(buf[j], '*');
		}

		(void) memset(buf, '*', sizeof(buf));
		result = path_normalize(buf + 10, sizeof(buf) - 10,
			relative_cases[i].path, true, NULL, NULL);
		eq(result, 0);
		if (relative_cases[i].path_out[lo - 1] == PATH_SEPC) {
			require(streq(buf + 10, relative_cases[i].path_out));
			adj = 0;
		} else {
			require(prefix(buf + 10, relative_cases[i].path_out)
				&& strlen(buf + 10) == lo + 1
				&& buf[10 + lo] == PATH_SEPC);
			adj = 1;
		}
		for (j = 0; j < 10; ++j) {
			eq(buf[j], '*');
		}
		for (j = (int) lreq + adj + 10; j < (int) sizeof(buf); ++j) {
			eq(buf[j], '*');
		}

		result = path_normalize(NULL, 0, relative_cases[i].path, false,
			&lreq, NULL);
		eq(result, 1);
		eq(lreq, relative_cases[i].exp_trunc_req_sz);

		if (lo > 4) {
			(void) memset(buf, '*', sizeof(buf));
			result = path_normalize(buf + 10, lo - 4,
				relative_cases[i].path, true, NULL, &lroot);
			eq(result, 1);
			eq(lroot, relative_cases[i].exp_root_sz);
			for (j = 0; j < 10; ++j) {
				eq(buf[j], '*');
			}
			eq(buf[lo + 5], '\0');
			for (j = (int) lo + 6; j < (int) sizeof(buf); ++j) {
				eq(buf[j], '*');
			}
		}
	}

	ok;
}

static int test_redundant_separators(void *state) {
	struct {
		const char *path, *path_out;
		size_t exp_root_sz;
	} redundant_cases[] = {
#ifdef WINDOWS
		{ "C:\\Windows\\\\x", "C:\\Windows\\x", 3 },
		{ "C:\\Windows\\\\\\x", "C:\\Windows\\x", 3 },
		{ "C:\\Windows\\x\\\\", "C:\\Windows\\x\\", 3 },
		{ "C:\\Windows\\x\\\\\\", "C:\\Windows\\x\\", 3 },
		{ "C:\\\\", "C:\\", 3 },
		{ "C:\\\\\\", "C:\\", 3 },
		{ "\\\\rephial.org\\angband\\src\\\\test",
			"\\\\rephial.org\\angband\\src\\test", 22 },
		{ "\\\\rephial.org\\angband\\src\\\\\\test",
			"\\\\rephial.org\\angband\\src\\test", 22 },
		{ "\\\\rephial.org\\angband\\src\\\\",
			"\\\\rephial.org\\angband\\src\\", 22 },
		{ "\\\\rephial.org\\angband\\src\\\\\\",
			"\\\\rephial.org\\angband\\src\\", 22 },
		{ "\\\\rephial.org\\angband\\\\",
			"\\\\rephial.org\\angband\\", 22 },
		{ "\\\\rephial.org\\angband\\\\\\",
			"\\\\rephial.org\\angband\\", 22 },
		{ "\\\\.\\C:\\\\", "\\\\.\\C:\\", 4 },
#else
		{ "/var//tmp", "/var/tmp", 1 },
		{ "/var///tmp", "/var/tmp", 1 },
		{ "/var/tmp//", "/var/tmp/", 1 },
		{ "/var/tmp///", "/var/tmp/", 1 },
		{ "//", "/", 1 },
		{ "///", "/", 1 },
#endif
	};
	char buf[256];
	size_t lo, lreq, lroot;
	int i, j, result, adj;

	for (i = 0; i < (int) N_ELEMENTS(redundant_cases); ++i) {
		lo = strlen(redundant_cases[i].path_out);

		(void) memset(buf, '*', sizeof(buf));
		result = path_normalize(buf + 10, sizeof(buf) - 10,
			redundant_cases[i].path, false, &lreq, &lroot);
		eq(result, 0);
		require(streq(buf + 10, redundant_cases[i].path_out));
		eq(lreq, lo + 1);
		eq(lroot, redundant_cases[i].exp_root_sz);
		for (j = 0; j < 10; ++j) {
			eq(buf[j], '*');
		}
		for (j = (int) lo + 11; j < (int) sizeof(buf); ++j) {
			eq(buf[j], '*');
		}

		(void) memset(buf, '*', sizeof(buf));
		result = path_normalize(buf + 10, sizeof(buf) - 10,
			redundant_cases[i].path, true, NULL, NULL);
		eq(result, 0);
		if (redundant_cases[i].path_out[lo - 1] == PATH_SEPC) {
			require(streq(buf + 10, redundant_cases[i].path_out));
			adj = 0;
		} else {
			require(prefix(buf + 10, redundant_cases[i].path_out)
				&& strlen(buf + 10) == lo + 1
				&& buf[10 + lo] == PATH_SEPC);
			adj = 1;
		}
		for (j = 0; j < 10; ++j) {
			eq(buf[j], '*');
		}
		for (j = (int) lo + adj + 11; j < (int) sizeof(buf); ++j) {
			eq(buf[j], '*');
		}
	}

	ok;
}

static int test_working_directory(void *state) {
#if defined(WINDOWS) || defined(UNIX)
	struct {
		const char *path, *fmt_out;
		size_t exp_root_sz;
		bool need_sep, work_trunc_1;
	} working_cases[] = {
#ifdef WINDOWS
		{ "", "%s", 3, false, false },
		{ "hello.txt", "%shello.txt", 3, true, false },
		{ "work\\hello.txt", "%swork\\hello.txt", 3, true, false },
		{ ".hidden", "%s.hidden", 3, true, false },
		{ "..hidden2", "%s..hidden2", 3, true, false },
		{ "~", "%s~", 3, true, false },
		{ ".\\hello.txt", "%shello.txt", 3, true, false },
		{ ".\\work\\hello.txt", "%swork\\hello.txt", 3, true, false },
		{ ".\\.hidden", "%s.hidden", 3, true, false },
		{ ".\\..hidden2", "%s..hidden2", 3, true, false },
		{ ".", "%s", 3, false, false },
		{ ".\\", "%s", 3, true, false },
		{ "..\\hello.txt", "%shello.txt", 3, true, true },
		{ "..\\work\\hello.txt", "%swork\\hello.txt", 3, true, true },
		{ "..\\.hidden", "%s.hidden", 3, true, true },
		{ "..\\..hidden2", "%s..hidden2", 3, true, true },
		{ "..", "%s", 3, false, true },
		{ "..\\", "%s", 3, true, true },
#else
		{ "", "%s", 1, false, false },
		{ "hello.txt", "%shello.txt", 1, true, false },
		{ "work/hello.txt", "%swork/hello.txt", 1, true, false },
		{ ".hidden", "%s.hidden", 1, true, false },
		{ "..hidden2", "%s..hidden2", 1, true, false },
		{ "./hello.txt", "%shello.txt", 1, true, false },
		{ "./work/hello.txt", "%swork/hello.txt", 1, true, false },
		{ "./.hidden", "%s.hidden", 1, true, false },
		{ "./..hidden2", "%s..hidden2", 1, true, false },
		{ ".", "%s", 1, false, false },
		{ "./", "%s", 1, true, false },
		{ "../hello.txt", "%shello.txt", 1, true, true },
		{ "../work/hello.txt", "%swork/hello.txt", 1, true, true },
		{ "../.hidden", "%s.hidden", 1, true, true },
		{ "../..hidden2", "%s..hidden2", 1, true, true },
		{ "..", "%s", 1, false, true },
		{ "../", "%s", 1, true, true },
#endif /* else defined(WINDOWS) */
	};
	char wd[1024], wd_trunc_1[1024], buf[1024], expected[1024];
	char *p;
	size_t lwd, lo, lreq, lroot;
	int i, j, result, adj;

#ifdef WINDOWS
	lwd = GetCurrentDirectory(sizeof(wd), wd);
	require(lwd > 0);
	if (wd[0] == PATH_SEPC) {
		/* It's a UNC path. */
		require(wd[1] == PATH_SEPC);
		require((p = strchr(wd + 2, PATH_SEPC))
			&& (p = strchr(p + 1, PATH_SEPC)));
		lroot = (p - wd);
		for (i = 0; i < (int) N_ELEMENTS(working_cases); ++i) {
			working_cases[i].exp_root_sz = lroot;
		}
	} else {
		require(wd[1] == ':' && wd[2] == PATH_SEPC);
		lroot = 3;
	}
#else
	p = getcwd(wd, sizeof(wd));
	notnull(p);
	lwd = strlen(wd);
	require(lwd > 0 && wd[0] == PATH_SEPC);
	lroot = 1;
#endif /* else defined(WINDOWS) */
	if (wd[lwd - 1] != PATH_SEPC) {
		require(lwd < sizeof(wd) - 1);
		wd[lwd] = PATH_SEPC;
		++lwd;
		wd[lwd] = '\0';
	}
	/* Get a version with the last non-root component stripped off. */
	my_strcpy(wd_trunc_1, wd, sizeof(wd_trunc_1));
	if (lwd > lroot) {
		lo = lwd - 1;
		while (1) {
			assert(lo > 0);
			if (wd_trunc_1[lo - 1] == PATH_SEPC) {
				wd_trunc_1[lo] = '\0';
				break;
			}
			--lo;
		}
	}
	for (i = 0; i < (int) N_ELEMENTS(working_cases); ++i) {
		size_t lreq_exp, sepl = 0;

		if (!working_cases[i].need_sep) {
			/* Temporarily remove trailing path separator. */
			if (working_cases[i].work_trunc_1) {
				sepl = strlen(wd_trunc_1);
				if (sepl > working_cases[i].exp_root_sz) {
					wd_trunc_1[sepl - 1] = '\0';
				}
			} else {
				sepl = strlen(wd);
				if (sepl > working_cases[i].exp_root_sz) {
					wd[sepl - 1] = '\0';
				}
			}
		}
		(void) strnfmt(expected, sizeof(expected),
			working_cases[i].fmt_out,
			(working_cases[i].work_trunc_1) ? wd_trunc_1 : wd);
		lo = strlen(expected);
		if (sepl > working_cases[i].exp_root_sz) {
			/* Add back the trailing path separator. */
			if (working_cases[i].work_trunc_1) {
				wd_trunc_1[sepl - 1] = PATH_SEPC;
			} else {
				wd[sepl - 1] = PATH_SEPC;
			}
		}

		(void) memset(buf, '*', sizeof(buf));
		result = path_normalize(buf + 10, sizeof(buf) - 10,
			working_cases[i].path, false, &lreq, &lroot);
		eq(result, 0);
		require(streq(buf + 10, expected));
		lreq_exp = MAX(lwd + ((working_cases[i].path[0]) ? 1 : 0),
			lo + 1);
		eq(lreq, lreq_exp);
		eq(lroot, working_cases[i].exp_root_sz);
		for (j = 0; j < 10; ++j) {
			eq(buf[j], '*');
		}
		for (j = (int) lreq_exp + 10; j < (int) sizeof(buf); ++j) {
			eq(buf[j], '*');
		}

		(void) memset(buf, '*', sizeof(buf));
		result = path_normalize(buf + 10, sizeof(buf) - 10,
			working_cases[i].path, true, NULL, NULL);
		eq(result, 0);
		if (expected[lo - 1] == PATH_SEPC) {
			require(streq(buf + 10, expected));
			adj = 0;
		} else {
			require(prefix(buf + 10, expected)
				&& strlen(buf + 10) == lo + 1
				&& buf[10 + lo] == PATH_SEPC);
			adj = 1;
		}
		for (j = 0; j < 10; ++j) {
			eq(buf[j], '*');
		}
		lreq_exp =  MAX(lwd + ((working_cases[i].path[0]) ? 1 : 0),
			lo + 1 + adj);
		for (j = (int) lreq_exp + 10; j < (int) sizeof(buf); ++j) {
			eq(buf[j], '*');
		}
	}
#endif /* defined(WINDOWS) || defined(UNIX) */
	ok;
}

static int test_home_directory(void *state) {
#ifdef UNIX
	struct {
		const char *path, *fmt_out;
		bool need_sep, home_trunc_1;
	} home_cases[] = {
		{ "", "%s", false, false },
		{ "/", "%s", true, false },
		{ "/hello.txt", "%shello.txt", true, false },
		{ "/work/hello.txt", "%swork/hello.txt", true, false },
		{ "/.hidden", "%s.hidden", true, false },
		{ "/..hidden2", "%s..hidden2", true, false },
		{ "/./hello.txt", "%shello.txt", true, false },
		{ "/./work/hello.txt", "%swork/hello.txt", true, false },
		{ "/./.hidden", "%s.hidden", true, false },
		{ "/./..hidden2", "%s..hidden2", true, false },
		{ "/../hello.txt", "%shello.txt", true, true },
		{ "/../work/hello.txt", "%swork/hello.txt", true, true },
		{ "/../.hidden", "%s.hidden", true, true },
		{ "/../..hidden2", "%s..hidden2", true, true },
	};
	char hd[1024], hd_trunc_1[1024], path_in[1204], buf[1024],
		expected[1024];
	char user[128];
	struct passwd *pw;
	size_t lhd, lo, lreq, lroot;
	int i, j, result, adj;

	pw = getpwuid(getuid());
	notnull(pw);
	notnull(pw->pw_dir);
	lhd = strlen(pw->pw_dir);
	require(lhd > 0 && pw->pw_dir[0] == PATH_SEPC);
	require(lhd < sizeof(hd));
	my_strcpy(hd, pw->pw_dir, sizeof(hd));
	if (hd[lhd - 1] != PATH_SEPC) {
		require(lhd < sizeof(hd) - 1);
		hd[lhd] = PATH_SEPC;
		++lhd;
		hd[lhd] = '\0';
	}
	/* Get a version with the last non-root component stripped off. */
	my_strcpy(hd_trunc_1, hd, sizeof(hd_trunc_1));
	if (lhd > 1) {
		lo = lhd - 1;
		while (1) {
			assert(lo > 0);
			if (hd_trunc_1[lo - 1] == PATH_SEPC) {
				hd_trunc_1[lo] = '\0';
				break;
			}
			--lo;
		}
	}
	notnull(pw->pw_name);
	require(strlen(pw->pw_name) < sizeof(user) && pw->pw_name[0]);
	my_strcpy(user, pw->pw_name, sizeof(user));

	for (i = 0; i < (int) N_ELEMENTS(home_cases); ++i) {
		size_t lreq_exp, sepl = 0;

		if (!home_cases[i].need_sep) {
			/* Temporarily remove trailing path separator. */
			if (home_cases[i].home_trunc_1) {
				sepl = strlen(hd_trunc_1);
				if (sepl > 1) {
					hd_trunc_1[sepl - 1] = '\0';
				}
			} else {
				sepl = strlen(hd);
				if (sepl > 1) {
					hd[sepl - 1] = '\0';
				}
			}
		}
		(void) strnfmt(expected, sizeof(expected),
			home_cases[i].fmt_out,
			(home_cases[i].home_trunc_1) ? hd_trunc_1 : hd);
		lo = strlen(expected);
		if (sepl > 1) {
			/* Add back the trailing path separator. */
			if (home_cases[i].home_trunc_1) {
				hd_trunc_1[sepl - 1] = PATH_SEPC;
			} else {
				hd[sepl - 1] = PATH_SEPC;
			}
		}

		(void) strnfmt(path_in, sizeof(path_in), "~%s",
			home_cases[i].path);

		(void) memset(buf, '*', sizeof(buf));
		result = path_normalize(buf + 10, sizeof(buf) - 10,
			path_in, false, &lreq, &lroot);
		eq(result, 0);
		require(streq(buf + 10, expected));
		lreq_exp = MAX(lhd + ((home_cases[i].path[0]) ? 1 : 0),
			lo + 1);
		eq(lreq, lreq_exp);
		eq(lroot, 1);
		for (j = 0; j < 10; ++j) {
			eq(buf[j], '*');
		}
		for (j = (int) lreq_exp + 10; j < (int) sizeof(buf); ++j) {
			eq(buf[j], '*');
		}

		(void) memset(buf, '*', sizeof(buf));
		result = path_normalize(buf + 10, sizeof(buf) - 10,
			path_in, true, NULL, NULL);
		eq(result, 0);
		if (expected[lo - 1] == PATH_SEPC) {
			require(streq(buf + 10, expected));
			adj = 0;
		} else {
			require(prefix(buf + 10, expected)
				&& strlen(buf + 10) == lo + 1
				&& buf[10 + lo] == PATH_SEPC);
			adj = 1;
		}
		for (j = 0; j < 10; ++j) {
			eq(buf[j], '*');
		}
		lreq_exp =  MAX(lhd + ((home_cases[i].path[0]) ? 1 : 0),
			lo + 1 + adj);
		for (j = (int) lreq_exp + 10; j < (int) sizeof(buf); ++j) {
			eq(buf[j], '*');
		}

		(void) strnfmt(path_in, sizeof(path_in), "~%s%s", user,
			 home_cases[i].path);

		(void) memset(buf, '*', sizeof(buf));
		result = path_normalize(buf + 10, sizeof(buf) - 10,
			path_in, false, &lreq, &lroot);
		eq(result, 0);
		require(streq(buf + 10, expected));
		lreq_exp = MAX(lhd + ((home_cases[i].path[0]) ? 1 : 0),
			lo + 1);
		eq(lreq, lreq_exp);
		eq(lroot, 1);
		for (j = 0; j < 10; ++j) {
			eq(buf[j], '*');
		}
		for (j = (int) lreq_exp + 10; j < (int) sizeof(buf); ++j) {
			eq(buf[j], '*');
		}

	}
#endif

	ok;
}

static int test_invalid_user(void *state) {
#ifdef UNIX
	const char letters[] = "abcdefghijklmnopqrstuvwxyz"
		"ABCDEFGHIJKLMNOPQRSTUVWXYZ";
	char buf[40], out[256];
	size_t nreq, nroot;
	int nlet = (int) strlen(letters), i, result;

	buf[0] = '~';
	for (i = 1; i < 33; ++i) {
		buf[i] = letters[randint0(nlet)];
	}
	buf[33] = '\0';
	result = path_normalize(NULL, 0, buf, false, NULL, NULL);
	eq(result, 2);
	out[0] = 'a';
	nreq = 99;
	nroot = 99;
	result = path_normalize(out, sizeof(out), buf, false, &nreq, &nroot);
	eq(result, 2);
	eq(out[0], '\0');
	eq(nreq, 0);
	eq(nroot, 0);
	buf[33] = PATH_SEPC;
	buf[34] = '\0';
	result = path_normalize(NULL, 0, buf, false, NULL, NULL);
	eq(result, 2);
	out[0] = 'a';
	nreq = 99;
	nroot = 99;
	result = path_normalize(out, sizeof(out), buf, false, &nreq, &nroot);
	eq(result, 2);
	eq(out[0], '\0');
	eq(nreq, 0);
	eq(nroot, 0);
#endif
	ok;
}

static int test_invalid_unc_path(void *state) {
#ifdef WINDOWS
	const char * const cases[] = {
		"\\\\",
		"\\\\a",
		"\\\\a\\",
		"\\\\a\\b",
	};
	char out[256];
	int i;

	for (i = 0; i < (int) N_ELEMENTS(cases); ++i) {
		size_t nreq = 99, nroot = 99;
		int result = path_normalize(NULL, 0, cases[i], false, NULL,
			NULL);

		eq(result, 2);
		out[0] = 'a';
		result = path_normalize(out, sizeof(out), cases[i], false,
			&nreq, &nroot);
		eq(result, 2);
		eq(out[0], '\0');
		eq(nreq, 0);
		eq(nroot, 0);
	}
#endif
	ok;
}

const char *suite_name = "z-file/path-normalize";
struct test tests[] = {
	{ "unchanged", test_unchanged },
	{ "relative_parts", test_relative_parts },
	{ "redundant_separators", test_redundant_separators },
	{ "working_directory", test_working_directory },
	{ "home_directory", test_home_directory },
	{ "invalid_user", test_invalid_user },
	{ "invalid_unc_path", test_invalid_unc_path },
	{ NULL, NULL }
};
