## ------------------------                                 -*- Autoconf -*-
# adapted for LyX from the automake python support.
## Python file handling
## From Andrew Dalke
## Updated by James Henstridge
## ------------------------
# Copyright (C) 1999-2015 Free Software Foundation, Inc.
#
# This file is free software; the Free Software Foundation
# gives unlimited permission to copy and/or distribute it,
# with or without modifications, as long as this notice is preserved.

dnl The minimum version for a python2 interpreter
m4_define(py2_ver, 2.7.0)
dnl The minimum version for a python3 interpreter
m4_define(py3_ver, 3.3.0)

dnl Usage: LYX_PATH_PYTHON
dnl Find a suitable Python interpreter, that is either python2 >= py2_ver
dnl or python3 >= py3_ver. Stop with an error message if it has not been found.
AC_DEFUN([LYX_PATH_PYTHON],
 [
  m4_define_default([_AM_PYTHON_INTERPRETER_LIST],
[python python2 python3 python3.3 python3.2 python3.1 python3.0 python2.7 dnl
 python2.6 python2.5 python2.4 python2.3 python2.2 python2.1 python2.0])

    if test -n "$PYTHON"; then
      # If the user set $PYTHON, use it and don't search something else.
      AC_MSG_CHECKING([whether $PYTHON version is >= py2_ver or py3_ver])
      LYX_PYTHON_CHECK_VERSION([$PYTHON],
			      [AC_MSG_RESULT([yes])],
			      [AC_MSG_RESULT([no])
			       AC_MSG_ERROR([Python interpreter is not suitable])])
      am_display_PYTHON=$PYTHON
    else
      # Otherwise, try each interpreter until we find one that satisfies
      # LYX_PYTHON_CHECK_VERSION.
      AC_CACHE_CHECK([for a Python interpreter with version >= py2_ver or py3_ver],
	[am_cv_pathless_PYTHON],[
	for am_cv_pathless_PYTHON in _AM_PYTHON_INTERPRETER_LIST none; do
	  test "$am_cv_pathless_PYTHON" = none && break
	  LYX_PYTHON_CHECK_VERSION([$am_cv_pathless_PYTHON], [break])
	done])
      # Set $PYTHON to the absolute path of $am_cv_pathless_PYTHON.
      if test "$am_cv_pathless_PYTHON" = none; then
	PYTHON=:
      else
        AC_PATH_PROG([PYTHON], [$am_cv_pathless_PYTHON])
      fi
      am_display_PYTHON=$am_cv_pathless_PYTHON
    fi

    if test "$PYTHON" = : ; then
      AC_MSG_ERROR([no suitable Python interpreter found])
    fi
])

m4_define(py2_ver_list, [patsubst(py2_ver,[\.],[,])])
m4_define(py3_ver_list, [patsubst(py3_ver,[\.],[,])])

# LYX_PYTHON_CHECK_VERSION(PROG, [ACTION-IF-TRUE], [ACTION-IF-FALSE])
# ---------------------------------------------------------------------------
# Run ACTION-IF-TRUE if the Python interpreter PROG has version >= py2_ver or py3_ver.
# Run ACTION-IF-FALSE otherwise.
AC_DEFUN([LYX_PYTHON_CHECK_VERSION],
 [prog="import sys
version = sys.version_info@<:@:3@:>@
sys.exit(not ((py2_ver_list) <= version < (3,0,0) or version >= (py3_ver_list)))"
  AS_IF([AM_RUN_LOG([$1 -c "$prog"])], [$2], [$3])])
