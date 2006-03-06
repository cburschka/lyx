dnl find a binary in the path
AC_DEFUN([QT_FIND_PATH],
[
	AC_MSG_CHECKING([for $1])
	AC_CACHE_VAL(qt_cv_path_$1,
	[
		qt_cv_path_$1="NONE"
		if test -n "$$2"; then
			qt_cv_path_$1="$$2";
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
							qt_cv_path_$1="$dir/$1"
							break
						fi
					else
						qt_cv_path_$1="$dir/$1"
						break
					fi
				fi
			done
		fi
	])

	if test -z "$qt_cv_path_$1" || test "$qt_cv_path_$1" = "NONE"; then
		AC_MSG_RESULT(not found)
		$4
	else
		AC_MSG_RESULT($qt_cv_path_$1)
		$2=$qt_cv_path_$1
	fi
])

dnl Find the uic compiler on the path or in qt_cv_dir
AC_DEFUN([QT_FIND_UIC],
[
	QT_FIND_PATH(uic, ac_uic, $qt_cv_dir/bin)
	if test -z "$ac_uic" -a "$FATAL" = 1; then
		AC_MSG_ERROR([uic binary not found in \$PATH or $qt_cv_dir/bin !])
	fi
	AC_MSG_CHECKING([whether uic supports -nounload])
	if $ac_uic --help 2>&1 | grep nounload >/dev/null ; then
		AC_MSG_RESULT([yes])
		ac_uic="$ac_uic -nounload"
	else
		AC_MSG_RESULT([no])
	fi
])

dnl Find the right moc in path/qt_cv_dir
AC_DEFUN([QT_FIND_MOC],
[
	QT_FIND_PATH(moc2, ac_moc2, $qt_cv_dir/bin)
	QT_FIND_PATH(moc, ac_moc1, $qt_cv_dir/bin)

	if test -n "$ac_moc1" -a -n "$ac_moc2"; then
		dnl found both. Prefer Qt3's if it exists else moc2
		$ac_moc1 -v 2>&1 | grep "Qt 3" >/dev/null
		if test "$?" = 0; then
			ac_moc=$ac_moc1;
		else
			ac_moc=$ac_moc2;
		fi
	else
		if test -n "$ac_moc1"; then
			ac_moc=$ac_moc1;
		else
			ac_moc=$ac_moc2;
		fi
	fi

	if test -z "$ac_moc"  -a "$FATAL" = 1; then
		AC_MSG_ERROR([moc binary not found in \$PATH or $qt_cv_dir/bin !])
	fi
])

dnl check a particular libname
AC_DEFUN([QT_TRY_LINK],
[
	SAVE_LIBS="$LIBS"
	LIBS="$LIBS $1"
	AC_TRY_LINK([
	#include <qglobal.h>
	#include <qstring.h>
		],
	[
	QString s("mangle_failure");
	#if (QT_VERSION < 221)
	break_me_(\\\);
	#endif
	],
	qt_cv_libname=$1,
	)
	LIBS="$SAVE_LIBS"
])

dnl check we can do a compile
AC_DEFUN([QT_CHECK_COMPILE],
[
	AC_MSG_CHECKING([for Qt library name])

	AC_CACHE_VAL(qt_cv_libname,
	[
		AC_LANG_CPLUSPLUS
		SAVE_CXXFLAGS=$CXXFLAGS
		CXXFLAGS="$CXXFLAGS $QT_INCLUDES $QT_LDFLAGS"

		for libname in -lqt-mt -lqt-mt3 -lqt3 -lqt2 -lqt;
		do
			QT_TRY_LINK($libname)
			if test -n "$qt_cv_libname"; then
				break;
			fi
		done

		CXXFLAGS=$SAVE_CXXFLAGS
	])

	if test -z "$qt_cv_libname"; then
		AC_MSG_RESULT([failed])
		if test "$FATAL" = 1 ; then
			AC_MSG_ERROR([Cannot compile a simple Qt executable. Check you have the right \$QTDIR !])
		fi
	else
		AC_MSG_RESULT([$qt_cv_libname])
	fi
])

dnl get Qt version we're using
AC_DEFUN([QT_GET_VERSION],
[
	AC_CACHE_CHECK([Qt version],lyx_cv_qtversion,
	[
		AC_LANG_CPLUSPLUS
		SAVE_CPPFLAGS=$CPPFLAGS
		CPPFLAGS="$CPPFLAGS $QT_INCLUDES"

		cat > conftest.$ac_ext <<EOF
#line __oline__ "configure"
#include "confdefs.h"
#include <qglobal.h>
"%%%"QT_VERSION_STR"%%%"
EOF
		lyx_cv_qtversion=`(eval "$ac_cpp conftest.$ac_ext") 2>&5 | \
			grep '^"%%%"'  2>/dev/null | \
			sed -e 's/"%%%"//g' -e 's/"//g'`
		rm -f conftest.$ac_ext
		CPPFLAGS=$SAVE_CPPFLAGS
	])

	QT_VERSION=$lyx_cv_qtversion
	AC_SUBST(QT_VERSION)
])

dnl Ascertain whether the Qt libraries are multi-threaded or not
AC_DEFUN([QT_CHECK_IS_MULTITHREADED],
[
	AC_CACHE_CHECK([whether the Qt library is multi-threaded],
	               qt_cv_is_multithreaded,
	[
		AC_LANG_CPLUSPLUS
		SAVE_CXXFLAGS=$CXXFLAGS
		SAVE_LIBS="$LIBS"
		CXXFLAGS="$CXXFLAGS $QT_INCLUDES $QT_LDFLAGS"
		LIBS="$LIBS $QT_LIB"

		AC_TRY_LINK(
		[
#define QT_THREAD_SUPPORT
#include <qapplication.h>
		],
		[
	QApplication a(0,0);
	a.unlock();
		],
		qt_cv_is_multithreaded=yes,
		qt_cv_is_multithreaded=no
		)

		LIBS="$SAVE_LIBS"
		CXXFLAGS=$SAVE_CXXFLAGS
	])

	if test x"$qt_cv_is_multithreaded" = xyes; then
		QT_CPPFLAGS="$QT_CPPFLAGS -DQT_THREAD_SUPPORT"
	fi
])

dnl start here
AC_DEFUN([QT_DO_IT_ALL],
[
	dnl this variable is precious
	AC_ARG_VAR(QTDIR, [the place where the Qt files are, e.g. /usr/lib/qt])

	dnl Please leave this alone. I use this file in
	dnl oprofile.
	FATAL=0

	AC_ARG_WITH(qt-dir, [  --with-qt-dir           where the root of Qt is installed ],
		[ qt_cv_dir=`eval echo "$withval"/` ])

	AC_ARG_WITH(qt-includes, [  --with-qt-includes      where the Qt includes are. ],
		[ qt_cv_includes=`eval echo "$withval"` ])

	AC_ARG_WITH(qt-libraries, [  --with-qt-libraries     where the Qt library is installed.],
		[  qt_cv_libraries=`eval echo "$withval"` ])

	dnl pay attention to $QTDIR unless overridden
	if test -z "$qt_cv_dir"; then
		qt_cv_dir=$QTDIR
	fi

	dnl derive inc/lib if needed
	if test -n "$qt_cv_dir"; then
		if test -z "$qt_cv_includes"; then
			qt_cv_includes=$qt_cv_dir/include
		fi
		if test -z "$qt_cv_libraries"; then
			qt_cv_libraries=$qt_cv_dir/lib
		fi
	fi

	dnl flags for compilation
	QT_INCLUDES=
	QT_LDFLAGS=
	if test -n "$qt_cv_includes"; then
		QT_INCLUDES="-I$qt_cv_includes"
	fi
	if test -n "$qt_cv_libraries"; then
		QT_LDFLAGS="-L$qt_cv_libraries"
	fi
	AC_SUBST(QT_INCLUDES)
	AC_SUBST(QT_LDFLAGS)

	dnl Preprocessor flags
	QT_CPPFLAGS="-DQT_CLEAN_NAMESPACE -DQT_GENUINE_STR -DQT_NO_STL"
	case ${host} in
	*mingw*) QT_CPPFLAGS="-DQT_DLL $QT_CPPFLAGS";;
	esac
	AC_SUBST(QT_CPPFLAGS)

	if test -z "$MOC"; then
		QT_FIND_MOC
		MOC=$ac_moc
	fi
	AC_SUBST(MOC)
	if test -z "$UIC"; then
		QT_FIND_UIC
		UIC=$ac_uic
	fi
	AC_SUBST(UIC)

	QT_CHECK_COMPILE

	QT_LIB=$qt_cv_libname;
	AC_SUBST(QT_LIB)

	QT_CHECK_IS_MULTITHREADED

	if test -n "$qt_cv_libname"; then
		QT_GET_VERSION
	fi
])

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
		QT4_TRY_LINK(-lQtCore)
		CXXFLAGS=$SAVE_CXXFLAGS
	])

	if test -z "$qt4_cv_libname"; then
		AC_MSG_RESULT([failed])
		if test "$FATAL" = 1 ; then
			AC_MSG_ERROR([Cannot compile a simple Qt 4 executable. Check you have the right \$QT4DIR !])
		fi
	else
		qt4_cv_libname="$qt4_cv_libname -lQtGui -lQt3Support"
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
		for i in Qt QtCore QtGui Qt3Support; do
			QT4_INCLUDES="$QT4_INCLUDES -I$qt4_cv_includes/$i"
		done
	fi
	if test -n "$qt4_cv_libraries"; then
		QT4_LDFLAGS="-L$qt4_cv_libraries"
	fi
	AC_SUBST(QT4_INCLUDES)
	AC_SUBST(QT4_LDFLAGS)

	dnl Preprocessor flags
	QT4_CPPFLAGS="-DQT_CLEAN_NAMESPACE -DQT_GENUINE_STR -DQT_NO_STL -DQT3_SUPPORT"
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
