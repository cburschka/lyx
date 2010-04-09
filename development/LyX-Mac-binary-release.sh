#!/bin/sh

# set -x

# This script automates creating universal binaries of LyX on Mac.
# Author: Bennett Helm (and extended by Konrad Hofbauer)
# modified by Stephan Witt
# Last modified: 17 April 2010

# Prerequisite:
# * a decent checkout of LyX sources (probably you have it already)
# * Qt4 - build with static libraries for the used platforms (i386 and ppc)
# * for aspell support:
#   the aspell sources placed in a sibling directory (variable ASpellSourceVersion)

# Set these variables -- to
# (1) the location of your Qt4 installation and optionally
# (2) the location of resulting DMG
# (3) the version of private aspell installation
#     (to define the location assign ASpellSourceDir instead)
# (4) the list of architectures to build for

QtInstallDir=${QTDIR:-"/opt/qt4"}
DMGLocation=.. # one level above LyxSourceDir
ASpellSourceVersion="aspell-0.60.6"
ARCH_LIST="ppc i386"

GetText="--with-included-gettext"
ConfigureOptions="--enable-warnings --enable-optimization=-Os"

# strip="-strip"

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

ASpellSourceDir=${ASpellSourceDir:-`dirname "${LyxSourceDir}"`/${ASpellSourceVersion}}
ASpellInstallDir=${ASpellInstallDir:-`dirname "${LyxSourceDir}"`/${ASpellSourceVersion}.lib}

if [ ! -f "${LyxSourceDir}"/configure ]; then
	( cd "${LyxSourceDir}" && sh autogen.sh )
fi
if [ -z "${LyXVersion}" ]; then
	LyXVersion=`grep AC_INIT "${LyxSourceDir}"/configure.ac | cut -d, -f2 | tr -d " ()"`
fi

LyxName="LyX"
LyxBase="${LyxName}-${LyXVersion}"
LyxApp="${LyxBase}.app"
LyxAppDir=`dirname "${LyxSourceDir}"`/"${LyxBase}"
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

# Check for existing SDKs
# 10.6 cannot be used for 10.4 currently
SDKs=`echo /Developer/SDKs/MacOSX10*sdk`
case "$SDKs" in
*10.6*)
	MACOSX_DEPLOYMENT_TARGET="10.5"; export MACOSX_DEPLOYMENT_TARGET
	SDKROOT="/Developer/SDKs/MacOSX10.5.sdk"; export SDKROOT
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

if [ -d "${ASpellSourceDir}" -a ! -d "${ASpellInstallDir}" ]; then
	# we have a private ASpell source tree at hand...
	# so let's build and install it
	if [ -z "${ASpellVersion}" ]; then
		ASpellVersion=`grep AC_INIT "${ASpellSourceDir}"/configure.ac | cut -d, -f2|tr -d " ()"`
	fi

	ASpellName="Aspell"
	ASpellBase="${ASpellName}-${ASpellVersion}"
	ASpellLib="${ASpellBase}.lib"
	ASpellLibDir=`dirname "${ASpellSourceDir}"`/"${ASpellBase}"
	ASpellLibPrefix="${ASpellInstallDir:-${ASpellLibDir}.lib}"

	# Clear Output
	if [ -n "${ASpellLibZip}" -a -f "${ASpellLibZip}" ]; then rm "${ASpellLibZip}"; fi
	if [ -d "${ASpellLibPrefix}" ]; then rm -r "${ASpellLibPrefix}"; fi

	# ASpell builds inplace only :(
	cd "${ASpellSourceDir}" && make distclean

	# ----------------------------------------
	# Build ASpell for different architectures
	# ----------------------------------------
	FILE_LIST="libaspell.15.dylib libpspell.15.dylib"

	for arch in ${ARCH_LIST} ; do
		LDFLAGS="${SDKROOT:+-isysroot ${SDKROOT}} -arch ${arch}"; export LDFLAGS
		LDFLAGS="${CPPFLAGS}"; export LDFLAGS
		HOSTSYSTEM=`eval "echo \\$HostSystem_$arch"`
		"${ASpellSourceDir}/configure"\
			--prefix="${ASpellLibPrefix}"\
			${ConfigureOptions}\
			${GetText} --host="${HOSTSYSTEM}" ${BuildSystem:+"--build=${BuildSystem}"} --enable-build-type=rel
		make && make install${strip}
		for file in ${FILE_LIST} ; do
			if [ -f "${ASpellLibPrefix}"/lib/${file} ]; then
				mv "${ASpellLibPrefix}"/lib/${file}\
					"${ASpellLibPrefix}"/lib/${file}-${arch} 
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
			cd "${ASpellLibPrefix}"
			lipo -create ${OBJ_LIST} -o lib/${file}
		)
	done
	# --------
	# Clean up
	# --------
	for arch in ${ARCH_LIST} ; do
		rm -f "${ASpellLibPrefix}"/lib/*-${arch}
	done
fi

# Clear Output
if [ -n "${LyxAppZip}" -a -f "${LyxAppZip}" ]; then rm "${LyxAppZip}"; fi
if [ -d "${LyxAppPrefix}" ]; then rm -r "${LyxAppPrefix}"; fi

# -------------------------------------
# Build LyX for different architectures
# -------------------------------------
FILE_LIST="lyx lyxclient tex2lyx"

if [ -d "${ASpellInstallDir}" ]; then
	ConfigureOptions="${ConfigureOptions} --with-extra-inc=${ASpellInstallDir}/include"
	ConfigureOptions="${ConfigureOptions} --with-extra-lib=${ASpellInstallDir}/lib"
fi

for arch in ${ARCH_LIST} ; do

	if [ -d "${LyxBuildDir}" ];  then rm -r "${LyxBuildDir}"; fi
	mkdir "${LyxBuildDir}" && cd "${LyxBuildDir}"

	CPPFLAGS="${SDKROOT:+-isysroot ${SDKROOT}} -arch ${arch}"; export CPPFLAGS
	LDFLAGS="${CPPFLAGS}"; export LDFLAGS
	HOSTSYSTEM=`eval "echo \\$HostSystem_$arch"`

	echo LDFLAGS="${LDFLAGS}"
	echo CPPFLAGS="${CPPFLAGS}"
	"${LyxSourceDir}/configure"\
		--prefix="${LyxAppPrefix}" --with-version-suffix="-${LyXVersion}"\
		${QtInstallDir:+"--with-qt4-dir=${QtInstallDir}"} \
		${ConfigureOptions}\
		${GetText} --host="${HOSTSYSTEM}" --build="${BuildSystem}" --enable-build-type=rel
	make && make install${strip}
	for file in ${FILE_LIST} ; do
		if [ -f "${LyxAppPrefix}"/Contents/MacOS/${file} ]; then
			mv "${LyxAppPrefix}"/Contents/MacOS/${file}\
				"${LyxAppPrefix}"/Contents/MacOS/${file}-${arch} 
		else
			echo Cannot build and install LyX for ${arch}.
			exit 1
		fi
	done
done

# -------------------------
# Create universal binaries
# -------------------------

ASpellFramework="Frameworks/Aspell.framework"
if [ -d "${ASpellInstallDir}" ]; then
	ASpellDyLib=`basename "${ASpellInstallDir}/lib/"libaspell.*.dylib`
	mkdir -p "${LyxAppPrefix}"/Contents/${ASpellFramework}/lib
	cp -p "${ASpellInstallDir}/lib/${ASpellDyLib}" "${LyxAppPrefix}"/Contents/${ASpellFramework}/lib
	install_name_tool -id "@executable_path/../${ASpellFramework}/lib/${ASpellDyLib}" "${LyxAppPrefix}"/Contents/${ASpellFramework}/lib/"${ASpellDyLib}"
fi

for file in ${FILE_LIST} ; do
	OBJ_LIST=
	for arch in ${ARCH_LIST} ; do
		OBJ_LIST="${OBJ_LIST} Contents/MacOS/${file}-${arch}"
	done
	(
		cd "${LyxAppPrefix}" &&
		lipo -create ${OBJ_LIST} -o "${LyxAppPrefix}"/Contents/MacOS/${file}
	)
	if [ -d "${ASpellInstallDir}" ]; then
		install_name_tool -change \
			"${ASpellInstallDir}/lib/${ASpellDyLib}" "@executable_path/../${ASpellFramework}/${ASpellDyLib}" \
			"${LyxAppPrefix}"/Contents/MacOS/${file}
	fi
done

if [ -d "${ASpellInstallDir}" ]; then
	if [ -d /opt/local/lib/aspell-0.60 ]; then ASpellInstallDir=/opt/local ; fi
	mkdir -p "${LyxAppPrefix}"/Contents/${ASpellFramework}/lib
	cp -p -r "${ASpellInstallDir}/lib/aspell-0.60" "${LyxAppPrefix}"/Contents/${ASpellFramework}/lib
	mkdir -p "${LyxAppPrefix}"/Contents/${ASpellFramework}/share
	cp -p -r "${ASpellInstallDir}/share/aspell" "${LyxAppPrefix}"/Contents/${ASpellFramework}/share
fi

# --------
# Clean up
# --------

for arch in ${ARCH_LIST} ; do
	rm -f "${LyxAppPrefix}"/Contents/MacOS/*-${arch}
done

# ------------------------------
# Building distribution packages
# ------------------------------

test -n "${LyxAppZip}" && (
	cd "${LyxAppPrefix}" && zip -r "${LyxAppZip}" .
)

DMGNAME="${LyxBase}-Uncompressed.dmg"
DMGSIZE="350m"
COMPRESSEDDMGNAME="${LyxBase}.dmg"
BACKGROUND="${LyxBase}.app/Contents/Resources/images/banner.png"

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

test -n "${DMGLocation}" && (
	cd "${DMGLocation}"

	BGSIZE=`file "$BACKGROUND" | awk -F , '/PNG/{print $2 }' | tr x ' '`
	BG_W=`echo ${BGSIZE} | awk '{print $1 }'`
	BG_H=`echo ${BGSIZE} | awk '{h = $2 + 20 ;print h }'`
	BG_Y=`echo ${BGSIZE} | awk '{y = $2 - 60 ;print y }'`

	rm -f ${DMGNAME}
	rm -f ${COMPRESSEDDMGNAME}

	hdiutil create -size ${DMGSIZE:-"250m"} -fs HFS+ -volname "${LyxBase}" "${DMGNAME}"
	# Unmount currently mounted disk image
	test -d /Volumes/"${LyxBase}" && umount /Volumes/"${LyxBase}"

	# Mount the disk image
	hdiutil attach ${DMGNAME}

	# Obtain device information
	DEVS=$(hdiutil attach ${DMGNAME} | cut -f 1)
	DEV=$(echo $DEVS | cut -f 1 -d ' ')
	VOLUME=$(mount |grep ${DEV} | cut -f 3 -d ' ')

	# copy in the application bundle
	cp -Rp ${LyxBase}.app ${VOLUME}/${LyxName}.app

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
	hdiutil convert ${DMGNAME} -format UDBZ -o ${COMPRESSEDDMGNAME}
	rm -f ${DMGNAME}
)
