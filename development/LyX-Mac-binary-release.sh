#!/bin/sh

# set -x

# This script automates creating universal binaries of LyX on Mac.
# Author: Bennett Helm (and extended by Konrad Hofbauer)
# latest changes by Stephan Witt
# Last modified: February 2020

QtAPI=${QtAPI:-"-cocoa"}
QtVersion=${QtVersion:-"4.6.3"}
QtSourceVersion=${QtSourceVersion:-"qt-everywhere-opensource-src-${QtVersion}"}
QtBuildSubDir="qt-${QtVersion}-build${QtAPI}"
QtConfigureOptions=${QtConfigureOptions:-"-release"}
QtSkipComponents=${QtSkipComponents:-"qtconnectivity qtscript qtquickcontrols qtmultimedia qtvirtualkeyboard qtwayland qtwebengine"}
LibMagicVersion=${LibMagicVersion:-"5.19"}
LibMagicSource="file-${LibMagicVersion}"
LibMagicLibrary="libmagic.1.dylib"

ASpellVersion=${ASpellVersion:-"0.60.6.1"}
ASpellSource="aspell-${ASpellVersion}"
ASpellLibrary="libaspell.15.dylib"

HunSpellVersion=${HunSpellVersion:-"1.3.3"}
HunSpellSource="hunspell-${HunSpellVersion}"
HunSpellLibrary="libhunspell-1.3.0.dylib"

unset DYLD_LIBRARY_PATH LD_LIBRARY_PATH

# Prerequisite:
# * a decent checkout of LyX sources (probably you have it already)
# * Qt - build with shared or static libraries for the used platforms (default: i386 and ppc)
#   or - an unpacked source tree of Qt in $QTSOURCEDIR or in the sibling directory (variable QtSourceVersion)
# * for aspell support:
#   the aspell sources placed in a sibling directory (variable ASpellSource)
# * for hunspell support:
#   the hunspell sources placed in a sibling directory (variable HunSpellSource)
# * for dictionary deployment (only hunspell dicts and mythes thesauri are deployed per default):
#   - aspell:   the dictionary files of macports (in /opt/local/share/aspell and /opt/local/lib/aspell-0.60)
#   - hunspell: the dictionary files in the sibling directory dictionaries/dicts
#   - mythes:   the data and idx files in the sibling directory dictionaries/thes
# * for magic file type detection support:
#   the libmagic sources placed in a sibling directory (variable LibMagicSource)

LyXConfigureOptions="--enable-warnings --enable-optimization=-Os --with-x=no"
LyXConfigureOptions="${LyXConfigureOptions} --disable-stdlib-debug"
AspellConfigureOptions="--enable-warnings --enable-optimization=-O0 --enable-debug --disable-nls --enable-compile-in-filters --disable-pspell-compatibility"
HunspellConfigureOptions="--with-warnings --disable-nls --disable-static"

QtMajorVersion=qt4
QtConfigureOptions="${QtConfigureOptions} -opensource -silent -shared -confirm-license"
# stupid special case...
case "${QtVersion}:${QtAPI}" in
4.6*:-carbon)
	QtConfigureOptions="${QtConfigureOptions} -fast -no-exceptions"
	QtConfigureOptions="${QtConfigureOptions} -no-webkit -no-qt3support -no-javascript-jit -no-dbus"
	QtConfigureOptions="${QtConfigureOptions} -nomake examples -nomake demos -nomake docs -nomake tools"
	for arch in ${ARCH_LIST} ; do
		QTARCHS="${QTARCHS} -arch ${arch}"
	done
	;;
5.0*)
	QtConfigureOptions="${QtConfigureOptions} -fast -no-strip"
	QtConfigureOptions="${QtConfigureOptions} -no-javascript-jit -no-pkg-config"
	QtConfigureOptions="${QtConfigureOptions} -nomake examples -nomake demos -nomake docs -nomake tools"
	QtMajorVersion=qt5
	;;
5.6*|5.7*)
	QtConfigureOptions="${QtConfigureOptions} -no-strip"
	QtConfigureOptions="${QtConfigureOptions} -no-kms -no-pkg-config"
	QtConfigureOptions="${QtConfigureOptions} -nomake examples -nomake tools"
	QtConfigureOptions="${QtConfigureOptions} -skip qtconnectivity -skip qtscript"
	QtConfigureOptions="${QtConfigureOptions} -skip qtquickcontrols"
	QtConfigureOptions="${QtConfigureOptions} -skip qttools"
	QtConfigureOptions="${QtConfigureOptions} -skip qtdeclarative"
	QtMajorVersion=qt5
	;;
5.*)
	QtConfigureOptions="${QtConfigureOptions} -no-strip"
	QtConfigureOptions="${QtConfigureOptions} -no-kms -no-pkg-config"
	QtConfigureOptions="${QtConfigureOptions} -nomake examples -nomake tools"
	for component in ${QtSkipComponents} ; do
		QtConfigureOptions="${QtConfigureOptions} -skip ${component}"
	done
	QtMajorVersion=qt5
	;;
*)
	QtConfigureOptions="${QtConfigureOptions} -fast -no-exceptions"
	QtConfigureOptions="${QtConfigureOptions} -no-webkit -no-qt3support -no-javascript-jit -no-dbus"
	QtConfigureOptions="${QtConfigureOptions} -nomake examples -nomake demos -nomake docs -nomake tools"
	QtConfigureOptions="${QtConfigureOptions} ${QtAPI}"
	for arch in ${ARCH_LIST} ; do
		QTARCHS="${QTARCHS} -arch ${arch}"
	done
	;;
esac

aspell_dictionaries="no"
hunspell_dictionaries="yes"

libmagic_deployment="yes"
aspell_deployment="yes"
hunspell_deployment="yes"
thesaurus_deployment="yes"

qt_deployment="yes"

# auto detect Xcode location
DEVELOPER_SDKS=$(dirname $(xcrun --show-sdk-path))
if [ -n "${DEVELOPER_SDKS}" ]; then
	XCODE_DEVELOPER=$(dirname $(dirname $(xcrun --show-sdk-platform-path)))
	MACOSX_DEPLOYMENT_TARGET="10.7" # Lion support is default
	SDKROOT="${DEVELOPER_SDKS}/MacOSX$(xcrun --show-sdk-version).sdk" # use default SDK
elif [ -d "/Developer/SDKs" ]; then
	DEVELOPER_SDKS="/Developer/SDKs"
	XCODE_DEVELOPER="/Developer"
	MACOSX_DEPLOYMENT_TARGET="10.4" # Tiger support is default
	SDKROOT="${DEVELOPER_SDKS}/MacOSX10.5.sdk" # Leopard build is default
fi

# detection of script home
LyxSourceDir=$(dirname "$0")
if [ ! -d "${LyxSourceDir}" ]; then
	echo Missing LyX source directory.
	exit 2
fi
case "${LyxSourceDir}" in
/*/development)
	LyxSourceDir=$(dirname "${LyxSourceDir}")
	;;
/*)
	;;
*/development|development)
	LyxSourceDir=$(dirname "${LyxSourceDir}")
	LyxSourceDir=$(cd "${LyxSourceDir}";pwd)
	;;
*)
	LyxSourceDir=$(cd "${LyxSourceDir}";pwd)
	;;
esac

usage() {
	echo "*" Build script for LyX on Mac OS X
	echo
	echo Optional arguments:
	echo " --aspell-deployment=yes|no ." default yes
	echo " --with-qt-frameworks=yes|no." default no
	echo " --qt-deployment=yes|no ....." default yes
	echo " --with-sdkroot=SDKROOT ....." default 10.5 "(Leopard)"
	echo " --with-arch=ARCH ..........." default ppc,i386
	echo " --with-build-path=PATH ....." default \${lyx-src-dir}/../lyx-build
	echo " --with-dmg-location=PATH ..." default \${build-path}
	echo " --with-binary-strip=yes ...." default no
	echo " --codesign-identity=CSID ..." default is w/o CSID
	echo
	echo "All other arguments with -- are passed to configure"
	echo "including the defaults: ${LyXConfigureOptions}"
	case "${1}" in
	--help=short)
		;;
	*)
		if [ -x "${LyxSourceDir}/configure" ]; then
			echo
			echo "*" Configure options of LyX
			echo
			"${LyxSourceDir}/configure" --help
		fi
	esac
	exit 0
}

NCPU=$(sysctl -n hw.ncpu)
NCPU=$((NCPU / 2))
if [ $NCPU -gt 1 ]; then
	MAKEJOBS=-j${NCPU}
fi

while [ $# -gt 0 ]; do
	case "${1}" in
	--with-qt-frameworks=*)
		configure_qt_frameworks=$(echo ${1}|cut -d= -f2)
		if [ "$configure_qt_frameworks" = "yes" ]; then
			unset QTDIR
			qt_deployment="no"
		fi
		shift
		;;
	--with-qt-dir=*)
		QTDIR=$(echo ${1}|cut -d= -f2)
		shift
		;;
	--with-macos-deployment-target=*)
		MACOSX_DEPLOYMENT_TARGET=$(echo ${1}|cut -d= -f2)
		LyXConfigureOptions="${LyXConfigureOptions} ${1}"
		shift
		;;
	--with-sdkroot=*)
		SDKROOT=$(echo ${1}|cut -d= -f2)
		case "${SDKROOT}" in
		10.4)
			SDKROOT="${DEVELOPER_SDKS}/MacOSX10.4u.sdk"
			export CC=gcc-4.0
			export OBJC=gcc-4.0
			export CXX=g++-4.0
			;;
		*)
			SDKROOT="${DEVELOPER_SDKS}/MacOSX${SDKROOT}.sdk"
			if [ ! -d "${SDKROOT}" ]; then
				echo Invalid SDKROOT given: "${SDKROOT}"
				usage --help=short
			fi
			;;
		esac
		shift
		;;
	--codesign-identity=*)
		CODESIGN_IDENTITY=$(echo "${1}"|cut -d= -f2)
		shift
		;;
	--libmagic-deployment=*)
		libmagic_deployment=$(echo ${1}|cut -d= -f2)
		shift
		;;
	--aspell-deployment=*)
		aspell_deployment=$(echo ${1}|cut -d= -f2)
		aspell_dictionaries=$aspell_deployment
		shift
		;;
	--hunspell-deployment=*)
		hunspell_deployment=$(echo ${1}|cut -d= -f2)
		hunspell_dictionaries=$hunspell_deployment
		shift
		;;
	--thesaurus-deployment=*)
		thesaurus_deployment=$(echo ${1}|cut -d= -f2)
		shift
		;;
	--qt-deployment=*)
		qt_deployment=$(echo ${1}|cut -d= -f2)
		shift
		;;
	--with-arch=*)
		ARCH=$(echo ${1}|cut -d= -f2|tr ',' ' ')
		ARCH_LIST="${ARCH_LIST} ${ARCH}"
		shift
		;;
	--with-dmg-location=*)
		DMGLocation=$(echo ${1}|cut -d= -f2)
		shift
		;;
	--with-binary-strip=yes)
		strip="-strip"
		shift
		;;
	--with-build-path=*)
		LyxBuildDir=$(echo ${1}|cut -d= -f2)
		shift
		;;
	--with-util-dir=*)
		LyXUtilitiesDir=$(echo ${1}|cut -d= -f2)
		shift
		;;
	--help|--help=*)
		usage "${1}"
		;;
	--without-aspell)
		LyXConfigureOptions="${LyXConfigureOptions} ${1}"
		aspell_deployment="no"
		shift
		;;
	--with-included-hunspell)
		LyXConfigureOptions="${LyXConfigureOptions} ${1}"
		hunspell_deployment="no"
		shift
		;;
	--without-hunspell)
		LyXConfigureOptions="${LyXConfigureOptions} ${1}"
		hunspell_deployment="no"
		hunspell_dictionaries="no"
		shift
		;;
	--only-qt*=*)
		QtOnlyPackage=$(echo ${1}|cut -d= -f2)
		shift
		;;
	--only-package=*)
		LyxOnlyPackage=$(echo ${1}|cut -d= -f2)
		shift
		;;
	--enable-cxx11)
		LyXConfigureOptions="${LyXConfigureOptions} ${1}"
		EnableCXX11="--enable-cxx11"
		shift
		;;
	--*)
		LyXConfigureOptions="${LyXConfigureOptions} ${1}"
		shift
		;;
	*)
		break
		;;
	esac
done

if [ "${configure_qt_frameworks}" != "yes" ]; then
	QtInstallDir=${QTDIR:-"/opt/qt4"}
fi

ARCH_LIST=${ARCH_LIST:-"ppc i386"}

aspellstrip=

LyxBuildDir=${LyxBuildDir:-$(dirname "${LyxSourceDir}")/lyx-build}
DMGLocation=${DMGLocation:-"${LyxBuildDir}"}

LyXUtilitiesDir=${LyXUtilitiesDir:-"${LyxBuildDir}"/utilities}

LibMagicSourceDir=${LIBMAGICDIR:-$(dirname "${LyxSourceDir}")/${LibMagicSource}}
LibMagicBuildDir="${LyxBuildDir}"/"${LibMagicSource}"
LibMagicInstallDir=${LibMagicInstallDir:-"${LyXUtilitiesDir}"}
LibMagicInstallHdr="${LibMagicInstallDir}/include/magic.h"

ASpellSourceDir=${ASPELLDIR:-$(dirname "${LyxSourceDir}")/${ASpellSource}}
ASpellBuildDir="${ASpellSourceDir}"
ASpellInstallDir=${ASpellInstallDir:-"${LyXUtilitiesDir}"}
ASpellInstallHdr="${ASpellInstallDir}/include/aspell.h"

HunSpellSourceDir=${HUNSPELLDIR:-$(dirname "${LyxSourceDir}")/${HunSpellSource}}
HunSpellBuildDir="${HunSpellSourceDir}"
HunSpellInstallDir=${HunSpellInstallDir:-"${LyXUtilitiesDir}"}
HunSpellInstallHdr="${HunSpellInstallDir}/include/hunspell/hunspell.h"

QtSourceDir=${QTSOURCEDIR:-$(dirname "${LyxSourceDir}")/${QtSourceVersion}}
QtBuildDir=${QtBuildDir:-"${LyxBuildDir}"/${QtBuildSubDir:-"qt-build"}}

DictionarySourceDir=${DICTIONARYDIR:-$(dirname "${LyxSourceDir}")/dictionaries}
DocumentationDir=$(dirname "${LyxSourceDir}")/Documents
DmgBackground="${LyxSourceDir}"/development/MacOSX/dmg-background.png

if [ -z "${LyXVersion}" ]; then
	LyXVersion=$(grep AC_INIT "${LyxSourceDir}"/configure.ac | cut -d, -f2 | tr -d " []()")
fi
LyXVersionSuffix=${LyXVersionSuffix:-$(echo "${LyXVersion}" | cut -d. -f1-2)}
case "${LyXVersion}" in
*dev*)
	LyXGitCommitHash=$(cd "${LyxSourceDir}" ; git log -1 --pretty=format:%h)
	;;
esac

LyxName="LyX"
LyxBase="${LyxName}-${LyXVersion}"
LyxApp="${LyxBase}.app"
LyxAppDir="${LyxBuildDir}"/"${LyxBase}"
LyxBuildDir="${LyxAppDir}.build"
LyxAppPrefix="${LyxAppDir}.app"
# if zip file is needed... remove the comment sign
#LyxAppZip="${LyxAppPrefix}.zip"

# ---------------------------------
# DON'T MODIFY ANYTHING BELOW HERE!
# ---------------------------------

# don't change order here...
case "${QtVersion}" in
5.0.*|5.1.*)
	QtLibraries=${QtLibraries:-"QtSvg QtXml QtPrintSupport QtWidgets QtGui QtNetwork QtConcurrent QtCore"}
	QtFrameworkVersion="5"
	;;
5.12.*)
	QtLibraries=${QtLibraries:-"QtDBus QtSvg QtXml QtPrintSupport QtMacExtras QtWidgets QtGui QtNetwork QtConcurrent QtCore"}
	QtFrameworkVersion="5"
	;;
5*)
	QtLibraries=${QtLibraries:-"QtSvg QtXml QtPrintSupport QtMacExtras QtWidgets QtGui QtNetwork QtConcurrent QtCore"}
	QtFrameworkVersion="5"
	;;
6*)
	QtLibraries=${QtLibraries:-"QtCore5Compat QtDBus QtSvg QtXml QtPrintSupport QtSvgWidgets QtWidgets QtGui QtNetwork QtConcurrent QtCore"}
	QtFrameworkVersion="A"
	;;
*)
	QtLibraries=${QtLibraries:-"QtSvg QtXml QtGui QtNetwork QtCore"}
	QtFrameworkVersion="4"
	;;
esac

DMGNAME="${LyxBase}${LyXGitCommitHash:+-}${LyXGitCommitHash}"
DMGSIZE="550m"

# Check for existing SDKs
SDKs=$(echo ${DEVELOPER_SDKS}/MacOSX1[01]*sdk)
case $SDKs in
*${SDKROOT}*)
	;;
*10.6*)
	MACOSX_DEPLOYMENT_TARGET=${MACOSX_DEPLOYMENT_TARGET:-"10.5"}; export MACOSX_DEPLOYMENT_TARGET
	case "${MACOSX_DEPLOYMENT_TARGET}" in
	10.6)
		SDKROOT="${DEVELOPER_SDKS}/MacOSX10.6.sdk"; export SDKROOT
		;;
	10.5|10.4)
		SDKROOT=${SDKROOT:-"${DEVELOPER_SDKS}/MacOSX10.5.sdk"}; export SDKROOT
		;;
	esac
	;;
*10.5*)
	MACOSX_DEPLOYMENT_TARGET=${MACOSX_DEPLOYMENT_TARGET:-"10.4"}; export MACOSX_DEPLOYMENT_TARGET
	SDKROOT=${SDKROOT:-"${DEVELOPER_SDKS}/MacOSX10.5.sdk"}; export SDKROOT
	;;
*)
	echo Unknown or missing SDK for Mac OS X.
	exit 1
	;;
esac
MYCFLAGS="-mmacosx-version-min=${MACOSX_DEPLOYMENT_TARGET}"
MYLDFLAGS="-mmacosx-version-min=${MACOSX_DEPLOYMENT_TARGET}"

build_qt() {
	echo Build Qt library ${QtSourceDir}
	if [ "${QtInstallDir}" = "${QtBuildDir}" ]; then
		echo Bad install directory for Qt.
		echo Must be different from build directory "${QtBuildDir}".
		exit 1
	fi
	(
		mkdir -p "${QtBuildDir}" && cd "${QtBuildDir}"
		echo configure options:
		echo ${QtConfigureOptions} ${QTARCHS} -prefix "${QtInstallDir}"
		"${QtSourceDir}"/configure ${QtConfigureOptions} ${QTARCHS} -prefix "${QtInstallDir}"
		make -j1 && make -j1 install
	)
}

case ${QtOnlyPackage:-"no"} in
y*)
	build_qt
	exit 0
	;;
*)
	if [ "${configure_qt_frameworks}" != "yes" -a -d "${QtSourceDir}" -a ! \( -d "${QtBuildDir}" -a -d "${QtInstallDir}" \) ]; then
		build_qt
	fi
	;;
esac

if [ -d "${LibMagicSourceDir}" -a ! -f "${LibMagicInstallHdr}" ]; then
	# we have a private libmagic (file(1)) source tree at hand...
	# so let's build and install it
	if [ -z "${LibMagicVersion}" ]; then
		LibMagicVersion=$(grep AC_INIT "${LibMagicSourceDir}"/configure.ac | cut -d, -f2|tr -d " []()")
	fi

	LibMagicName="LibMagic"
	LibMagicBase="${LibMagicName}-${LibMagicVersion}"

	echo Build libmagic library ${LibMagicBase}
	echo configure options:
	echo --prefix="${LibMagicInstallDir}" ${LibMagicConfigureOptions}

	mkdir -p "${LibMagicBuildDir}" && cd "${LibMagicBuildDir}"

	# ----------------------------------------
	# Build LibMagic for different architectures
	# ----------------------------------------
	FILE_LIST="${LibMagicLibrary}"

	for arch in ${ARCH_LIST} ; do
		CPPFLAGS="${SDKROOT:+-isysroot ${SDKROOT}} -arch ${arch} ${MYCFLAGS}"; export CPPFLAGS
		LDFLAGS="${SDKROOT:+-isysroot ${SDKROOT}} -arch ${arch} ${MYLDFLAGS}"; export LDFLAGS
		"${LibMagicSourceDir}/configure"\
			--prefix="${LibMagicInstallDir}"\
			${LibMagicConfigureOptions}
		make && make install${strip}
		for file in ${FILE_LIST} ; do
			if [ -f "${LibMagicInstallDir}"/lib/${file} ]; then
				mv "${LibMagicInstallDir}"/lib/${file}\
					"${LibMagicInstallDir}"/lib/${file}-${arch} 
			else
				echo Cannot build and install LibMagic for ${arch}.
				exit 1
			fi
		done
	done
	# -------------------------
	# Create universal binaries
	# -------------------------
	for file in ${FILE_LIST} ; do
		OBJ_LIST=
		for arch in ${ARCH_LIST} ; do
			OBJ_LIST="${OBJ_LIST} lib/${file}-${arch}"
		done
		(
			cd "${LibMagicInstallDir}"
			lipo -create ${OBJ_LIST} -o lib/${file}
			# check for the "missing link"...
			test -f lib/libmagic.dylib || (cd lib ; ln -s "${LibMagicLibrary}" libmagic.dylib)
		)
	done
	# --------
	# Clean up
	# --------
	for arch in ${ARCH_LIST} ; do
		rm -f "${LibMagicInstallDir}"/lib/*-${arch}
	done
fi

if [ -d "${HunSpellSourceDir}" -a ! -f "${HunSpellInstallHdr}" ]; then
	# we have a private HunSpell source tree at hand...
	# so let's build and install it
	if [ -z "${HunSpellVersion}" ]; then
		HunSpellVersion=$(grep AC_INIT "${HunSpellSourceDir}"/configure.ac | cut -d, -f2|tr -d " []()")
	fi

	HunSpellName="Hunspell"
	HunSpellBase="${HunSpellName}-${HunSpellVersion}"

	echo Build hunspell library ${HunSpellBase}
	echo configure options:
	echo --prefix="${HunSpellInstallDir}" ${HunspellConfigureOptions}

	mkdir -p "${HunSpellBuildDir}" && cd "${HunSpellBuildDir}"

	# ----------------------------------------
	# Build HunSpell for different architectures
	# ----------------------------------------
	FILE_LIST="${HunSpellLibrary}"

	for arch in ${ARCH_LIST} ; do
		make distclean
		CPPFLAGS="${SDKROOT:+-isysroot ${SDKROOT}} -arch ${arch} ${MYCFLAGS}"; export CPPFLAGS
		LDFLAGS="${SDKROOT:+-isysroot ${SDKROOT}} -arch ${arch} ${MYLDFLAGS}"; export LDFLAGS
		"${HunSpellSourceDir}/configure"\
			--prefix="${HunSpellInstallDir}"\
			${HunspellConfigureOptions}
		make && make install${strip}
		for file in ${FILE_LIST} ; do
			if [ -f "${HunSpellInstallDir}"/lib/${file} ]; then
				mv "${HunSpellInstallDir}"/lib/${file}\
					"${HunSpellInstallDir}"/lib/${file}-${arch} 
			else
				echo Cannot build and install HunSpell for ${arch}.
				exit 1
			fi
		done
	done
	# -------------------------
	# Create universal binaries
	# -------------------------
	for file in ${FILE_LIST} ; do
		OBJ_LIST=
		for arch in ${ARCH_LIST} ; do
			OBJ_LIST="${OBJ_LIST} lib/${file}-${arch}"
		done
		(
			cd "${HunSpellInstallDir}"
			lipo -create ${OBJ_LIST} -o lib/${file}
			# check for the "missing link"...
			test -f lib/libhunspell.dylib || (cd lib ; ln -s "${HunSpellLibrary}" libhunspell.dylib)
		)
	done
	# --------
	# Clean up
	# --------
	for arch in ${ARCH_LIST} ; do
		rm -f "${HunSpellInstallDir}"/lib/*-${arch}
	done
fi

if [ -d "${ASpellSourceDir}" -a ! -f "${ASpellInstallHdr}" -a "yes" = "${aspell_deployment}" ]; then
	# we have a private ASpell source tree at hand...
	# so let's build and install it
	if [ -z "${ASpellVersion}" ]; then
		ASpellVersion=$(grep AC_INIT "${ASpellSourceDir}"/configure.ac | cut -d, -f2|tr -d " []()")
	fi

	ASpellName="Aspell"
	ASpellBase="${ASpellName}-${ASpellVersion}"

	echo Build aspell library ${ASpellBase}
	echo configure options:
	echo --prefix="${ASpellInstallDir}" ${AspellConfigureOptions}

	# ASpell builds inplace only :(
	cd "${ASpellSourceDir}"

	# ----------------------------------------
	# Build ASpell for different architectures
	# ----------------------------------------
	FILE_LIST="${ASpellLibrary}"

	for arch in ${ARCH_LIST} ; do
		make distclean
		CPPFLAGS="${SDKROOT:+-isysroot ${SDKROOT}} -arch ${arch} ${MYCFLAGS}"; export CPPFLAGS
		LDFLAGS="${SDKROOT:+-isysroot ${SDKROOT}} -arch ${arch} ${MYLDFLAGS}"; export LDFLAGS
		CXXFLAGS=-g "${ASpellSourceDir}/configure"\
			--prefix="${ASpellInstallDir}"\
			${AspellConfigureOptions}
		make && make install${aspellstrip}
		for file in ${FILE_LIST} ; do
			if [ -f "${ASpellInstallDir}"/lib/${file} ]; then
				mv "${ASpellInstallDir}"/lib/${file}\
					"${ASpellInstallDir}"/lib/${file}-${arch} 
			else
				echo Cannot build and install ASpell for ${arch}.
				exit 1
			fi
		done
	done
	# -------------------------
	# Create universal binaries
	# -------------------------
	for file in ${FILE_LIST} ; do
		OBJ_LIST=
		for arch in ${ARCH_LIST} ; do
			OBJ_LIST="${OBJ_LIST} lib/${file}-${arch}"
		done
		(
			cd "${ASpellInstallDir}"
			lipo -create ${OBJ_LIST} -o lib/${file}
		)
	done
	# --------
	# Clean up
	# --------
	for arch in ${ARCH_LIST} ; do
		rm -f "${ASpellInstallDir}"/lib/*-${arch}
	done
fi


framework_name() {
	echo "Frameworks/${1}.framework"
}

LYX_FILE_LIST="lyx lyxclient tex2lyx lyxconvert"
BUNDLE_PATH="Contents/MacOS"
LYX_BUNDLE_PATH="${LyxAppPrefix}/${BUNDLE_PATH}"

build_lyx() {
	# Clear Output
	if [ -n "${LyxAppZip}" -a -f "${LyxAppZip}" ]; then rm "${LyxAppZip}"; fi
	if [ -d "${LyxAppPrefix}" ]; then
		find "${LyxAppPrefix}" -type d -exec chmod u+w '{}' \;
		rm -rf "${LyxAppPrefix}"
	fi

	case "${EnableCXX11}" in
	"--enable-cxx11")
		export CC=cc
		export CXX="c++ -stdlib=libc++"
		export CXXFLAGS=-std=c++11
		;;
	esac

	# -------------------------------------
	# Automate configure check
	# -------------------------------------
	if [ ! -f "${LyxSourceDir}"/configure -o "${LyxSourceDir}"/configure -ot "${LyxSourceDir}"/configure.ac ]; then
		( cd "${LyxSourceDir}" && sh autogen.sh )
	else
		find "${LyxSourceDir}" -name Makefile.am -print | while read file ; do
			dname=$(dirname "$file")
			if [ -f "$dname/Makefile.in" -a "$dname/Makefile.in" -ot "$file" ]; then
				( cd "${LyxSourceDir}" && sh autogen.sh )
				break
			fi
		done
	fi
	# -------------------------------------
	# Build LyX for different architectures
	# -------------------------------------

	if [ -d "${ASpellInstallDir}" -a "yes" = "${aspell_deployment}" ]; then
		ConfigureExtraInc="--with-extra-inc=${ASpellInstallDir}/include"
		ConfigureExtraLib="--with-extra-lib=${ASpellInstallDir}/lib"
	fi

	if [ -d "${HunSpellInstallDir}" -a "yes" = "${hunspell_deployment}" ]; then
		HunSpellFramework=$(framework_name Hunspell)
		HunSpellFramework=$(basename "${HunSpellFramework}")
		ConfigureExtraInc="--with-extra-inc=${HunSpellInstallDir}/include"
		ConfigureExtraLib="--with-extra-lib=${HunSpellInstallDir}/lib"
		# LyXConfigureOptions="${LyXConfigureOptions} --with-hunspell-framework=${HunSpellFramework}"
	fi
	LyXConfigureOptions="${LyXConfigureOptions} ${ConfigureExtraInc}"
	LyXConfigureOptions="${LyXConfigureOptions} ${ConfigureExtraLib}"

	for arch in ${ARCH_LIST} ; do

		if [ -d "${LyxBuildDir}" ];  then rm -r "${LyxBuildDir}"; fi
		mkdir -p "${LyxBuildDir}" && cd "${LyxBuildDir}"

		CPPFLAGS="${SDKROOT:+-isysroot ${SDKROOT}} -arch ${arch}"
		LDFLAGS="${SDKROOT:+-isysroot ${SDKROOT}} -arch ${arch}"

		if [ "$configure_qt_frameworks" = "yes" ]; then
			export QT_CORE_CFLAGS="-FQtCore"
			export QT_CORE_LIBS="-framework QtCore"
			export QT_FRONTEND_CFLAGS="-FQtGui"
			export QT_FRONTEND_LIBS="-framework QtGui"
			CPPFLAGS="${CPPFLAGS} -I${SDKROOT}/Library/Frameworks/QtCore.framework/Headers"
			CPPFLAGS="${CPPFLAGS} -I${SDKROOT}/Library/Frameworks/QtGui.framework/Headers"
		fi

		echo LDFLAGS="${LDFLAGS}"
		export LDFLAGS
		echo CPPFLAGS="${CPPFLAGS}"
		export CPPFLAGS
		echo CONFIGURE_OPTIONS="${LyXConfigureOptions}" ${QtInstallDir:+"--with-qt-dir=${QtInstallDir}"}
		"${LyxSourceDir}/configure"\
			--prefix="${LyxAppPrefix}" --with-version-suffix="-${LyXVersionSuffix}"\
			${QtInstallDir:+"--with-qt-dir=${QtInstallDir}"} \
			${LyXConfigureOptions}\
			--enable-build-type=rel && \
		make ${MAKEJOBS} && make install${strip}
		for file in ${LYX_FILE_LIST} ; do
			if [ -f "${LYX_BUNDLE_PATH}/${file}" ]; then
				mv "${LYX_BUNDLE_PATH}/${file}"\
					"${LYX_BUNDLE_PATH}/${file}-${arch}" 
			else
				echo ERROR: Cannot build and install ${file} for ${arch}.
				exit 1
			fi
		done
	done
}

content_directory() {
	target="$1"
	content=$(dirname "${target}")
	content=$(dirname "${content}")
	echo "${content}"
}

installname() {
		echo install_name_tool "$@"
		install_name_tool "$@" || exit 1
}

private_framework() {
	fwname="$1" ; shift
	source="$1" ; shift
	target="$1" ; shift
	version=$(echo ${1:-"1.1.1"}.1.1.1 | cut -d. -f1-3) ; shift
	fwdir=$(framework_name "${fwname}")
	condir=$(content_directory "${target}")
	libnm=$(basename "${source}")
	libid="org.lyx."$(echo "${libnm}" | cut -d. -f1)
	svrsn=$(echo "${version}" | cut -d. -f1-2)
	fwvrsn="1"
	mkdir -p "${condir}/${fwdir}"/Versions/${fwvrsn}/Headers
	mkdir -p "${condir}/${fwdir}"/Versions/${fwvrsn}/Resources
	if [ ! -f "${condir}/${fwdir}/Versions/${fwvrsn}/${fwname}" ]; then
		cp -p "${source}" "${condir}/${fwdir}/Versions/${fwvrsn}/${fwname}"
		for hfile in "$@" ; do
			test -f "${hfile}" && cp -p "${hfile}" "${condir}/${fwdir}"/Versions/${fwvrsn}/Headers
		done
		ln -s ${fwvrsn} "${condir}/${fwdir}/Versions/Current"
		ln -s Versions/Current/Headers "${condir}/${fwdir}/Headers"
		ln -s Versions/Current/Resources "${condir}/${fwdir}/Resources"
		ln -s Versions/Current/"${fwname}" "${condir}/${fwdir}/${fwname}"
		installname -id "@executable_path/../${fwdir}/${fwname}" "${condir}/${fwdir}/${fwname}"
		if [ -f "${LyxSourceDir}"/development/LyX-Mac-frameworks-template.plist ]; then
			cat "${LyxSourceDir}"/development/LyX-Mac-frameworks-template.plist | sed \
				-e "s/@CFBundleExecutable@/${fwname}/" \
				-e "s/@CFBundleIdentifier@/${libid}/" \
				-e "s/@CFBundleShortVersionString@/${svrsn}/" \
				-e "s/@CFBundleVersion@/${version}/" > "${condir}/${fwdir}"/Resources/Info.plist
		fi
	fi
	installname -change "${source}" "@executable_path/../${fwdir}/${fwname}" "${target}"
}

deploy_qtlibs() {
	source="${QtInstallDir}"
	target="$1"
	version="Versions/${QtFrameworkVersion}/"
	condir=$(content_directory "${target}")
	mkdir -p "${condir}/Resources"
	test -f "${condir}/Resources/qt.conf" || cat - > "${condir}/Resources/qt.conf" <<-EOF
[Paths]
Plugins = PlugIns
Translations = translations
EOF
	if [ ! -d "${condir}/PlugIns" ]; then
		mkdir -p "${condir}/PlugIns"
		find "${source}/plugins" -name \*.dylib -print | grep -v _debug.dylib | while read libname ; do
			echo Copy plugin "${libname}"
			dylib=$(basename "${libname}")
			dirname=$(dirname "${libname}")
			dirname=$(basename "${dirname}")
			mkdir -p "${condir}/PlugIns/${dirname}"
			cp -p "${libname}" "${condir}/PlugIns/${dirname}"
		done
	fi
	for libnm in ${QtLibraries} ; do
		fwdir=$(framework_name "$libnm")
		dirname=$(dirname "${fwdir}")
		mkdir -p "${condir}/${dirname}"
		dirname=$(basename "${fwdir}")
		test -d "${condir}/${fwdir}" || (
			echo Copy framework "${source}/lib/"$(basename "${fwdir}")
			cp -pR "${source}/lib/"$(basename "${fwdir}") "${condir}/${fwdir}"
			rm -f "${condir}/${fwdir}/${libnm}"_debug "${condir}/${fwdir}/${version}${libnm}"_debug
			test -f "${condir}/${fwdir}/${libnm}".prl && mv "${condir}/${fwdir}/${libnm}".prl "${condir}/${fwdir}"/Resources
			test -f "${condir}/${fwdir}/${libnm}"_debug.prl && mv "${condir}/${fwdir}/${libnm}"_debug.prl "${condir}/${fwdir}"/Resources
			installname -id "@executable_path/../${fwdir}/${version}${libnm}" "${condir}/${fwdir}/${version}${libnm}"
			find "${condir}/PlugIns" "${condir}/"$(dirname "${fwdir}") -name Headers -prune -o -type f -print | while read filename ; do
				if [ "${filename}" != "${target}" ]; then
					otool -L "${filename}" 2>/dev/null | sort -u | while read library ; do
						# pattern match for: /path/to/qt/lib/QtGui.framework/Versions/4/QtGui (compatibility version 4.6.0, current version 4.6.2)
						case "${library}" in
						*@rpath/*"${libnm}"*"("*version*")"*)
							# echo rpath based name for ${libnm} is ok.
							;;
						*"${libnm}"*"("*version*")"*)
							installname -change\
								"${source}/lib/${dirname}/${version}${libnm}"\
								"@executable_path/../${fwdir}/${version}${libnm}"\
								"${filename}"
							;;
						esac
					done
				fi
			done
		)
		installname -change\
			"${source}/lib/${dirname}/${version}${libnm}"\
			"@executable_path/../${fwdir}/${version}${libnm}"\
			"${target}"
	done
	if [ -d "${source}"/translations ]; then
		if [ ! -d "${condir}/translations" ]; then
			mkdir -p "${condir}/translations"
		fi
		echo Copy Qt translations to "${condir}/translations"
		cp -p "${source}"/translations/qt_*.qm "${condir}/translations"
	fi
}

# -------------------------
# Create universal binaries
# -------------------------
convert_universal() {
	cd "${LyxAppPrefix}"
	for file in ${LYX_FILE_LIST} ; do
		OBJ_LIST=
		for arch in ${ARCH_LIST} ; do
			if [ -f "${BUNDLE_PATH}/${file}-${arch}" ]; then
				OBJ_LIST="${OBJ_LIST} ${BUNDLE_PATH}/${file}-${arch}"
			fi
		done
		if [ -n "${OBJ_LIST}" ]; then
			lipo -create ${OBJ_LIST} -o "${BUNDLE_PATH}/${file}"
		fi
		if [ -f "${LibMagicInstallDir}/lib/${LibMagicLibrary}" -a "yes" = "${libmagic_deployment}" ]; then
			private_framework LibMagic "${LibMagicInstallDir}/lib/${LibMagicLibrary}" "${LYX_BUNDLE_PATH}/${file}" \
				"${LibMagicVersion}" "${LibMagicInstallHdr}"
		fi
		if [ -f "${ASpellInstallDir}/lib/${ASpellLibrary}" -a "yes" = "${aspell_deployment}" ]; then
			private_framework Aspell "${ASpellInstallDir}/lib/${ASpellLibrary}" "${LYX_BUNDLE_PATH}/${file}" \
				"${ASpellVersion}" "${ASpellInstallHdr}"
		fi
		if [ -f "${HunSpellInstallDir}/lib/${HunSpellLibrary}" -a "yes" = "${hunspell_deployment}" ]; then
			private_framework Hunspell "${HunSpellInstallDir}/lib/${HunSpellLibrary}" "${LYX_BUNDLE_PATH}/${file}" \
				"${HunSpellVersion}" "${HunSpellInstallDir}/include/hunspell/"*.hxx "${HunSpellInstallHdr}"
		fi
		if [ -d "${QtInstallDir}/lib/QtCore.framework/Versions/${QtFrameworkVersion}" -a "yes" = "${qt_deployment}" ]; then
			deploy_qtlibs "${LYX_BUNDLE_PATH}/${file}"
		fi
		otool -L "${BUNDLE_PATH}/${file}" | while read reference ; do
			case "${reference}" in
			*"${LyxBuildDir}"*"("*")")
				echo ERROR: Bad reference to "${reference}" found!!
				;;
			esac
		done
	done
	for arch in ${ARCH_LIST} ; do
		rm -f "${BUNDLE_PATH}"/*-${arch}
	done
}

# -------------------------
# Create code sign signatures
# -------------------------
code_sign() {
	target="$1"
	condir=$(content_directory "${target}"/lyx)
	appdir=$(dirname "${condir}")
	# have to sign frameworks first
	for fwname in Aspell Hunspell LibMagic ; do
		fwitem="${condir}"/$(framework_name "${fwname}")
		if [ -d "${fwitem}" ]; then
			codesign --verbose --force --sign "${CODESIGN_IDENTITY}" "${fwitem}"
		fi
	done
	for csitem in \
		"${condir}"/Frameworks/Qt*.framework/Versions/${QtFrameworkVersion} \
		"${condir}"/PlugIns/*/lib*.dylib \
		"${condir}"/Library/Spotlight/* \
		"${target}"/inkscape \
		"${target}"/lilypond \
		"${target}"/lilypond-book \
		"${target}"/maxima \
		"${target}"/tex2lyx \
		"${target}"/lyxeditor \
		"${target}"/lyxconvert \
		"${target}"/lyxclient
	do
		codesign --verbose --force --sign "${CODESIGN_IDENTITY}" "${csitem}"
	done

	/usr/bin/codesign --verbose --force --sign "${CODESIGN_IDENTITY}" "${appdir}" || {
		echo Warning: codesign failed with certificate named '"'${CODESIGN_IDENTITY}'"'
	}
}

deduplicate() {
	find "$@" -type f -print | while read file ; do
		echo $(md5 -q "$file") "$file"
	done | sort | while read hash file ; do
		ppath=$(dirname "$pfile")
		path=$(dirname "$file")
		if [ "$phash" = "$hash" -a "$ppath" = "$path" ]; then
			pname=$(basename "$pfile")
			name=$(basename "$file")
			cmp -s "$pfile" "$file" && (
				rm "$file"
				cd "$path" && ln -s "$pname" "$name" && echo link for "$file" created
			)
		fi
		phash="$hash"
		pfile="$file"
	done
}

copy_dictionaries() {
	if [ -d "${ASpellInstallDir}" -a "yes" = "${aspell_dictionaries}" ]; then
		ASpellResources="${LyxAppPrefix}/Contents/Resources"
		# try to reuse macports dictionaries for now
		if [ -d /opt/local/lib/aspell-0.60 ]; then ASpellInstallDir=/opt/local ; fi
		mkdir -p "${ASpellResources}"
		echo Copy Aspell dictionaries from "${ASpellInstallDir}"
		mkdir -p "${ASpellResources}"/data "${ASpellResources}"/dicts
		cp -p -r "${ASpellInstallDir}/lib/aspell-0.60"/* "${ASpellResources}"/data
		cp -p -r "${ASpellInstallDir}/share/aspell"/* "${ASpellResources}"/dicts
	fi
	if [ -d "${DictionarySourceDir}" -a "yes" = "${hunspell_dictionaries}" ]; then
		HunSpellResources="${LyxAppPrefix}/Contents/Resources"
		( cd "${DictionarySourceDir}" && find dicts -name .svn -prune -o -type f -print | cpio -pmdv "${HunSpellResources}" )
		deduplicate "${HunSpellResources}"/dicts
	fi
	if [ -d "${DictionarySourceDir}" -a "yes" = "${thesaurus_deployment}" ]; then
		MyThesResources="${LyxAppPrefix}/Contents/Resources"
		( cd "${DictionarySourceDir}" && find thes -name .svn -prune -o -type f -print | cpio -pmdv "${MyThesResources}" )
		deduplicate "${MyThesResources}"/thes
	fi
}

set_bundle_display_options() {
	X_BOUNDS=$2
	Y_BOUNDS=$3
	Y_POSITION=$((Y_BOUNDS - 65))
	Y_BOUNDS=$((Y_BOUNDS + 20))
	LYX_X_POSITION=$((X_BOUNDS / 4))
	LYX_Y_POSITION=$Y_POSITION
	APP_X_POSITION=$((3 * X_BOUNDS / 4))
	APP_Y_POSITION=$Y_POSITION
	WITH_DOCUMENTS=$(test -d "${1}/Documents" && echo true || echo false)
	osascript <<-EOF
	tell application "Finder"
        set f to POSIX file ("${1}" as string) as alias
        tell folder f
            open
            tell container window
                set toolbar visible to false
                set statusbar visible to false
                set current view to icon view
                delay 1 -- sync
                set the bounds to {20, 50, $X_BOUNDS, $Y_BOUNDS}
            end tell
            delay 1 -- sync
            set icon size of the icon view options of container window to 64
            set arrangement of the icon view options of container window to not arranged
            if ${WITH_DOCUMENTS} then
               set position of item "Documents" to {$LYX_X_POSITION,0}
            end if
            set position of item "${LyxName}.app" to {$LYX_X_POSITION,$LYX_Y_POSITION}
            set position of item "Applications" to {$APP_X_POSITION,$APP_Y_POSITION}
            set background picture of the icon view options\
               of container window to file "background.png" of folder "Pictures"
            set the bounds of the container window to {0, 0, $X_BOUNDS, $Y_BOUNDS}
            update without registering applications
            delay 5 -- sync
            close
        end tell
        delay 5 -- sync
    end tell
EOF
}

make_dmg() {
	cd "${1}"

	BGSIZE=$(file "$DmgBackground" | awk -F , '/PNG/{print $2 }' | tr x ' ')
	BG_W=$(echo ${BGSIZE} | awk '{print $1 }')
	BG_H=$(echo ${BGSIZE} | awk '{print $2 }')

	rm -f "${DMGNAME}.sparseimage" "${DMGNAME}.dmg"

	hdiutil create -type SPARSE -size ${DMGSIZE:-"250m"} -fs HFS+ -volname "${LyxBase}" "${DMGNAME}"
	# Unmount currently mounted disk image
	mount | grep "${LyxBase}" && umount /Volumes/"${LyxBase}"
	test -d /Volumes/"${LyxBase}" && rmdir /Volumes/"${LyxBase}"

	# Mount the disk image
	DEVICES=$(hdiutil attach "${DMGNAME}.sparseimage" | cut -f 1)

	# Obtain device information
	DEVICE=$(echo $DEVICES | cut -f 1 -d ' ')
	VOLUME=$(mount |grep ${DEVICE} | cut -f 3 -d ' ')

	# copy in the application bundle
	ditto --hfsCompression "${LyxAppPrefix}" "${VOLUME}/${LyxName}.app"

	# copy in background image
	mkdir -p "${VOLUME}/Pictures"
	ditto --hfsCompression "${DmgBackground}" "${VOLUME}/Pictures/background.png"
	# symlink applications
	ln -s /Applications/ "${VOLUME}"/Applications
	test -d "${DocumentationDir}" && ditto --hfsCompression "${DocumentationDir}" "${VOLUME}"
	set_bundle_display_options "${VOLUME}" ${BG_W} ${BG_H}
	PATH="${XCODE_DEVELOPER}/Tools:$PATH" SetFile -a C "${VOLUME}"
	mv "${VOLUME}/Pictures" "${VOLUME}/.Pictures"

	# Unmount the disk image
	hdiutil detach ${DEVICE}

	# Convert the disk image to read-only
	hdiutil convert "${DMGNAME}.sparseimage" -format UDBZ -o "${DMGNAME}.dmg"
	rm -f "${DMGNAME}.sparseimage"
}

# ------------------------------
# Building distribution packages
# ------------------------------

build_package() {
	test -n "${LyxAppZip}" && (
		cd "${LyxAppPrefix}" && zip -r "${LyxAppZip}" .
	)

	DMGARCH=""
	for arch in ${ARCH_LIST} ; do
		DMGARCH="${DMGARCH}-${arch}"
	done
	QtDmgArchSuffix=${QtMajorVersion}${DMGARCH}${QtAPI}.dmg

	test -n "${DMGLocation}" && (
		make_dmg "${DMGLocation}"
		if [ -d "${QtInstallDir}/lib/QtCore.framework/Versions/${QtFrameworkVersion}" -a "yes" = "${qt_deployment}" ]; then
			rm -f "${DMGLocation}/${DMGNAME}+${QtDmgArchSuffix}"
			echo move to "${DMGLocation}/${DMGNAME}+${QtDmgArchSuffix}"
			mv "${DMGLocation}/${DMGNAME}.dmg" "${DMGLocation}/${DMGNAME}+${QtDmgArchSuffix}"
		fi
	)
}

# ------------------------------
# main block
# ------------------------------

if [ ${LyxOnlyPackage:-"no"} = "no" ]; then
	build_lyx
	convert_universal
	copy_dictionaries
	test -n "${CODESIGN_IDENTITY}" && code_sign "${LYX_BUNDLE_PATH}"
	find "${LyxAppPrefix}" -type d -exec chmod go-w '{}' \;
fi
build_package
