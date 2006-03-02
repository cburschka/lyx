#! /bin/sh

# This script aims to do everything necessary to automate the building
# of a LyX/Win package.

# Invocation:
# sh build_lyxwin.sh "1.3.6-pre23"
# The string will be shown in the "About LyX" dialog.

# Notes:
# It uses the MSYS environment and MinGW compiler.

# It asks whether the Qt and LyX cvs trees are up to date.
# It asks whether the Qt library has been compiled.
# It checks that qt-mt3.dll, mingw10.dll and clean_dvi.py exist.
# It compiles the dv2dt and dt2dv utilites.
# It compiles and installs LyX.

# Once all this is done, you're ready to "package" LyX.
# See the README for details.

# The script compiles the .dll version of the Qt libraries. Linking of LyX
# against this will, therefore, take "some time".

# You may need to change these four variables.
MINGW_DIR="/j/MinGW"
QT_DIR="${HOME}"/Qt/3x-msys

# Everything from here on down should be OK "as is".
LYX_DIR="../../.."
PACKAGING_DIR="$LYX_DIR/development/Win32/packaging"
DTL_DIR="$PACKAGING_DIR/dtl"
CLEAN_DVI_DIR="$PACKAGING_DIR"

ASPELL_INSTALL_DIR="c:/Aspell"
LYX_ASPELL_DIR="/c/Aspell" # the Autotools don't like "C:/" syntax.
LYX_RELATIVE_BUILDDIR=build
LYX_INSTALL_DIR=installprefix

# These are all installed in the final LyX package
QT_DLL="${QT_DIR}/bin/qt-mt3.dll"
MINGW_DLL="${MINGW_DIR}/bin/mingwm10.dll"

DT2DV="${DTL_DIR}/dt2dv.exe"
DV2DT="${DTL_DIR}/dv2dt.exe"
CLEAN_DVI_PY="${CLEAN_DVI_DIR}/clean_dvi.py"

# Change this to 'mv -f' when you are confident that
# the various sed scripts are working correctly.
MV='mv -f'

check_dirs_exist()
{
    for dir in "$QT_DIR" "$LYX_DIR" "$DTL_DIR"
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
    for file in "${QT_DLL}" "${MINGW_DLL}" "${CLEAN_DVI_PY}"
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


run_automake()
{
    (
	cd "${LYX_DIR}" || {
	    echo "Unable to cd ${LYX_DIR}" >&2
	    exit 1
	}

	# Check the line endings of configure.ac
	# The configure script will be unable to create config.h if it
	# contains Win32-style line endings.
	sed 's/\r$//' configure.ac > configure.ac.$$
	cmp -s configure.ac configure.ac.$$ && {
	    rm -f configure.ac.$$
	} || {
	    mv -f configure.ac.$$ configure.ac
	    cat <<EOF >&2
configure.ac has Win32-style line endings. Corrected
Please use the Cygwin flavours of the autotools to
run autogen.sh
EOF
	    exit 1
	}

#	./autogen.sh || {
#	    echo "autogen.sh failed" >&2
#	    exit 1
#	}
    )
}


build_lyx()
{
    (
	cd "${LYX_DIR}" || {
	    echo "Unable to cd ${LYX_DIR}" >&2
	    exit 1
	}

	BUILDDIR="${LYX_RELATIVE_BUILDDIR}"
	test ! -d "${BUILDDIR}" && {
	    mkdir "${BUILDDIR}" || \
		Error "Unable to create build dir, ${BUILDDIR}."
	}

	CONFIGURE="../configure --without-x --with-included-gettext --with-extra-prefix='${LYX_ASPELL_DIR}' --with-frontend=qt QTDIR='$QT_DIR' --disable-maintainer-mode --disable-debug --enable-optimization --disable-pch --disable-concept-checks --disable-stdlib-debug"

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
	    echo "Failed to install" >&2
	    exit 1
	}
    )
}

LYX_VERSION_STR=""
test $# -ne 0 && LYX_VERSION_STR=$1

check_dirs_exist || exit 1
query_qt || exit 1
check_files_exist || exit 1
build_dtl || exit 1
run_automake || exit 1
build_lyx || exit 1
install_lyx || exit 1
# The end
