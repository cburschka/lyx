dnl We use this until autoconf fixes its version.
AC_DEFUN(LYX_FUNC_SELECT_ARGTYPES,
[AC_MSG_CHECKING([types of arguments for select()])
 AC_CACHE_VAL(ac_cv_func_select_arg234,dnl
 [AC_CACHE_VAL(ac_cv_func_select_arg1,dnl
  [AC_CACHE_VAL(ac_cv_func_select_arg5,dnl
   [for ac_cv_func_select_arg234 in 'fd_set *' 'int *' 'void *'; do
     for ac_cv_func_select_arg1 in 'int' 'size_t' 'unsigned long' 'unsigned'; do      for ac_cv_func_select_arg5 in 'struct timeval *' 'const struct timeval *'; do
       AC_TRY_COMPILE(dnl
[#ifdef HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif
#ifdef HAVE_SYS_TIME_H
#include <sys/time.h>
#endif
#ifdef HAVE_SYS_SELECT_H
#include <sys/select.h>
#endif
#ifdef HAVE_SYS_SOCKET_H
#include <sys/socket.h>
#endif
extern int select ($ac_cv_func_select_arg1,$ac_cv_func_select_arg234,$ac_cv_func_select_arg234,$ac_cv_func_select_arg234,$ac_cv_func_select_arg5);],,dnl
	[ac_not_found=no ; break 3],ac_not_found=yes)
      done
     done
    done
   ])dnl AC_CACHE_VAL
  ])dnl AC_CACHE_VAL
 ])dnl AC_CACHE_VAL
 if test "$ac_not_found" = yes; then
  ac_cv_func_select_arg1=int
  ac_cv_func_select_arg234='int *'
  ac_cv_func_select_arg5='struct timeval *'
 fi
 AC_MSG_RESULT([$ac_cv_func_select_arg1,$ac_cv_func_select_arg234,$ac_cv_func_select_arg5])
 AC_DEFINE_UNQUOTED(SELECT_TYPE_ARG1,$ac_cv_func_select_arg1,
		    [Define to the type of arg1 for select().])
 AC_DEFINE_UNQUOTED(SELECT_TYPE_ARG234,($ac_cv_func_select_arg234),
		    [Define to the type of args 2, 3 and 4 for select().])
 AC_DEFINE_UNQUOTED(SELECT_TYPE_ARG5,($ac_cv_func_select_arg5),
		    [Define to the type of arg5 for select().])
])

dnl Check things are declared in headers to avoid errors or warnings.
dnl Called like LYX_CHECK_DECL(function, headerfile)
dnl Defines HAVE_DECL_{FUNCTION}
AC_DEFUN(LYX_CHECK_DECL,
[AC_MSG_CHECKING(if $1 is declared by header $2)
tr_func=`echo $1 | tr 'abcdefghijklmnopqrstuvwxyz' 'ABCDEFGHIJKLMNOPQRSTUVWXYZ'`

tr_hdr=`echo $2 | tr . _`
AC_CACHE_VAL([lyx_cv_declare_${tr_hdr}_$1],
[AC_EGREP_HEADER($1, $2, [eval "lyx_cv_declare_${tr_hdr}_$1=yes"], [eval "lyx_cv_declare_${tr_hdr}_$1=no"])])
if eval "test \"\${lyx_cv_declare_${tr_hdr}_$1}\" = \"yes\""; then
	AC_DEFINE_UNQUOTED(HAVE_DECL_${tr_func},1,[dummy])
	AC_MSG_RESULT(yes)
else
	AC_MSG_RESULT(no)
fi])

dnl This is the multiple headers version of the LYX_CHECK_DECL macro above.
dnl Called like LYX_CHECK_DECL_HDRS(function, file1 file2 file3)
AC_DEFUN(LYX_CHECK_DECL_HDRS,
[ got="no"
for I in $2; do
tr_hdr=`echo $I | tr . _`
if test "${got}" = "no"; then
    LYX_CHECK_DECL($1, $I)
fi
if eval "test \"\${lyx_cv_declare_${tr_hdr}_$1}\" = \"yes\""; then
     got="yes"
fi
done])

# Copied from autoconf 2.53's general.m4
m4_define([AC_HELP_STRING],
[m4_pushdef([AC_Prefix], m4_default([$3], [                          ]))dnl
m4_pushdef([AC_Prefix_Format],
	   [  %-]m4_eval(m4_len(AC_Prefix) - 3)[s ])dnl [  %-23s ]
m4_text_wrap([$2], AC_Prefix, m4_format(AC_Prefix_Format, [$1]))dnl
m4_popdef([AC_Prefix_Format])dnl
m4_popdef([AC_Prefix])dnl
])
