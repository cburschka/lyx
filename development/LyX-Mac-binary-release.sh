#!/bin/sh

# set -x

# This script automates creating universal binaries of LyX on Mac.
# Author: Bennett Helm (and extended by Konrad Hofbauer)
# modified by Stephan Witt
# Last modified: 17 April 2010

# Prerequisite:
# * a decent checkout of LyX sources (probably you have it already)
# * Qt4 - build with shared or static libraries for the used platforms (i386 and ppc)
# * for aspell support:
#   the aspell sources placed in a sibling directory (variable ASpellSourceVersion)

ConfigureOptions="--enable-warnings --enable-optimization=-Os --with-included-gettext"
dict_deployment="yes"
qt4_deployment="yes"
MACOSX_DEPLOYMENT_TARGET="10.4" # Tiger support is default

usage() {
	echo Build script for LyX on Mac OS X
	echo
	echo Optional arguments:
	echo " --tiger-support=yes|no ....." default yes
	echo " --dict-deployment=yes|no ..." default yes
	echo " --qt4-deployment=yes|no ...." default yes
	echo " --with-arch=ARCH ..........." default ppc,i386
	echo " --with-build-path=PATH ....." default \${lyx-src-dir}/../lyx-build
	echo " --with-dmg-location=PATH ..." default \${build-path}
	echo
	echo "All other arguments with -- are passed to configure"
	echo "including the defaults: ${ConfigureOptions}"
	echo
	exit 0
}

while [ $# -gt 0 ]; do
	case "${1}" in
	--with-qt4-dir=*)
		QTDIR=`echo ${1}|cut -d= -f2`
		shift
		;;
	--tiger-support=[Nn][Oo])
		MACOSX_DEPLOYMENT_TARGET=""
		MYCFLAGS=""
		shift
		;;
	--dict-deployment=*)
		dict_deployment=`echo ${1}|cut -d= -f2`
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
	--*)
		ConfigureOptions="${ConfigureOptions} ${1}"
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
ARCH_LIST=${ARCH_LIST:-"ppc i386"}

strip="-strip"

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
ASpellInstallDir=${ASpellInstallDir:-"${LyxBuildDir}"/${ASpellSourceVersion}.lib}

if [ ! -f "${LyxSourceDir}"/configure ]; then
	( cd "${LyxSourceDir}" && sh autogen.sh )
fi
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

# These variables define the identifiers of the
# system (both Intel and PowerPC) to compile for.
# (Note: darwin8 is 10.4; darwin9 is 10.5.)
# Only change these if necessary

HostSystem_i386="i686-apple-darwin8"
HostSystem_ppc="powerpc-apple-darwin8"

DMGNAME="${LyxBase}-Uncompressed.dmg"
DMGSIZE="550m"
COMPRESSEDDMGNAME="${LyxBase}.dmg"
BACKGROUND="${LyxAppDir}.app/Contents/Resources/images/banner.png"

# Check for existing SDKs
SDKs=`echo /Developer/SDKs/MacOSX10*sdk`
case "$SDKs" in
*10.6*)
	MACOSX_DEPLOYMENT_TARGET=${MACOSX_DEPLOYMENT_TARGET:-"10.5"}; export MACOSX_DEPLOYMENT_TARGET
	case "${MACOSX_DEPLOYMENT_TARGET}" in
	10.5)
		SDKROOT="/Developer/SDKs/MacOSX10.5.sdk"; export SDKROOT
		;;
	10.4)
		SDKROOT="/Developer/SDKs/MacOSX10.4u.sdk"; export SDKROOT
		CC=gcc-4.0 ; export CC
		CXX=g++-4.0 ; export CXX
		OBJC=gcc-4.0 ; export OBJC
		;;
	esac
	;;
*10.5*)
	MACOSX_DEPLOYMENT_TARGET="10.4"; export MACOSX_DEPLOYMENT_TARGET
	SDKROOT="/Developer/SDKs/MacOSX10.4u.sdk"; export SDKROOT
	;;
*)
	echo Unknown or missing SDK for Mac OS X.
	exit 1
	;;
esac
MYCFLAGS="-mmacosx-version-min=${MACOSX_DEPLOYMENT_TARGET}"

if [ -d "${ASpellSourceDir}" -a ! -d "${ASpellInstallDir}" ]; then
	# we have a private ASpell source tree at hand...
	# so let's build and install it
	if [ -z "${ASpellVersion}" ]; then
		ASpellVersion=`grep AC_INIT "${ASpellSourceDir}"/configure.ac | cut -d, -f2|tr -d " ()"`
	fi

	ASpellName="Aspell"
	ASpellBase="${ASpellName}-${ASpellVersion}"

	echo Build aspell library ${ASpellBase}

	# Clear Output
	if [ -n "${ASpellLibZip}" -a -f "${ASpellLibZip}" ]; then rm "${ASpellLibZip}"; fi
	if [ -d "${ASpellInstallDir}" ]; then rm -r "${ASpellInstallDir}"; fi

	# ASpell builds inplace only :(
	cd "${ASpellSourceDir}" && make distclean

	# ----------------------------------------
	# Build ASpell for different architectures
	# ----------------------------------------
	FILE_LIST="libaspell.15.dylib libpspell.15.dylib"

	for arch in ${ARCH_LIST} ; do
		CPPFLAGS="${SDKROOT:+-isysroot ${SDKROOT}} -arch ${arch} ${MYCFLAGS}"; export CPPFLAGS
		LDFLAGS="${SDKROOT:+-isysroot ${SDKROOT}} -arch ${arch}"; export LDFLAGS
		HOSTSYSTEM=`eval "echo \\$HostSystem_$arch"`
		"${ASpellSourceDir}/configure"\
			--prefix="${ASpellInstallDir}"\
			${ConfigureOptions}\
			--host="${HOSTSYSTEM}" ${BuildSystem:+"--build=${BuildSystem}"} --enable-build-type=rel
		make && make install${strip}
		for file in ${FILE_LIST} ; do
			if [ -f "${ASpellInstallDir}"/lib/${file} ]; then
				mv "${ASpellInstallDir}"/lib/${file}\
					"${ASpellInstallDir}"/lib/${file}-${arch} 
			else
				echo Cannot build and install ASpell for ${arch}.
				exit 1
			fi
		done
		make distclean
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

	if [ -d "${ASpellInstallDir}" ]; then
		ASpellFramework=`framework_name Aspell`
		ASpellFramework=`basename "${ASpellFramework}"`
		ConfigureOptions="${ConfigureOptions} --with-extra-inc=${ASpellInstallDir}/include"
		ConfigureOptions="${ConfigureOptions} --with-extra-lib=${ASpellInstallDir}/lib"
		ConfigureOptions="${ConfigureOptions} --with-aspell-framework=${ASpellFramework}"
	fi

	for arch in ${ARCH_LIST} ; do

		if [ -d "${LyxBuildDir}" ];  then rm -r "${LyxBuildDir}"; fi
		mkdir "${LyxBuildDir}" && cd "${LyxBuildDir}"

		CPPFLAGS="${SDKROOT:+-isysroot ${SDKROOT}} -arch ${arch} ${MYCFLAGS}"; export CPPFLAGS
		LDFLAGS="${SDKROOT:+-isysroot ${SDKROOT}} -arch ${arch}"; export LDFLAGS
		HOSTSYSTEM=`eval "echo \\$HostSystem_$arch"`

		echo LDFLAGS="${LDFLAGS}"
		echo CPPFLAGS="${CPPFLAGS}"
		echo CONFIGURE_OPTIONS="${ConfigureOptions}"
		"${LyxSourceDir}/configure"\
			--prefix="${LyxAppPrefix}" --with-version-suffix="-${LyXVersion}"\
			${QtInstallDir:+"--with-qt4-dir=${QtInstallDir}"} \
			${ConfigureOptions}\
			--host="${HOSTSYSTEM}" --build="${BuildSystem}" --enable-build-type=rel
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
	# don't change order here...
	for libnm in QtSvg QtXml QtGui QtNetwork QtCore ; do
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
		if [ -d "${ASpellInstallDir}" ]; then
			private_framework Aspell "${ASpellInstallDir}/lib/libaspell.15.dylib" "${LYX_BUNDLE_PATH}/${file}"
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
	if [ -d "${ASpellInstallDir}" -a "yes" = "${dict_deployment}" ]; then
		ASpellFramework=`framework_name Aspell`
		ASpellResources="${LyxAppPrefix}/Contents/${ASpellFramework}/Resources"
		# try to reuse macports dictionaries for now
		if [ -d /opt/local/lib/aspell-0.60 ]; then ASpellInstallDir=/opt/local ; fi
		mkdir -p "${ASpellResources}"
		echo Copy Aspell dictionaries from "${ASpellInstallDir}"
		cp -p -r "${ASpellInstallDir}/lib/aspell-0.60" "${ASpellResources}"/data
		cp -p -r "${ASpellInstallDir}/share/aspell" "${ASpellResources}"/dict
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

	rm -f ${DMGNAME}
	rm -f ${COMPRESSEDDMGNAME}

	hdiutil create -type SPARSE -size ${DMGSIZE:-"250m"} -fs HFS+ -volname "${LyxBase}" "${DMGNAME}"
	# Unmount currently mounted disk image
	test -d /Volumes/"${LyxBase}" && umount /Volumes/"${LyxBase}"

	# Mount the disk image
	hdiutil attach ${DMGNAME}.sparseimage

	# Obtain device information
	DEVS=$(hdiutil attach ${DMGNAME}.sparseimage | cut -f 1)
	DEV=$(echo $DEVS | cut -f 1 -d ' ')
	VOLUME=$(mount |grep ${DEV} | cut -f 3 -d ' ')

	# copy in the application bundle
	cp -Rp ${LyxAppDir}.app ${VOLUME}/${LyxName}.app

	# copy in background image
	mkdir -p ${VOLUME}/Pictures
	cp ${BACKGROUND} ${VOLUME}/Pictures/background.png
	# symlink applications
	ln -s /Applications/ ${VOLUME}/Applications
	set_bundle_display_options ${VOLUME} ${BG_W} ${BG_H} ${BG_Y}
	mv ${VOLUME}/Pictures ${VOLUME}/.Pictures

	# Unmount the disk image
	hdiutil detach ${DEV}

	# Convert the disk image to read-only
	hdiutil convert ${DMGNAME}.sparseimage -format UDBZ -o ${COMPRESSEDDMGNAME}
	rm -f ${DMGNAME}.sparseimage
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

test -n "${DMGLocation}" && make_dmg "${DMGLocation}"
