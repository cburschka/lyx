#!/bin/sh

# set -x

# This script automates creating universal binaries of LyX on Mac.
# Author: Bennett Helm (and extended by Konrad Hofbauer)
# modified by Stephan Witt
# Last modified: 9 July 2010

#Qt4SourceVersion="qt-everywhere-opensource-src-4.7.0-beta1"
#Qt4Build="qt4.7-beta"

# Prerequisite:
# * a decent checkout of LyX sources (probably you have it already)
# * Qt4 - build with shared or static libraries for the used platforms (default: i386 and ppc)
#    or - an unpacked source tree of Qt4 in $QT4SOURCEDIR or in the sibling directory (variable Qt4SourceVersion)
# * for aspell support:
#   the aspell sources placed in a sibling directory (variable ASpellSourceVersion)
# * for hunspell support:
#   the hunspell sources placed in a sibling directory (variable HunSpellSourceVersion)
# * for dictionary deployment (per default thesauri only):
#   - aspell:   the dictionary files of macports (in /opt/local/share/aspell and /opt/local/lib/aspell-0.60)
#   - hunspell: the dictionary files in the sibling directory Dictionaries/dict
#   - mythes:   the data and idx files in the sibling directory Dictionaries/thes

LyXConfigureOptions="--enable-warnings --enable-optimization=-Os --with-included-gettext --x-includes=/usr/X11/include --x-libraries=/usr/X11/lib"
AspellConfigureOptions="--enable-warnings --enable-optimization=-O0 --enable-debug --disable-nls --enable-compile-in-filters --disable-pspell-compatibility"
HunspellConfigureOptions="--with-warnings --disable-nls --with-included-gettext --disable-static"
Qt4ConfigureOptions="-opensource -silent -shared -release -fast -no-exceptions"
Qt4ConfigureOptions="${Qt4ConfigureOptions} -no-webkit -no-qt3support -no-javascript-jit -no-dbus"
Qt4ConfigureOptions="${Qt4ConfigureOptions} -nomake examples -nomake demos -nomake docs -nomake tools"

aspell_dictionaries="no"
hunspell_dictionaries="no"

aspell_deployment="yes"
hunspell_deployment="yes"
thesaurus_deployment="yes"

qt4_deployment="yes"

MACOSX_DEPLOYMENT_TARGET="10.4" # Tiger support is default
SDKROOT="/Developer/SDKs/MacOSX10.5.sdk" # Leopard build is default

usage() {
	echo Build script for LyX on Mac OS X
	echo
	echo Optional arguments:
	echo " --aspell-deployment=yes|no ." default yes
	echo " --qt4-deployment=yes|no ...." default yes
	echo " --with-macosx-target=TARGET " default 10.4 "(Tiger)"
	echo " --with-sdkroot=SDKROOT ....." default 10.5 "(Leopard)"
	echo " --with-arch=ARCH ..........." default ppc,i386
	echo " --with-build-path=PATH ....." default \${lyx-src-dir}/../lyx-build
	echo " --with-dmg-location=PATH ..." default \${build-path}
	echo
	echo "All other arguments with -- are passed to configure"
	echo "including the defaults: ${LyXConfigureOptions}"
	echo
	exit 0
}

while [ $# -gt 0 ]; do
	case "${1}" in
	--with-qt4-dir=*)
		QTDIR=`echo ${1}|cut -d= -f2`
		shift
		;;
	--with-macosx-target=*)
		MACOSX_DEPLOYMENT_TARGET=`echo ${1}|cut -d= -f2`
		shift
		;;
	--with-sdkroot=*)
		SDKROOT=`echo ${1}|cut -d= -f2`
		case "${SDKROOT}" in
		10.4)
			SDKROOT="/Developer/SDKs/MacOSX10.4u.sdk"
			;;
		10.5|10.6)
			SDKROOT="/Developer/SDKs/MacOSX${SDKROOT}.sdk"
			;;
		*)
			usage
			;;
		esac
		shift
		;;
	--aspell-deployment=*)
		aspell_deployment=`echo ${1}|cut -d= -f2`
		shift
		;;
	--hunspell-deployment=*)
		hunspell_deployment=`echo ${1}|cut -d= -f2`
		shift
		;;
	--thesaurus-deployment=*)
		thesaurus_deployment=`echo ${1}|cut -d= -f2`
		shift
		;;
	--qt4-deployment=*)
		qt4_deployment=`echo ${1}|cut -d= -f2`
		shift
		;;
	--with-arch=*)
		ARCH=`echo ${1}|cut -d= -f2|tr ',' ' '`
		ARCH_LIST="${ARCH_LIST} ${ARCH}"
		shift
		;;
	--with-dmg-location=*)
		DMGLocation=`echo ${1}|cut -d= -f2`
		shift
		;;
	--with-build-path=*)
		LyxBuildDir=`echo ${1}|cut -d= -f2`
		shift
		;;
	--help)
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
	--*)
		LyXConfigureOptions="${LyXConfigureOptions} ${1}"
		shift
		;;
	*)
		break
		;;
	esac
done

# Set these variables -- to
# (1) the location of your Qt4 installation
# (2) the location of resulting DMG
# (3) the version of private aspell installation
#     (to define the location assign ASpellSourceDir instead)
# (4) the list of architectures to build for

QtInstallDir=${QTDIR:-"/opt/qt4"}
QtFrameworkVersion="4"
ASpellSourceVersion="aspell-0.60.6"
HunSpellSourceVersion="hunspell-1.2.9"
Qt4SourceVersion=${Qt4SourceVersion:-"qt-everywhere-opensource-src-4.6.3"}

ARCH_LIST=${ARCH_LIST:-"ppc i386"}

strip="-strip"
aspellstrip=

# detection of script home
LyxSourceDir=${1:-`dirname "$0"`}
if [ ! -d "${LyxSourceDir}" ]; then
	echo Missing LyX source directory.
	exit 2
fi
case "${LyxSourceDir}" in
/*/development)
	LyxSourceDir=`dirname "${LyxSourceDir}"`
	;;
/*)
	;;
*/development|development)
	LyxSourceDir=`dirname "${LyxSourceDir}"`
	LyxSourceDir=`cd "${LyxSourceDir}";pwd`
	;;
*)
	LyxSourceDir=`cd "${LyxSourceDir}";pwd`
	;;
esac

LyxBuildDir=${LyxBuildDir:-`dirname "${LyxSourceDir}"`/lyx-build}
DMGLocation=${DMGLocation:-"${LyxBuildDir}"}

ASpellSourceDir=${ASPELLDIR:-`dirname "${LyxSourceDir}"`/${ASpellSourceVersion}}
ASpellInstallDir=${ASpellInstallDir:-"${LyxBuildDir}"/SpellChecker.lib}
HunSpellSourceDir=${HUNSPELLDIR:-`dirname "${LyxSourceDir}"`/${HunSpellSourceVersion}}
HunSpellInstallDir=${HunSpellInstallDir:-"${LyxBuildDir}"/SpellChecker.lib}
Qt4SourceDir=${QT4SOURCEDIR:-`dirname "${LyxSourceDir}"`/${Qt4SourceVersion}}
Qt4BuildDir=${Qt4BuildDir:-"${LyxBuildDir}"/${Qt4Build:-"qt4-build"}}
DictionarySourceDir=${DICTIONARYDIR:-`dirname "${LyxSourceDir}"`/Dictionaries}

ASpellInstallHdr="${ASpellInstallDir}/include/aspell.h"
HunSpellInstallHdr="${HunSpellInstallDir}/include/hunspell/hunspell.h"

if [ -z "${LyXVersion}" ]; then
	LyXVersion=`grep AC_INIT "${LyxSourceDir}"/configure.ac | cut -d, -f2 | tr -d " ()"`
fi

LyxName="LyX"
LyxBase="${LyxName}-${LyXVersion}"
LyxApp="${LyxBase}.app"
LyxAppDir="${LyxBuildDir}"/"${LyxBase}"
LyxBuildDir="${LyxAppDir}.build"
LyxAppPrefix="${LyxAppDir}.app"
# if zip file is needed... remove the comment sign
#LyxAppZip="${LyxAppPrefix}.zip"

BuildSystem=`"${LyxSourceDir}/config/config.guess"`

# ---------------------------------
# DON'T MODIFY ANYTHING BELOW HERE!
# ---------------------------------

# don't change order here...
QtLibraries="QtSvg QtXml QtGui QtNetwork QtCore"

DMGNAME="${LyxBase}"
DMGSIZE="550m"
BACKGROUND="${LyxAppDir}.app/Contents/Resources/images/banner.png"

# Check for existing SDKs
SDKs=`echo /Developer/SDKs/MacOSX10*sdk`
case "$SDKs" in
${SDKROOT})
	;;
*10.6*)
	MACOSX_DEPLOYMENT_TARGET=${MACOSX_DEPLOYMENT_TARGET:-"10.5"}; export MACOSX_DEPLOYMENT_TARGET
	case "${MACOSX_DEPLOYMENT_TARGET}" in
	10.5|10.4)
		SDKROOT=${SDKROOT:-"/Developer/SDKs/MacOSX10.5.sdk"}; export SDKROOT
		;;
	esac
	;;
*10.5*)
	MACOSX_DEPLOYMENT_TARGET=${MACOSX_DEPLOYMENT_TARGET:-"10.4"}; export MACOSX_DEPLOYMENT_TARGET
	SDKROOT=${SDKROOT:-"/Developer/SDKs/MacOSX10.5.sdk"}; export SDKROOT
	;;
*)
	echo Unknown or missing SDK for Mac OS X.
	exit 1
	;;
esac
MYCFLAGS="-mmacosx-version-min=${MACOSX_DEPLOYMENT_TARGET}"

# These variables define the identifiers of the
# system (both Intel and PowerPC) to compile for.
# (Note: darwin8 is 10.4; darwin9 is 10.5.)
# Only change these if necessary

HostSystem_i386="i686-apple-darwin8"
HostSystem_ppc="powerpc-apple-darwin8"

updateDictionaries() {
	TMP_DIR="/tmp/lyx-build-$$"
	mkdir -p "$1"/dict "$1"/thes
	mkdir -p "$TMP_DIR" && (
		for pack in "$1"/*.zip ; do
			case "${pack}" in
			*de_DE-pack.zip)
				cd "$TMP_DIR" && unzip "${pack}" de_DE_comb.zip thes_de_DE_v2.zip
				cd "$1"/dict && unzip -o "$TMP_DIR"/de_DE_comb.zip
				cd "$1"/thes && unzip -o "$TMP_DIR"/thes_de_DE_v2.zip
				;;
			*pl_PL-pack.zip)
				cd "$TMP_DIR" && unzip "${pack}" pl_PL.zip thes_pl_PL_v2.zip
				cd "$1"/dict && unzip -o "$TMP_DIR"/pl_PL.zip
				cd "$1"/thes && unzip -o "$TMP_DIR"/thes_pl_PL_v2.zip
				;;
			*fr_FR-pack.zip)
				cd "$TMP_DIR" && unzip "${pack}" fr_FR.zip thes_fr_FR_v2.zip
				cd "$1"/dict && unzip -o "$TMP_DIR"/fr_FR.zip
				cd "$1"/thes && unzip -o "$TMP_DIR"/thes_fr_FR_v2.zip
				;;
			*es_ES-pack.zip)
				cd "$TMP_DIR" && unzip "${pack}" es_ES.zip es_MX.zip thes_es_ES_v2.zip
				cd "$1"/dict && unzip -o "$TMP_DIR"/es_ES.zip
				cd "$1"/dict && unzip -o "$TMP_DIR"/es_MX.zip
				cd "$1"/thes && unzip -o "$TMP_DIR"/thes_es_ES_v2.zip
				;;
			*pt_PT-pack.zip)
				cd "$TMP_DIR" && unzip "${pack}" pt_PT.zip
				cd "$1"/dict && unzip -o "$TMP_DIR"/pt_PT.zip
				cd "$1"/dict && unzip -o "$1"/pt_BR.zip
				cd "$1"/thes && unzip -o "$1"/thes_pt_PT_v2.zip
				;;
			*it_IT-pack.zip)
				cd "$TMP_DIR" && unzip "${pack}" it_IT.zip
				cd "$1"/dict && unzip -o "$TMP_DIR"/it_IT.zip
				cd "$1"/thes && unzip -o "$1"/thes_it_IT_v2.zip
				;;
			*ru_RU-pack.zip)
				cd "$TMP_DIR" && unzip "${pack}" ru_RU.zip
				cd "$1"/dict && unzip -o "$TMP_DIR"/ru_RU.zip
				cd "$1"/thes && tar xvf "$1"/thes_ru_RU_v2.tar.bz2
				;;
			*en_EN-pack.zip)
				cd "$TMP_DIR" && unzip "${pack}" en_AU.zip en_CA.zip en_GB.zip en_NZ.zip en_US.zip
				for zipfile in en_AU.zip en_CA.zip en_GB.zip en_NZ.zip en_US.zip ; do
					( cd "$1"/dict && unzip -o "$TMP_DIR/$zipfile" )
				done
				cd "$1"/thes && unzip -o "$1"/thes_en_US_v2.zip
				;;
			XXXX*-pack*)
				cd "$TMP_DIR" && unzip -l "${pack}" | while read len date time zipfile ; do
					case "$zipfile" in
					thes*_v2.zip)
						echo "$zipfile"
						cd "$TMP_DIR" && unzip -o "${pack}" "$zipfile"
						cd "$1"/thes && unzip -o "$TMP_DIR"/"$zipfile"
						;;
					[a-z][a-z]_[A-Z][A-Z].zip)
						echo "$zipfile"
						cd "$TMP_DIR" && unzip -o "${pack}" "$zipfile"
						cd "$1"/dict && unzip -o "$TMP_DIR"/"$zipfile"
						;;
					esac
				done
				# echo Ignore dictionary package `basename "${pack}"`
				;;
			esac
		done
	)
	rm -rf "$TMP_DIR"
}

if [ -d "${Qt4SourceDir}" -a ! -d "${Qt4BuildDir}" ]; then
	echo Build Qt4 library ${Qt4SourceDir}

	(
		mkdir -p "${Qt4BuildDir}" && cd "${Qt4BuildDir}"
		for arch in ${ARCH_LIST} ; do
			ARCHS="${ARCHS} -arch ${arch}"
		done
		echo configure options:
		echo ${Qt4ConfigureOptions} ${ARCHS} -prefix "${QtInstallDir}"

		echo yes | "${Qt4SourceDir}"/configure ${Qt4ConfigureOptions} ${ARCHS} -prefix "${QtInstallDir}"
		make && make install
	)
	cd "${QtInstallDir}" && (
		mkdir -p include
		cd include
		for libnm in ${QtLibraries} ; do
			test -d ${libnm} -o -L ${libnm} || ln -s ../lib/${libnm}.framework/Headers ${libnm}
		done
	)
fi

# updateDictionaries "${DictionarySourceDir}"
# exit

if [ -d "${HunSpellSourceDir}" -a ! -f "${HunSpellInstallHdr}" ]; then
	# we have a private HunSpell source tree at hand...
	# so let's build and install it
	if [ -z "${HunSpellVersion}" ]; then
		HunSpellVersion=`grep AC_INIT "${HunSpellSourceDir}"/configure.ac | cut -d, -f2|tr -d " ()"`
	fi

	HunSpellName="Hunspell"
	HunSpellBase="${HunSpellName}-${HunSpellVersion}"

	echo Build hunspell library ${HunSpellBase}
	echo configure options:
	echo --prefix="${HunSpellInstallDir}" ${HunspellConfigureOptions}

	cd "${HunSpellSourceDir}"

	# ----------------------------------------
	# Build HunSpell for different architectures
	# ----------------------------------------
	FILE_LIST="libhunspell-1.2.0.dylib"

	for arch in ${ARCH_LIST} ; do
		make distclean
		CPPFLAGS="${SDKROOT:+-isysroot ${SDKROOT}} -arch ${arch} ${MYCFLAGS}"; export CPPFLAGS
		LDFLAGS="${SDKROOT:+-isysroot ${SDKROOT}} -arch ${arch} ${MYCFLAGS}"; export LDFLAGS
		HOSTSYSTEM=`eval "echo \\$HostSystem_$arch"`
		"${HunSpellSourceDir}/configure"\
			--prefix="${HunSpellInstallDir}"\
			${HunspellConfigureOptions}
			#--host="${HOSTSYSTEM}" ${BuildSystem:+"--build=${BuildSystem}"}
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
			test -f lib/libhunspell.dylib || (cd lib ; ln -s libhunspell-1.2.dylib libhunspell.dylib)
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
		ASpellVersion=`grep AC_INIT "${ASpellSourceDir}"/configure.ac | cut -d, -f2|tr -d " ()"`
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
	FILE_LIST="libaspell.15.dylib"

	for arch in ${ARCH_LIST} ; do
		make distclean
		CPPFLAGS="${SDKROOT:+-isysroot ${SDKROOT}} -arch ${arch} ${MYCFLAGS}"; export CPPFLAGS
		LDFLAGS="${SDKROOT:+-isysroot ${SDKROOT}} -arch ${arch} ${MYCFLAGS}"; export LDFLAGS
		HOSTSYSTEM=`eval "echo \\$HostSystem_$arch"`
		CXXFLAGS=-g "${ASpellSourceDir}/configure"\
			--prefix="${ASpellInstallDir}"\
			${AspellConfigureOptions}
			#--host="${HOSTSYSTEM}" ${BuildSystem:+"--build=${BuildSystem}"}
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

# exit 0


framework_name() {
	echo "Frameworks/${1}.framework"
}

if [ ! -f "${LyxSourceDir}"/configure -o "${LyxSourceDir}"/configure -ot "${LyxSourceDir}"/configure.ac ]; then
	( cd "${LyxSourceDir}" && sh autogen.sh )
fi

FILE_LIST="lyx lyxclient tex2lyx"
BUNDLE_PATH="Contents/MacOS"
LYX_BUNDLE_PATH="${LyxAppPrefix}/${BUNDLE_PATH}"
build_lyx() {
	# Clear Output
	if [ -n "${LyxAppZip}" -a -f "${LyxAppZip}" ]; then rm "${LyxAppZip}"; fi
	if [ -d "${LyxAppPrefix}" ]; then rm -rf "${LyxAppPrefix}"; fi

	# -------------------------------------
	# Build LyX for different architectures
	# -------------------------------------

	if [ -d "${ASpellInstallDir}" -a "yes" = "${aspell_deployment}" ]; then
		ASpellFramework=`framework_name Aspell`
		ASpellFramework=`basename "${ASpellFramework}"`
		ConfigureExtraInc="--with-extra-inc=${ASpellInstallDir}/include"
		ConfigureExtraLib="--with-extra-lib=${ASpellInstallDir}/lib"
		LyXConfigureOptions="${LyXConfigureOptions} --with-aspell-framework=${ASpellFramework}"
	fi

	if [ -d "${HunSpellInstallDir}" -a "yes" = "${hunspell_deployment}" ]; then
		HunSpellFramework=`framework_name Hunspell`
		HunSpellFramework=`basename "${HunSpellFramework}"`
		ConfigureExtraInc="--with-extra-inc=${HunSpellInstallDir}/include"
		ConfigureExtraLib="--with-extra-lib=${HunSpellInstallDir}/lib"
		# LyXConfigureOptions="${LyXConfigureOptions} --with-hunspell-framework=${HunSpellFramework}"
	fi
	LyXConfigureOptions="${LyXConfigureOptions} ${ConfigureExtraInc}"
	LyXConfigureOptions="${LyXConfigureOptions} ${ConfigureExtraLib}"

	for arch in ${ARCH_LIST} ; do

		if [ -d "${LyxBuildDir}" ];  then rm -r "${LyxBuildDir}"; fi
		mkdir "${LyxBuildDir}" && cd "${LyxBuildDir}"

		CPPFLAGS="${SDKROOT:+-isysroot ${SDKROOT}} -arch ${arch} ${MYCFLAGS}"; export CPPFLAGS
		LDFLAGS="${SDKROOT:+-isysroot ${SDKROOT}} -arch ${arch} ${MYCFLAGS}"; export LDFLAGS
		HOSTSYSTEM=`eval "echo \\$HostSystem_$arch"`

		echo LDFLAGS="${LDFLAGS}"
		echo CPPFLAGS="${CPPFLAGS}"
		echo CONFIGURE_OPTIONS="${LyXConfigureOptions}"
		"${LyxSourceDir}/configure"\
			--prefix="${LyxAppPrefix}" --with-version-suffix="-${LyXVersion}"\
			${QtInstallDir:+"--with-qt4-dir=${QtInstallDir}"} \
			${LyXConfigureOptions}\
			--host="${HOSTSYSTEM}" --build="${BuildSystem}" --enable-build-type=rel && \
		make && make install${strip}
		for file in ${FILE_LIST} ; do
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
	content=`dirname "${target}"`
	content=`dirname "${content}"`
	echo "${content}"
}

private_framework() {
	fwdir=`framework_name "$1"`
	source="$2"
	target="$3"
	condir=`content_directory "${target}"`
	libnm=`basename "${source}"`
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
	condir=`content_directory "${target}"`
	mkdir -p "${condir}/Resources"
	test -f "${condir}/Resources/qt.conf" || cat - > "${condir}/Resources/qt.conf" <<-EOF
[Paths]
Plugins = PlugIns
EOF
	if [ ! -d "${condir}/PlugIns" ]; then
		mkdir -p "${condir}/PlugIns"
		find "${source}/plugins" -name \*.dylib -print | while read libname ; do
			echo Copy plugin "${libname}"
			dylib=`basename "${libname}"`
			dirname=`dirname "${libname}"`
			dirname=`basename "${dirname}"`
			mkdir -p "${condir}/PlugIns/${dirname}"
			cp -p "${libname}" "${condir}/PlugIns/${dirname}"
		done
	fi
	for libnm in ${QtLibraries} ; do
		fwdir=`framework_name "$libnm"`
		dirname=`basename "${fwdir}"`
		test -d "${condir}/${fwdir}" || (
			echo Copy framework "${source}/lib/"`basename "${fwdir}"`
			cp -pR "${source}/lib/"`basename "${fwdir}"` "${condir}/${fwdir}"
			echo Set library id in "${condir}/${fwdir}/${version}${libnm}"
			install_name_tool -id "@executable_path/../${fwdir}/${version}${libnm}" "${condir}/${fwdir}/${version}${libnm}"
			find "${condir}/PlugIns" "${condir}/"`dirname "${fwdir}"` -name Headers -prune -o -type f -print | while read filename ; do
				otool -L "${filename}" 2>/dev/null | while read library ; do
					# pattern match for: /path/to/qt4/lib/QtGui.framework/Versions/4/QtGui (compatibility version 4.6.0, current version 4.6.2)
					case "${library}" in
					*"${libnm}"*"("*")"*)
						echo Correct library id reference to "${libnm}" in "${filename}"
						install_name_tool -change\
							"${source}/lib/${dirname}/${version}${libnm}"\
							"@executable_path/../${fwdir}/${version}${libnm}"\
							"${filename}"
						;;
					esac
				done
			done
		)
		echo Correct library id reference to "${libnm}" in "${target}"
		install_name_tool -change\
			"${source}/lib/${dirname}/${version}${libnm}"\
			"@executable_path/../${fwdir}/${version}${libnm}"\
			"${target}"
	done
}

# -------------------------
# Create universal binaries
# -------------------------
convert_universal() {
	cd "${LyxAppPrefix}"
	for file in ${FILE_LIST} ; do
		OBJ_LIST=
		for arch in ${ARCH_LIST} ; do
			if [ -f "${BUNDLE_PATH}/${file}-${arch}" ]; then
				OBJ_LIST="${OBJ_LIST} ${BUNDLE_PATH}/${file}-${arch}"
			fi
		done
		if [ -n "${OBJ_LIST}" ]; then
			lipo -create ${OBJ_LIST} -o "${BUNDLE_PATH}/${file}"
		fi
		if [ -d "${ASpellInstallDir}" -a "yes" = "${aspell_deployment}" ]; then
			private_framework Aspell "${ASpellInstallDir}/lib/libaspell.15.dylib" "${LYX_BUNDLE_PATH}/${file}"
		fi
		if [ -d "${HunSpellInstallDir}" -a "yes" = "${hunspell_deployment}" ]; then
			private_framework Hunspell "${HunSpellInstallDir}/lib/libhunspell-1.2.0.dylib" "${LYX_BUNDLE_PATH}/${file}"
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

copy_dictionaries() {
	if [ -d "${ASpellInstallDir}" -a "yes" = "${aspell_dictionaries}" ]; then
		ASpellResources="${LyxAppPrefix}/Contents/Resources"
		# try to reuse macports dictionaries for now
		if [ -d /opt/local/lib/aspell-0.60 ]; then ASpellInstallDir=/opt/local ; fi
		mkdir -p "${ASpellResources}"
		echo Copy Aspell dictionaries from "${ASpellInstallDir}"
		mkdir -p "${ASpellResources}"/data "${ASpellResources}"/dict
		cp -p -r "${ASpellInstallDir}/lib/aspell-0.60"/* "${ASpellResources}"/data
		cp -p -r "${ASpellInstallDir}/share/aspell"/* "${ASpellResources}"/dict
	fi
	if [ -d "${HunSpellInstallDir}" -a "yes" = "${hunspell_dictionaries}" ]; then
		HunSpellResources="${LyxAppPrefix}/Contents/Resources"
		if [ -d "${DictionarySourceDir}" ]; then
			updateDictionaries "${DictionarySourceDir}"
			cp -p -r "${DictionarySourceDir}/dict" "${HunSpellResources}"
		fi
	fi
	if [ -d "${DictionarySourceDir}" -a "yes" = "${thesaurus_deployment}" ]; then
		MyThesResources="${LyxAppPrefix}/Contents/Resources"
		cp -p -r "${DictionarySourceDir}/thes" "${MyThesResources}"
	fi
}

function set_bundle_display_options() {
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
                set the bounds to {20, 50, $2, $3}
            end tell
            delay 1 -- sync
            set icon size of the icon view options of container window to 64
            set arrangement of the icon view options of container window to not arranged
            set position of item "${LyxName}.app" to {100,$4}
            set position of item "Applications" to {280,$4}
            set background picture of the icon view options\
					of container window to file "background.png" of folder "Pictures"
            set the bounds of the container window to {0, 0, $2, $3}
            update without registering applications
            delay 5 -- sync
            close
        end tell
        delay 5 -- sync
    end tell
EOF
}

function make_dmg() {
	cd "${1}"

	BGSIZE=`file "$BACKGROUND" | awk -F , '/PNG/{print $2 }' | tr x ' '`
	BG_W=`echo ${BGSIZE} | awk '{print $1 }'`
	BG_H=`echo ${BGSIZE} | awk '{h = $2 + 20 ;print h }'`
	BG_Y=`echo ${BGSIZE} | awk '{y = $2 - 60 ;print y }'`

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
	cp "${BACKGROUND}" "${VOLUME}/Pictures/background.png"
	# symlink applications
	ln -s /Applications/ "${VOLUME}"/Applications
	set_bundle_display_options "${VOLUME}" ${BG_W} ${BG_H} ${BG_Y}
	mv "${VOLUME}/Pictures" "${VOLUME}/.Pictures"

	# Unmount the disk image
	hdiutil detach ${DEV}

	# Convert the disk image to read-only
	hdiutil convert "${DMGNAME}.sparseimage" -format UDBZ -o "${DMGNAME}.dmg"
	rm -f "${DMGNAME}.sparseimage"
}

build_lyx
convert_universal
copy_dictionaries

# ------------------------------
# Building distribution packages
# ------------------------------

test -n "${LyxAppZip}" && (
	cd "${LyxAppPrefix}" && zip -r "${LyxAppZip}" .
)

test -n "${DMGLocation}" && (
	make_dmg "${DMGLocation}"
	if [ -d "${QtInstallDir}/lib/QtCore.framework/Versions/${QtFrameworkVersion}" -a "yes" = "${qt4_deployment}" ]; then
		rm -f "${DMGLocation}/${DMGNAME}+qt4.dmg"
		mv "${DMGLocation}/${DMGNAME}.dmg" "${DMGLocation}/${DMGNAME}+qt4.dmg"
		#for libnm in ${QtLibraries} ; do
		#	fwdir=`framework_name "$libnm"`
		#	rm -rf "${LyxAppDir}.app/Contents/${fwdir}"
		#done
		#make_dmg "${DMGLocation}"
	fi
)
