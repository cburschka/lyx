#!/bin/sh

# set -x

# This script automates creating universal binaries of LyX on Mac.
# Author: Bennett Helm (and extended by Konrad Hofbauer)
# latest changes by Stephan Witt
# Last modified: January 2013

Qt4API=${Qt4API:-"-cocoa"}
Qt4Version=${Qt4Version:-"4.6.3"}
Qt4SourceVersion="qt-everywhere-opensource-src-${Qt4Version}"
Qt4BuildSubDir="qt-${Qt4Version}-build${Qt4API}"
Qt4ConfigureOptions=${Qt4ConfigureOptions:-"-release"}

GettextVersion=${GettextVersion:-"0.18.2"}
GettextSource="gettext-${GettextVersion}"
GettextLibrary="libintl.8.dylib"

ASpellVersion=${ASpellVersion:-"0.60.6.1"}
ASpellSource="aspell-${ASpellVersion}"
ASpellLibrary="libaspell.15.dylib"

HunSpellVersion=${HunSpellVersion:-"1.3.2"}
HunSpellSource="hunspell-${HunSpellVersion}"
HunSpellLibrary="libhunspell-1.3.0.dylib"

unset DYLD_LIBRARY_PATH LD_LIBRARY_PATH

# Prerequisite:
# * a decent checkout of LyX sources (probably you have it already)
# * Qt4 - build with shared or static libraries for the used platforms (default: i386 and ppc)
#    or - an unpacked source tree of Qt4 in $QT4SOURCEDIR or in the sibling directory (variable Qt4SourceVersion)
# * for aspell support:
#   the aspell sources placed in a sibling directory (variable ASpellSource)
# * for hunspell support:
#   the hunspell sources placed in a sibling directory (variable HunSpellSource)
# * for dictionary deployment (per default thesauri only):
#   - aspell:   the dictionary files of macports (in /opt/local/share/aspell and /opt/local/lib/aspell-0.60)
#   - hunspell: the dictionary files in the sibling directory dictionaries/dicts
#   - mythes:   the data and idx files in the sibling directory dictionaries/thes

LyXConfigureOptions="--enable-warnings --enable-optimization=-Os --with-x=no"
LyXConfigureOptions="${LyXConfigureOptions} --disable-stdlib-debug"
AspellConfigureOptions="--enable-warnings --enable-optimization=-O0 --enable-debug --disable-nls --enable-compile-in-filters --disable-pspell-compatibility"
HunspellConfigureOptions="--with-warnings --disable-nls --disable-static"
Qt4ConfigureOptions="${QtConfigureOptions} -opensource -silent -shared -fast -no-exceptions"
Qt4ConfigureOptions="${Qt4ConfigureOptions} -no-webkit -no-qt3support -no-javascript-jit -no-dbus"
Qt4ConfigureOptions="${Qt4ConfigureOptions} -nomake examples -nomake demos -nomake docs -nomake tools"
Qt4DmgSuffix=qt4${Qt4API}.dmg

# stupid special case...
case "${Qt4Version}:${Qt4API}" in
4.6*:-carbon)
	;;
5.0*)
	Qt4ConfigureOptions="${QtConfigureOptions} -opensource -silent -shared -fast -no-strip"
	Qt4ConfigureOptions="${Qt4ConfigureOptions} -no-javascript-jit -no-pkg-config"
	Qt4ConfigureOptions="${Qt4ConfigureOptions} -nomake examples -nomake demos -nomake docs -nomake tools"
	Qt4DmgSuffix=qt5${Qt4API}.dmg
	;;
5.*)
	Qt4DmgSuffix=qt5${Qt4API}.dmg
	;;
*)
	Qt4ConfigureOptions="${Qt4ConfigureOptions} ${Qt4API}"
	;;
esac

aspell_dictionaries="no"
hunspell_dictionaries="yes"

gettext_deployment="no"
aspell_deployment="yes"
hunspell_deployment="yes"
thesaurus_deployment="yes"

qt4_deployment="yes"

# auto detect Xcode location
if [ -d "/Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs" ]; then
	DEVELOPER_SDKS="/Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs"
	XCODE_DEVELOPER="/Applications/Xcode.app/Contents/Developer"
	MACOSX_DEPLOYMENT_TARGET="10.7" # Lion support is default
	SDKROOT="${DEVELOPER_SDKS}/MacOSX10.7.sdk" # Lion build is default
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
	echo " --with-qt4-frameworks=yes|no" default no
	echo " --qt4-deployment=yes|no ...." default yes
	echo " --with-macosx-target=TARGET " default 10.4 "(Tiger)"
	echo " --with-sdkroot=SDKROOT ....." default 10.5 "(Leopard)"
	echo " --with-arch=ARCH ..........." default ppc,i386
	echo " --with-build-path=PATH ....." default \${lyx-src-dir}/../lyx-build
	echo " --with-dmg-location=PATH ..." default \${build-path}
	echo
	echo "All other arguments with -- are passed to configure"
	echo "including the defaults: ${LyXConfigureOptions}"
	if [ -x "${LyxSourceDir}/configure" ]; then
		echo
		echo "*" Configure options of LyX
		echo
		"${LyxSourceDir}/configure" --help
	fi
	exit 0
}

NCPU=$(sysctl -n hw.ncpu)
NCPU=$((NCPU / 2))
if [ $NCPU -gt 1 ]; then
	MAKEJOBS=-j${NCPU}
fi

while [ $# -gt 0 ]; do
	case "${1}" in
	--with-qt4-frameworks=*)
		configure_qt4_frameworks=$(echo ${1}|cut -d= -f2)
		if [ "$configure_qt4_frameworks" = "yes" ]; then
			unset QTDIR
			qt4_deployment="no"
		fi
		shift
		;;
	--with-qt4-dir=*)
		QTDIR=$(echo ${1}|cut -d= -f2)
		shift
		;;
	--with-macosx-target=*)
		MACOSX_DEPLOYMENT_TARGET=$(echo ${1}|cut -d= -f2)
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
		10.5|10.6|10.7|10.8)
			SDKROOT="${DEVELOPER_SDKS}/MacOSX${SDKROOT}.sdk"
			;;
		*)
			usage
			;;
		esac
		shift
		;;
	--gettext-deployment=*)
		gettext_deployment=$(echo ${1}|cut -d= -f2)
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
	--qt4-deployment=*)
		qt4_deployment=$(echo ${1}|cut -d= -f2)
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
	--with-build-path=*)
		LyxBuildDir=$(echo ${1}|cut -d= -f2)
		shift
		;;
	--with-util-dir=*)
		LyXUtilitiesDir=$(echo ${1}|cut -d= -f2)
		shift
		;;
	--help|--help=*)
		usage
		;;
	--without-aspell)
		LyXConfigureOptions="${LyXConfigureOptions} ${1}"
		aspell_deployment="no"
		shift
		;;
	--without-hunspell)
		LyXConfigureOptions="${LyXConfigureOptions} ${1}"
		hunspell_deployment="no"
		shift
		;;
	--only-package=*)
		LyxOnlyPackage=$(echo ${1}|cut -d= -f2)
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

if [ "${configure_qt4_frameworks}" != "yes" ]; then
	QtInstallDir=${QTDIR:-"/opt/qt4"}
fi

ARCH_LIST=${ARCH_LIST:-"ppc i386"}

strip="-strip"
aspellstrip=

LyxBuildDir=${LyxBuildDir:-$(dirname "${LyxSourceDir}")/lyx-build}
DMGLocation=${DMGLocation:-"${LyxBuildDir}"}

LyXUtilitiesDir=${LyXUtilitiesDir:-"${LyxBuildDir}"/utilities}

GettextSourceDir=${GETTEXTDIR:-$(dirname "${LyxSourceDir}")/${GettextSource}}
GettextBuildDir="${LyxBuildDir}"/"${GettextSource}"
GettextInstallDir=${GettextInstallDir:-"${LyXUtilitiesDir}"}
GettextInstallHdr="${GettextInstallDir}/include/libintl.h"

ASpellSourceDir=${ASPELLDIR:-$(dirname "${LyxSourceDir}")/${ASpellSource}}
ASpellBuildDir="${ASpellSourceDir}"
ASpellInstallDir=${ASpellInstallDir:-"${LyXUtilitiesDir}"}
ASpellInstallHdr="${ASpellInstallDir}/include/aspell.h"

HunSpellSourceDir=${HUNSPELLDIR:-$(dirname "${LyxSourceDir}")/${HunSpellSource}}
HunSpellBuildDir="${HunSpellSourceDir}"
HunSpellInstallDir=${HunSpellInstallDir:-"${LyXUtilitiesDir}"}
HunSpellInstallHdr="${HunSpellInstallDir}/include/hunspell/hunspell.h"

Qt4SourceDir=${QT4SOURCEDIR:-$(dirname "${LyxSourceDir}")/${Qt4SourceVersion}}
Qt4BuildDir=${Qt4BuildDir:-"${LyxBuildDir}"/${Qt4BuildSubDir:-"qt4-build"}}

DictionarySourceDir=${DICTIONARYDIR:-$(dirname "${LyxSourceDir}")/dictionaries}
DocumentationDir=$(dirname "${LyxSourceDir}")/Documents
DmgBackground="${LyxSourceDir}"/development/MacOSX/dmg-background.png

if [ -z "${LyXVersion}" ]; then
	LyXVersion=$(grep AC_INIT "${LyxSourceDir}"/configure.ac | cut -d, -f2 | tr -d " ()")
fi
LyXVersionSuffix=${LyXVersionSuffix:-$(echo "${LyXVersion}" | cut -d. -f1-2)}

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
case "${Qt4Version}" in
5.0.*|5.1.*)
	QtLibraries="QtSvg QtXml QtPrintSupport QtWidgets QtGui QtNetwork QtConcurrent QtCore"
	QtFrameworkVersion="5"
	;;
5*)
	QtLibraries="QtSvg QtXml QtPrintSupport QtMacExtras QtWidgets QtGui QtNetwork QtConcurrent QtCore"
	QtFrameworkVersion="5"
	;;
*)
	QtLibraries="QtSvg QtXml QtGui QtNetwork QtCore"
	QtFrameworkVersion="4"
	;;
esac

DMGNAME="${LyxBase}"
DMGSIZE="550m"

# Check for existing SDKs
SDKs=$(echo ${DEVELOPER_SDKS}/MacOSX10*sdk)
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

# pkg-config is not usable with this script
export PKG_CONFIG=""

# These variables define the identifiers of the
# system (both Intel and PowerPC) to compile for.
# (Note: darwin8 is 10.4; darwin9 is 10.5.)
# Only change these if necessary

HostSystem_i386="i686-apple-darwin8"
HostSystem_ppc="powerpc-apple-darwin8"

if [ "${configure_qt4_frameworks}" != "yes" -a -d "${Qt4SourceDir}" -a ! \( -d "${Qt4BuildDir}" -a -d "${QtInstallDir}" \) ]; then
	echo Build Qt4 library ${Qt4SourceDir}
	if [ "${QtInstallDir}" = "${Qt4BuildDir}" ]; then
		echo Bad install directory for Qt.
		echo Must be different from build directory "${Qt4BuildDir}".
		exit 1
	fi
	(
		mkdir -p "${Qt4BuildDir}" && cd "${Qt4BuildDir}"
		for arch in ${ARCH_LIST} ; do
			ARCHS="${ARCHS} -arch ${arch}"
		done
		echo configure options:
		echo ${Qt4ConfigureOptions} ${ARCHS} -prefix "${QtInstallDir}"
		echo yes | "${Qt4SourceDir}"/configure ${Qt4ConfigureOptions} ${ARCHS} -prefix "${QtInstallDir}"
		make ${MAKEJOBS} && make install
	)
	cd "${QtInstallDir}" && (
		mkdir -p include
		cd include
		for libnm in ${QtLibraries} ; do
			test -d ${libnm} -o -L ${libnm} || ln -s ../lib/${libnm}.framework/Headers ${libnm}
		done
	)
fi

if [ -d "${GettextSourceDir}" -a ! -f "${GettextInstallHdr}" ]; then
	# we have a private Gettext source tree at hand...
	# so let's build and install it
	if [ -z "${GettextVersion}" ]; then
		GettextVersion=$(grep AC_INIT "${GettextSourceDir}"/configure.ac | cut -d, -f2|tr -d " ()")
	fi

	GettextName="Gettext"
	GettextBase="${GettextName}-${GettextVersion}"

	echo Build gettext library ${GettextBase}
	echo configure options:
	echo --prefix="${GettextInstallDir}" ${GettextConfigureOptions}

	mkdir -p "${GettextBuildDir}" && cd "${GettextBuildDir}"

	# ----------------------------------------
	# Build Gettext for different architectures
	# ----------------------------------------
	FILE_LIST="${GettextLibrary}"

	for arch in ${ARCH_LIST} ; do
		CPPFLAGS="${SDKROOT:+-isysroot ${SDKROOT}} -arch ${arch} ${MYCFLAGS}"; export CPPFLAGS
		LDFLAGS="${SDKROOT:+-isysroot ${SDKROOT}} -arch ${arch} ${MYCFLAGS}"; export LDFLAGS
		HOSTSYSTEM=$(eval "echo \\$HostSystem_$arch")
		"${GettextSourceDir}/configure"\
			--prefix="${GettextInstallDir}"\
			${GettextConfigureOptions}
		make && make install${strip}
		for file in ${FILE_LIST} ; do
			if [ -f "${GettextInstallDir}"/lib/${file} ]; then
				mv "${GettextInstallDir}"/lib/${file}\
					"${GettextInstallDir}"/lib/${file}-${arch} 
			else
				echo Cannot build and install Gettext for ${arch}.
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
			cd "${GettextInstallDir}"
			lipo -create ${OBJ_LIST} -o lib/${file}
		)
	done
	# --------
	# Clean up
	# --------
	for arch in ${ARCH_LIST} ; do
		rm -f "${GettextInstallDir}"/lib/*-${arch}
	done
fi

if [ -d "${HunSpellSourceDir}" -a ! -f "${HunSpellInstallHdr}" ]; then
	# we have a private HunSpell source tree at hand...
	# so let's build and install it
	if [ -z "${HunSpellVersion}" ]; then
		HunSpellVersion=$(grep AC_INIT "${HunSpellSourceDir}"/configure.ac | cut -d, -f2|tr -d " ()")
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
		LDFLAGS="${SDKROOT:+-isysroot ${SDKROOT}} -arch ${arch} ${MYCFLAGS}"; export LDFLAGS
		HOSTSYSTEM=$(eval "echo \\$HostSystem_$arch")
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
		ASpellVersion=$(grep AC_INIT "${ASpellSourceDir}"/configure.ac | cut -d, -f2|tr -d " ()")
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
		LDFLAGS="${SDKROOT:+-isysroot ${SDKROOT}} -arch ${arch} ${MYCFLAGS}"; export LDFLAGS
		HOSTSYSTEM=$(eval "echo \\$HostSystem_$arch")
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

LYX_FILE_LIST="lyx lyxclient tex2lyx"
BUNDLE_PATH="Contents/MacOS"
LYX_BUNDLE_PATH="${LyxAppPrefix}/${BUNDLE_PATH}"
build_lyx() {
	# Clear Output
	if [ -n "${LyxAppZip}" -a -f "${LyxAppZip}" ]; then rm "${LyxAppZip}"; fi
	if [ -d "${LyxAppPrefix}" ]; then rm -rf "${LyxAppPrefix}"; fi

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

		CPPFLAGS="${SDKROOT:+-isysroot ${SDKROOT}} -arch ${arch} ${MYCFLAGS}"
		LDFLAGS="${SDKROOT:+-isysroot ${SDKROOT}} -arch ${arch} ${MYCFLAGS}"
		HOSTSYSTEM=$(eval "echo \\$HostSystem_$arch")

		if [ "$configure_qt4_frameworks" = "yes" ]; then
			export QT4_CORE_CFLAGS="-FQtCore"
			export QT4_CORE_LIBS="-framework QtCore"
			export QT4_FRONTEND_CFLAGS="-FQtGui"
			export QT4_FRONTEND_LIBS="-framework QtGui"
			CPPFLAGS="${CPPFLAGS} -I${SDKROOT}/Library/Frameworks/QtCore.framework/Headers"
			CPPFLAGS="${CPPFLAGS} -I${SDKROOT}/Library/Frameworks/QtGui.framework/Headers"
		fi

		echo LDFLAGS="${LDFLAGS}"
		export LDFLAGS
		echo CPPFLAGS="${CPPFLAGS}"
		export CPPFLAGS
		echo CONFIGURE_OPTIONS="${LyXConfigureOptions}" ${QtInstallDir:+"--with-qt4-dir=${QtInstallDir}"}
		"${LyxSourceDir}/configure"\
			--prefix="${LyxAppPrefix}" --with-version-suffix="-${LyXVersionSuffix}"\
			${QtInstallDir:+"--with-qt4-dir=${QtInstallDir}"} \
			${LyXConfigureOptions}\
			--enable-build-type=rel && \
		make ${MAKEJOBS} && make install${strip}
		for file in ${LYX_FILE_LIST} ; do
			if [ -f "${LYX_BUNDLE_PATH}/${file}" ]; then
				mv "${LYX_BUNDLE_PATH}/${file}"\
					"${LYX_BUNDLE_PATH}/${file}-${arch}" 
			else
				echo ERROR: Cannot build and install LyX for ${arch}.
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

private_framework() {
	fwdir=$(framework_name "$1")
	source="$2"
	target="$3"
	condir=$(content_directory "${target}")
	libnm=$(basename "${source}")
	mkdir -p "${condir}/${fwdir}"
	if [ ! -f "${condir}/${fwdir}/${libnm}" ]; then
		cp -p "${source}" "${condir}/${fwdir}"
		echo Set library id in "${condir}/${fwdir}/${libnm}"
		install_name_tool -id "@executable_path/../${fwdir}/${libnm}" "${condir}/${fwdir}/${libnm}"
	fi
	echo Correct library id reference to "${libnm}" in "${target}"
	install_name_tool -change "${source}" "@executable_path/../${fwdir}/${libnm}" "${target}"
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
			echo Set library id in "${condir}/${fwdir}/${version}${libnm}"
			install_name_tool -id "@executable_path/../${fwdir}/${version}${libnm}" "${condir}/${fwdir}/${version}${libnm}"
			find "${condir}/PlugIns" "${condir}/"$(dirname "${fwdir}") -name Headers -prune -o -type f -print | while read filename ; do
				if [ "${filename}" != "${target}" ]; then
					otool -L "${filename}" 2>/dev/null | sort -u | while read library ; do
						# pattern match for: /path/to/qt4/lib/QtGui.framework/Versions/4/QtGui (compatibility version 4.6.0, current version 4.6.2)
						case "${library}" in
						*"${libnm}"*"("*version*")"*)
							echo Correct library id reference to "${libnm}" in "${filename}"
							install_name_tool -change\
								"${source}/lib/${dirname}/${version}${libnm}"\
								"@executable_path/../${fwdir}/${version}${libnm}"\
								"${filename}"
							;;
						esac
					done
				fi
			done
		)
		echo Correct library id reference to "${libnm}" in "${target}"
		install_name_tool -change\
			"${source}/lib/${dirname}/${version}${libnm}"\
			"@executable_path/../${fwdir}/${version}${libnm}"\
			"${target}"
	done
	if [ ! -d "${condir}/translations" ]; then
		mkdir -p "${condir}/translations"
	fi
	echo Copy Qt translations to "${condir}/translations"
	cp -p "${source}"/translations/qt_*.qm "${condir}/translations"
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
		if [ -f "${GettextInstallDir}/lib/${GettextLibrary}" -a "yes" = "${gettext_deployment}" ]; then
			private_framework Gettext "${GettextInstallDir}/lib/${GettextLibrary}" "${LYX_BUNDLE_PATH}/${file}"
		fi
		if [ -f "${ASpellInstallDir}/lib/${ASpellLibrary}" -a "yes" = "${aspell_deployment}" ]; then
			private_framework Aspell "${ASpellInstallDir}/lib/${ASpellLibrary}" "${LYX_BUNDLE_PATH}/${file}"
		fi
		if [ -f "${HunSpellInstallDir}/lib/${HunSpellLibrary}" -a "yes" = "${hunspell_deployment}" ]; then
			private_framework Hunspell "${HunSpellInstallDir}/lib/${HunSpellLibrary}" "${LYX_BUNDLE_PATH}/${file}"
		fi
		if [ -d "${QtInstallDir}/lib/QtCore.framework/Versions/${QtFrameworkVersion}" -a "yes" = "${qt4_deployment}" ]; then
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
		rm -f ${BUNDLE_PATH}/*-${arch}
	done
}

deduplicate() {
	find "$@" -type f -print | while read file ; do
		echo $(md5 -q "$file") "$file"
	done | sort | while read hash file ; do
		if [ "$phash" = "$hash" ]; then
			cmp -s "$pfile" "$file" && (
				rm "$file"
				ln -s "$pfile" "$file" && echo link for "$file" created
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
	if [ -d "${HunSpellInstallDir}" -a "yes" = "${hunspell_dictionaries}" ]; then
		HunSpellResources="${LyxAppPrefix}/Contents/Resources"
		if [ -d "${DictionarySourceDir}" ]; then
			( cd "${DictionarySourceDir}" && find dicts -name .svn -prune -o -type f -print | cpio -pmdv "${HunSpellResources}" )
			deduplicate "${HunSpellResources}"/dicts
		fi
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
	test -d /Volumes/"${LyxBase}" && umount /Volumes/"${LyxBase}"

	# Mount the disk image
	hdiutil attach "${DMGNAME}.sparseimage"

	# Obtain device information
	DEVS=$(hdiutil attach "${DMGNAME}.sparseimage" | cut -f 1)
	DEV=$(echo $DEVS | cut -f 1 -d ' ')
	VOLUME=$(mount |grep ${DEV} | cut -f 3 -d ' ')

	# copy in the application bundle
	cp -Rp "${LyxAppDir}.app" "${VOLUME}/${LyxName}.app"

	# copy in background image
	mkdir -p "${VOLUME}/Pictures"
	cp "${DmgBackground}" "${VOLUME}/Pictures/background.png"
	# symlink applications
	ln -s /Applications/ "${VOLUME}"/Applications
	test -d "${DocumentationDir}" && cp -r "${DocumentationDir}" "${VOLUME}"
	set_bundle_display_options "${VOLUME}" ${BG_W} ${BG_H}
	${XCODE_DEVELOPER}/Tools/SetFile -a C "${VOLUME}"
	mv "${VOLUME}/Pictures" "${VOLUME}/.Pictures"

	# Unmount the disk image
	hdiutil detach ${DEV}

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

	test -n "${DMGLocation}" && (
		make_dmg "${DMGLocation}"
		if [ -d "${QtInstallDir}/lib/QtCore.framework/Versions/${QtFrameworkVersion}" -a "yes" = "${qt4_deployment}" ]; then
			rm -f "${DMGLocation}/${DMGNAME}+${Qt4DmgSuffix}"
			echo move to "${DMGLocation}/${DMGNAME}+${Qt4DmgSuffix}"
			mv "${DMGLocation}/${DMGNAME}.dmg" "${DMGLocation}/${DMGNAME}+${Qt4DmgSuffix}"
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
fi
build_package
