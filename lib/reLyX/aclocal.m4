dnl aclocal.m4 generated automatically by aclocal 1.4a

dnl Copyright (C) 1994, 1995-8, 1999 Free Software Foundation, Inc.
dnl This file is free software; the Free Software Foundation
dnl gives unlimited permission to copy and/or distribute it,
dnl with or without modifications, as long as this notice is preserved.

dnl This program is distributed in the hope that it will be useful,
dnl but WITHOUT ANY WARRANTY, to the extent permitted by law; without
dnl even the implied warranty of MERCHANTABILITY or FITNESS FOR A
dnl PARTICULAR PURPOSE.

dnl Usage: RELYX_WARNING(message)  Displays the warning "message" and sets
the
dnl flag lyx_warning to yes.
AC_DEFUN(RELYX_WARNING,[
relyx_warning_txt="$relyx_warning_txt
== $1
"
relyx_warning=yes])


dnl RELYX_SEARCH_PROG(VARIABLE-NAME,PROGRAMS-LIST,ACTION-IF-FOUND)
dnl             
define(RELYX_SEARCH_PROG,[dnl
for ac_prog in $2 ; do
# Extract the first word of "$ac_prog", so it can be a program name with
# args.
  set dummy $ac_prog ; ac_word=$[2]
  if test ! -n "[$]$1"; then
    IFS="${IFS=         }"; ac_save_ifs="$IFS"; IFS="${IFS}:"
    for ac_dir in $PATH; do
      test -z "$ac_dir" && ac_dir=.
      if test -f [$ac_dir/$ac_word]; then
        $1="$ac_prog"
        break
      fi
    done
    IFS="$ac_save_ifs"
  fi

  if test -n "[$]$1"; then
    ac_result=yes
  else
    ac_result=no
  fi
  ifelse($3,,,[$3])
  test -n "[$]$1" && break
done
])


dnl Usage RELYX_PROG_PERL_OK
AC_DEFUN(RELYX_PROG_PERL_OK,[
if echo 'require 5.002;exit' | $ac_dir/$ac_prog 2>&5
then
  PERL=$ac_dir/$ac_prog
else
  PERL=
fi])

dnl Usage RELYX_CHECK_PERL
AC_DEFUN(RELYX_CHECK_PERL,[
AC_MSG_CHECKING([for perl >= 5.002])
RELYX_SEARCH_PROG(PERL, perl perl5 perl5.002 perl5.003 perl5.004,
RELYX_PROG_PERL_OK
)
if test -n "$PERL" ; then
  AC_MSG_RESULT($PERL)
else
  PERL="/bin/echo reLyX has been disabled because perl version 5.002 has
not \
been found.\nTo reenable it, edit "
  AC_MSG_RESULT(no)
  RELYX_WARNING(dnl
Configure has not been able to find a version 5.002 or better of Perl.
   reLyX has been setup to only display a warning message.
   Set variable PERL to some value to install a working reLyX.)
fi
AC_SUBST(PERL)])

dnl Usage: RELYX_CHECK_ERRORS  Displays a warning message if a RELYX_ERROR
dnl   has occured previously. 
AC_DEFUN(RELYX_CHECK_ERRORS,[
if test x$relyx_error = xyes; then
cat <<EOF
**** The following problems have been detected by configure. 
**** Please check the messages below before running 'make'.
**** (see the section 'Problems' in the INSTALL file)
$relyx_error_txt
$relyx_warning_txt
deleting cache $cache_file
EOF
  rm -f $cache_file
else

if test x$relyx_warning = xyes; then
cat <<EOF
=== The following minor problems have been detected by configure. 
=== Please check the messages below before running 'make'.
=== (see the section 'Problems' in the INSTALL file)
$relyx_warning_txt
EOF
fi
cat <<EOF
Configuration of LyX was successful.  
Type 'make' to compile the program, 
and then 'make install' to install it.
EOF
fi])


# Do all the work for Automake.  This macro actually does too much --
# some checks are only needed if your package does certain things.
# But this isn't really a big deal.

# serial 1

dnl Usage:
dnl AM_INIT_AUTOMAKE(package,version, [no-define])

AC_DEFUN(AM_INIT_AUTOMAKE,
[AC_REQUIRE([AC_PROG_INSTALL])
dnl We require 2.13 because we rely on SHELL being computed by configure.
AC_PREREQ([2.13])
PACKAGE=[$1]
AC_SUBST(PACKAGE)
VERSION=[$2]
AC_SUBST(VERSION)
dnl test to see if srcdir already configured
if test "`cd $srcdir && pwd`" != "`pwd`" && test -f $srcdir/config.status; then
  AC_MSG_ERROR([source directory already configured; run "make distclean" there first])
fi
ifelse([$3],,
AC_DEFINE_UNQUOTED(PACKAGE, "$PACKAGE", [Name of package])
AC_DEFINE_UNQUOTED(VERSION, "$VERSION", [Version number of package]))
AC_REQUIRE([AM_SANITY_CHECK])
AC_REQUIRE([AC_ARG_PROGRAM])
dnl FIXME This is truly gross.
missing_dir=`cd $ac_aux_dir && pwd`
AM_MISSING_PROG(ACLOCAL, aclocal, $missing_dir)
AM_MISSING_PROG(AUTOCONF, autoconf, $missing_dir)
AM_MISSING_PROG(AUTOMAKE, automake, $missing_dir)
AM_MISSING_PROG(AUTOHEADER, autoheader, $missing_dir)
AM_MISSING_PROG(MAKEINFO, makeinfo, $missing_dir)
AC_REQUIRE([AC_PROG_MAKE_SET])])

#
# Check to make sure that the build environment is sane.
#

AC_DEFUN(AM_SANITY_CHECK,
[AC_MSG_CHECKING([whether build environment is sane])
# Just in case
sleep 1
echo timestamp > conftestfile
# Do `set' in a subshell so we don't clobber the current shell's
# arguments.  Must try -L first in case configure is actually a
# symlink; some systems play weird games with the mod time of symlinks
# (eg FreeBSD returns the mod time of the symlink's containing
# directory).
if (
   set X `ls -Lt $srcdir/configure conftestfile 2> /dev/null`
   if test "[$]*" = "X"; then
      # -L didn't work.
      set X `ls -t $srcdir/configure conftestfile`
   fi
   if test "[$]*" != "X $srcdir/configure conftestfile" \
      && test "[$]*" != "X conftestfile $srcdir/configure"; then

      # If neither matched, then we have a broken ls.  This can happen
      # if, for instance, CONFIG_SHELL is bash and it inherits a
      # broken ls alias from the environment.  This has actually
      # happened.  Such a system could not be considered "sane".
      AC_MSG_ERROR([ls -t appears to fail.  Make sure there is not a broken
alias in your environment])
   fi

   test "[$]2" = conftestfile
   )
then
   # Ok.
   :
else
   AC_MSG_ERROR([newly created file is older than distributed files!
Check your system clock])
fi
rm -f conftest*
AC_MSG_RESULT(yes)])

dnl AM_MISSING_PROG(NAME, PROGRAM, DIRECTORY)
dnl The program must properly implement --version.
AC_DEFUN(AM_MISSING_PROG,
[AC_MSG_CHECKING(for working $2)
# Run test in a subshell; some versions of sh will print an error if
# an executable is not found, even if stderr is redirected.
# Redirect stdin to placate older versions of autoconf.  Sigh.
if ($2 --version) < /dev/null > /dev/null 2>&1; then
   $1=$2
   AC_MSG_RESULT(found)
else
   $1="$3/missing $2"
   AC_MSG_RESULT(missing)
fi
AC_SUBST($1)])

