# stdint_h.m4 serial 6
dnl Copyright (C) 1997-2004, 2006 Free Software Foundation, Inc.
dnl This file is free software; the Free Software Foundation
dnl gives unlimited permission to copy and/or distribute it,
dnl with or without modifications, as long as this notice is preserved.

dnl From Paul Eggert.

# Define HAVE_STDINT_H_WITH_UINTMAX if <stdint.h> exists,
# doesn't clash with <sys/types.h>, and declares uintmax_t.

AC_DEFUN([gl_AC_HEADER_STDINT_H],
[
  AC_CACHE_CHECK([for stdint.h], gl_cv_header_stdint_h,
  [AC_TRY_COMPILE(
    [#include <sys/types.h>
#include <stdint.h>],
    [uintmax_t i = (uintmax_t) -1; return !i;],
    gl_cv_header_stdint_h=yes,
    gl_cv_header_stdint_h=no)])
  if test $gl_cv_header_stdint_h = yes; then
    AC_DEFINE_UNQUOTED(HAVE_STDINT_H_WITH_UINTMAX, 1,
      [Define if <stdint.h> exists, doesn't clash with <sys/types.h>,
       and declares uintmax_t. ])
  fi
])

AC_CACHE_CHECK(for int16_t, gl_cv_type_int16_t,
    [AC_TRY_COMPILE(
         [#include <stdint.h>],
         [int16_t s;],
         [gl_cv_type_int16_t=yes],
         [gl_cv_type_int16_t=no]
     )
    ]
)
AC_CACHE_CHECK(for uint16_t, gl_cv_type_uint16_t,
    [AC_TRY_COMPILE(
         [#include <stdint.h>],
         [uint16_t s;],
         [gl_cv_type_uint16_t=yes],
         [gl_cv_type_uint16_t=no]
     )
    ]
)
AC_CACHE_CHECK(for int32_t, gl_cv_type_int32_t,
    [AC_TRY_COMPILE(
         [#include <stdint.h>],
         [int32_t s;],
         [gl_cv_type_int32_t=yes],
         [gl_cv_type_int32_t=no]
     )
    ]
)
AC_CACHE_CHECK(for uint32_t_t, gl_cv_type_uint32_t,
    [AC_TRY_COMPILE(
         [#include <stdint.h>],
         [uint32_t s;],
         [gl_cv_type_uint32_t=yes],
         [gl_cv_type_uint32_t=no]
     )
    ]
)

