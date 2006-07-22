dnl find a binary in the path
AC_DEFUN([QT4_FIND_PATH],
[
	AC_MSG_CHECKING([for $1])
	AC_CACHE_VAL(qt4_cv_path_$1,
	[
		qt4_cv_path_$1="NONE"
		if test -n "$$2"; then
			qt4_cv_path_$1="$$2";
		else
			dirs="$3"
			qt_save_IFS=$IFS
			IFS=':'
			for dir in $PATH; do
				dirs="$dirs $dir"
			done
			IFS=$qt_save_IFS

			for dir in $dirs; do
				if test -x "$dir/$1"; then
					if test -n "$5"; then
						evalstr="$dir/$1 $5 2>&1 "
						if eval $evalstr; then
							qt4_cv_path_$1="$dir/$1"
							break
						fi
					else
						qt4_cv_path_$1="$dir/$1"
						break
					fi
				fi
			done
		fi
	])

	if test -z "$qt4_cv_path_$1" || test "$qt4_cv_path_$1" = "NONE"; then
		AC_MSG_RESULT(not found)
		$4
	else
		AC_MSG_RESULT($qt4_cv_path_$1)
		$2=$qt4_cv_path_$1
	fi
])

dnl Find the uic compiler on the path or in qt_cv_dir
AC_DEFUN([QT_FIND_UIC4],
[
	QT4_FIND_PATH(uic, ac_uic4, $qt4_cv_dir/bin)

	if test -z "$ac_uic4" -a "$FATAL" = 1; then
		AC_MSG_ERROR([uic 4 binary not found in \$PATH or $qt4_cv_dir/bin !])
	fi
])

dnl Find the right moc in path/qt_cv_dir
AC_DEFUN([QT_FIND_MOC4],
[
	QT4_FIND_PATH(moc, ac_moc4, $qt4_cv_dir/bin)

	if test -z "$ac_moc4"  -a "$FATAL" = 1; then
		AC_MSG_ERROR([moc 4 binary not found in \$PATH or $qt4_cv_dir/bin !])
	fi
])

dnl check a particular libname
AC_DEFUN([QT4_TRY_LINK],
[
	SAVE_LIBS="$LIBS"
	LIBS="$LIBS $1"
	AC_TRY_LINK([
	#include <qglobal.h>
	#include <qstring.h>
		],
	[
	QString s("mangle_failure");
	#if (QT_VERSION < 400)
	break_me_(\\\);
	#endif
	],
	qt4_cv_libname=$1,
	)
	LIBS="$SAVE_LIBS"
])

dnl check we can do a compile
AC_DEFUN([QT4_CHECK_COMPILE],
[
	AC_MSG_CHECKING([for Qt 4 library name])

	AC_CACHE_VAL(qt4_cv_libname,
	[
		AC_LANG_CPLUSPLUS
		SAVE_CXXFLAGS=$CXXFLAGS
		CXXFLAGS="$CXXFLAGS $QT4_INCLUDES $QT4_LDFLAGS"
		for libname in '-lQtCore -lQtGui' \
		               '-lQtCore4 -lQtGui4'
		do
			QT4_TRY_LINK($libname)
			if test -n "$qt4_cv_libname"; then
				break;
			fi
		done
		CXXFLAGS=$SAVE_CXXFLAGS
	])

	if test -z "$qt4_cv_libname"; then
		AC_MSG_RESULT([failed])
		if test "$FATAL" = 1 ; then
			AC_MSG_ERROR([Cannot compile a simple Qt 4 executable. Check you have the right \$QT4DIR !])
		fi
	else
		AC_MSG_RESULT([$qt4_cv_libname])
	fi
])

dnl get Qt version we're using
AC_DEFUN([QT4_GET_VERSION],
[
	AC_CACHE_CHECK([Qt 4 version],lyx_cv_qt4version,
	[
		AC_LANG_CPLUSPLUS
		SAVE_CPPFLAGS=$CPPFLAGS
		CPPFLAGS="$CPPFLAGS $QT4_INCLUDES"

		cat > conftest.$ac_ext <<EOF
#line __oline__ "configure"
#include "confdefs.h"
#include <qglobal.h>
"%%%"QT_VERSION_STR"%%%"
EOF
		lyx_cv_qt4version=`(eval "$ac_cpp conftest.$ac_ext") 2>&5 | \
			grep '^"%%%"'  2>/dev/null | \
			sed -e 's/"%%%"//g' -e 's/"//g'`
		rm -f conftest.$ac_ext
		CPPFLAGS=$SAVE_CPPFLAGS
	])

	QT4_VERSION=$lyx_cv_qt4version
	AC_SUBST(QT4_VERSION)
])

dnl start here
AC_DEFUN([QT4_DO_IT_ALL],
[
	dnl this variable is precious
	AC_ARG_VAR(QT4DIR, [the place where the Qt 4 files are, e.g. /usr/lib/qt4])

	dnl Please leave this alone. I use this file in
	dnl oprofile.
	FATAL=0

	AC_ARG_WITH(qt4-dir, [  --with-qt4-dir           where the root of Qt 4 is installed ],
		[ qt4_cv_dir=`eval echo "$withval"/` ])

	AC_ARG_WITH(qt4-includes, [  --with-qt4-includes      where the Qt 4 includes are. ],
		[ qt4_cv_includes=`eval echo "$withval"` ])

	AC_ARG_WITH(qt4-libraries, [  --with-qt4-libraries     where the Qt 4 library is installed.],
		[  qt4_cv_libraries=`eval echo "$withval"` ])

	dnl pay attention to $QT4DIR unless overridden
	if test -z "$qt4_cv_dir"; then
		qt4_cv_dir=$QT4DIR
	fi

	dnl derive inc/lib if needed
	if test -n "$qt4_cv_dir"; then
		if test -z "$qt4_cv_includes"; then
			qt4_cv_includes=$qt4_cv_dir/include
		fi
		if test -z "$qt4_cv_libraries"; then
			qt4_cv_libraries=$qt4_cv_dir/lib
		fi
	fi

	dnl flags for compilation
	QT4_INCLUDES=
	QT4_LDFLAGS=
	if test -n "$qt4_cv_includes"; then
		QT4_INCLUDES="-I$qt4_cv_includes"
		for i in Qt QtCore QtGui; do
			QT4_INCLUDES="$QT4_INCLUDES -I$qt4_cv_includes/$i"
		done
	fi
	if test -n "$qt4_cv_libraries"; then
		QT4_LDFLAGS="-L$qt4_cv_libraries"
	fi
	AC_SUBST(QT4_INCLUDES)
	AC_SUBST(QT4_LDFLAGS)

	dnl Preprocessor flags
	QT4_CPPFLAGS="-DQT_CLEAN_NAMESPACE -DQT_GENUINE_STR -DQT_NO_STL -DQT3_SUPPORT -DQT_NO_KEYWORDS"
	case ${host} in
	*mingw*) QT4_CPPFLAGS="-DQT_DLL $QT4_CPPFLAGS";;
	esac
	AC_SUBST(QT4_CPPFLAGS)

	if test -z "$MOC4"; then
		QT_FIND_MOC4
		MOC4=$ac_moc4
	fi
	AC_SUBST(MOC4)
	if test -z "$UIC4"; then
		QT_FIND_UIC4
		UIC4=$ac_uic4
	fi
	AC_SUBST(UIC4)

	QT4_CHECK_COMPILE

	QT4_LIB=$qt4_cv_libname;
	AC_SUBST(QT4_LIB)

	if test -n "$qt4_cv_libname"; then
		QT4_GET_VERSION
	fi
])
