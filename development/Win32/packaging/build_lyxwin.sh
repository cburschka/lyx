#! /bin/sh

# This script aims to do averything necessary to automate the building
# of a LyX/Win package.

# Invocation:
# sh build_lyxwin.sh "1.3.6-pre23"
# The string will be shown in the "About LyX" dialog.

# This script is written as a bunch of subroutines. You'll probably
# only need to build a couple of the packages (dtl, aspell) once.
# Thereafter, the invocation of these subroutines can be commented out.
# (See the end of the script.)

# Notes:
# It uses the MinGW/MinSYS environment and compiler.

# It asks whether the Qt and LyX cvs trees are up to date.
# It asks whether the Qt library has been compiled.
# It checks that qt-mt3.dll, libiconv-2.dll,
#   mingw10.dll and clean_dvi.py exist.
# It compiles the dv2dt and dt2dv utilites.
# It compiles and installs the Aspell library.
# It compiles and installs LyX.
# It copies the dv2dt and dt2dv utilites, the .dlls and
#   clean_dvi.py to the lyx package.
# It modifies the Resources/lyx/configure script to
#   ensure that the generated .dvi file is usable.

# Once all this is done, you're ready to "package" LyX.
# See the README for details.

# The script compiles the .dll version of the Qt libraries. Linking of lyx
# against this will, therefore, take "some time".

# It compiles the static version of the Aspell libraries because no
# .dll version exists.

HOME="/home/Angus"

# You may need to change these three variables.
QT_DIR="${HOME}"/qt3
ASPELL_DIR="${HOME}"/aspell-0.50.5
# A space-separated string of directories
# ASPELL_DICT_DIRS="${HOME}/aspell-en-0.50-2 ${HOME}/aspell-de-0.50-2 "
ASPELL_DICT_DIRS="${HOME}/aspell-en-0.50-2"
LYX_DIR="${HOME}"/lyx/13x

# Everything from here on down should be OK "as is".
PACKAGING_DIR="$LYX_DIR/development/Win32/packaging"
DTL_DIR="$PACKAGING_DIR/dtl"
CLEAN_DVI_DIR="$PACKAGING_DIR"

ASPELL_INSTALL_DIR="c:/Aspell"
LYX_ASPELL_DIR="/c/Aspell" # the Autotools don't like "C:/" syntax.
LYX_RELATIVE_BUILDDIR=build
LYX_INSTALL_DIR=installprefix

# These are all installed in the final LyX package
QT_DLL="${QT_DIR}/bin/qt-mt3.dll"
LIBICONV_DLL="/j/MinGW/bin/libiconv-2.dll"
MINGW_DLL="/j/MinGW/bin/mingwm10.dll"

DT2DV="$DTL_DIR/dt2dv.exe"
DV2DT="$DTL_DIR/dv2dt.exe"
CLEAN_DVI_PY="$CLEAN_DVI_DIR/clean_dvi.py"

# Change this to 'mv -f' when you are confident that
# the various sed scripts are working correctly.
MV='mv -f'

check_dirs_exist()
{
    for dir in "$QT_DIR" "$ASPELL_DIR" "$LYX_DIR" "$DTL_DIR"
    do
      test -d "$dir" || {
	  echo "$dir does not exist" >&2
	  exit 1
      }
    done
}


query_qt()
{
    echo "Please ensure that the Qt and LyX cvs trees are up to date"
    echo "and that the Qt library is compiled and ready to go."
    echo "Press any key to continue"
    read ans
}


check_files_exist()
{
    # Check that the dlls and clean_dvi.py exist
    for file in "${QT_DLL}" "${LIBICONV_DLL}" "${MINGW_DLL}" "${CLEAN_DVI_PY}"
    do
      test -r "${file}" || {
	  echo "$file does not exist" >&2
	  exit 1
      }
    done
}


build_dtl()
{
    # dt2dv and dv2dt
    (
	cd "$DTL_DIR" || {
	    echo "Unable to cd $DTL_DIR" >&2
	    exit 1
	}

	make || {
	    echo "Failed to make $DTL_DIR" >&2
	    exit 1
	}
    )

    for file in "${DT2DV}" "${DV2DT}"
    do
      test -x "$file" || {
	  echo "${file} does not exist or is not executable" >&2
	  exit 1
      }
    done
}


build_aspell()
{
    # Aspell
    (
	cd "$ASPELL_DIR" || {
	    echo "Unable to cd $ASPELL_DIR" >&2
	    exit 1
	}

	./configure --enable-static --disable-shared --prefix="${ASPELL_INSTALL_DIR}" --sysconfdir="${ASPELL_INSTALL_DIR}" --enable-docdir="${ASPELL_INSTALL_DIR}/doc" --datadir="${ASPELL_INSTALL_DIR}/data" --enable-pkgdatadir="${ASPELL_INSTALL_DIR}/data" --enable-dict-dir="${ASPELL_INSTALL_DIR}/dict" --enable-win32-relocatable || {
	    echo "Failed to configure $ASPELL_DIR" >&2
	    exit 1
	}

        # We have to clean up two of the generated Makefiles.
	TMP=tmp.$$
	MAKEFILE=examples/Makefile
	sed '
# Replace "CC = gcc" with "CC = g++"
s/^ *\(CC *= *\)gcc *$/\1g++/
# Remove trailing "/" from the -I directory.
s@^ *\(INCLUDES *= *-I\${top_srcdir}/interfaces/cc\)/ *$@\1@
' "${MAKEFILE}" > "${TMP}"
	cmp -s "${MAKEFILE}" "${TMP}" && {
	    echo "${MAKEFILE} is unchanged" 2>&1
	} || {
	    diff -u "${MAKEFILE}" "${TMP}"
	    ${MV} "${TMP}" "${MAKEFILE}"
	}
	rm -f "${TMP}"

	MAKEFILE=prog/Makefile
	sed '
# Remove trailing "/" from the -I directories.
/^ *INCLUDES *= *-I\${top_srcdir}\/common/{
:loop
$!{
N
/\n *$/!bloop
}
s@/ *\(\\ *\n\)@ \1@g
}' "${MAKEFILE}" > "${TMP}"
	cmp -s "${MAKEFILE}" "${TMP}" && {
	    echo "${MAKEFILE} is unchanged" 2>&1
	} || {
	    diff -u "${MAKEFILE}" "${TMP}"
	    ${MV} "${TMP}" "${MAKEFILE}"
	}
	rm -f "${TMP}"

	make || {
	    echo "Failed to make $ASPELL_DIR" >&2
	    exit 1
	}

	rm -fr "$ASPELL_INSTALL_DIR" || {
	    echo "Failed to remove $ASPELL_INSTALL_DIR prior to installing Aspell" >&2
	    exit 1
	}

	make install || {
	    echo "Failed to install $ASPELL_DIR" >&2
	    exit 1
	}
    )
}


build_aspell_dicts()
{
    (
	PATH="${LYX_ASPELL_DIR}:$PATH"
	export PATH

	for dir in $ASPELL_DICT_DIRS
	do
	  (
		cd $dir
		./configure
		make
		make install
	  )
	done
    )

}

modify_version_C()
{
	VERSION_C="src/version.C"
	test -r "${VERSION_C}" || {
	    echo "Unable to find ${VERSION_C}"
	    return
	}
	test "${LYX_VERSION_STR}" == "" && return

	sed '/char const \* lyx_version = /s/"[^"]*"/"'${LYX_VERSION_STR}'"/' \
	    ${VERSION_C} > tmp.$$
	diff -u ${VERSION_C} tmp.$$
	${MV} tmp.$$ ${VERSION_C}
}


build_lyx()
{
    (
	cd "${LYX_DIR}" || {
	    echo "Unable to cd ${LYX_DIR}" >&2
	    exit 1
	}

	# Check the line endings of configure.ac
	# The configure script will be unable to create config.h if it
	# contains Win32-style line endings.
	sed 's/
$//' ${LYX_DIR}/configure.ac > configure.ac.$$
	cmp -s ${LYX_DIR}/configure.ac configure.ac.$$ && {
	    rm -f configure.ac.$$
	} || {
	    mv -f configure.ac.$$ ${LYX_DIR}/configure.ac
	    echo 'configure.ac has Win32-style line endings. Corrected' >&2
	}

	./autogen.sh || {
	    echo "autogen.sh failed" >&2
	    exit 1
	}

	BUILDDIR="${LYX_RELATIVE_BUILDDIR}"
	test ! -d "${BUILDDIR}" && {
	    mkdir "${BUILDDIR}" || \
		Error "Unable to create build dir, ${BUILDDIR}."
	}

	CONFIGURE="../configure --without-x --with-included-gettext --with-extra-prefix='${LYX_ASPELL_DIR}' --with-frontend=qt QTDIR='$QT_DIR'"

	echo "${CONFIGURE}"
	cd "${BUILDDIR}"
	echo "${PWD}"
	eval "${CONFIGURE}" || {
	    echo "Failed to configure LyX" >&2
	    exit 1
	}

	# Modify the "lyx_version" string in build/src/version.C
	modify_version_C

	# Build LyX
	make || {
	    echo "Failed to make $LYX_DIR" >&2
	    exit 1
	}
    )
}


install_lyx()
{
    (
	BUILDDIR="${LYX_RELATIVE_BUILDDIR}"
	cd "${LYX_DIR}/${BUILDDIR}" || {
	    echo "Unable to cd ${LYX_DIR}/${BUILDDIR}" >&2
	    exit 1
	}

	rm -fr "$LYX_INSTALL_DIR" || {
	    echo "Failed to remove $LYX_INSTALL_DIR prior to installing LyX" >&2
	    exit 1
	}

	make install || {
	    echo "Failed to install $LYX_DIR" >&2
	    exit 1
	}
    )
}

LYX_VERSION_STR=""
test $# -ne 0 && LYX_VERSION_STR=$1

check_dirs_exist || exit 1
query_qt || exit 1
check_files_exist || exit 1
#build_dtl || exit 1
#build_aspell || exit 1
#build_aspell_dicts || exit 1
build_lyx || exit 1
install_lyx || exit 1
# The end
