dnl this is used by the macro blow to general a proper config.h.in entry
m4_define([LYX_AH_CHECK_DECL],
[AH_TEMPLATE(AS_TR_CPP(HAVE_DECL_$1),
  [Define if you have the prototype for function `$1'])])

dnl Check things are declared in headers to avoid errors or warnings.
dnl Called like LYX_CHECK_DECL(function, header1 header2...)
dnl Defines HAVE_DECL_{FUNCTION}
AC_DEFUN([LYX_CHECK_DECL],
[LYX_AH_CHECK_DECL($1)
for ac_header in $2
do
  AC_MSG_CHECKING([if $1 is declared by header $ac_header])
  AC_EGREP_HEADER($1, $ac_header,
      [AC_MSG_RESULT(yes)
       AC_DEFINE_UNQUOTED(AS_TR_CPP(HAVE_DECL_$1))
       break],
      [AC_MSG_RESULT(no)])
done])
