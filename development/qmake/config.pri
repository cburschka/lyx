
TEMPLATE = lib

CONFIG += debug_and_release
#CONFIG += create_prl
CONFIG += precompile_header

DEFINE += QT_NO_KEYWORDS

DESTDIR = ../lib

CONFIG(release, debug|release) DEBUGSUFFIX =
CONFIG(debug, debug|release) DEBUGSUFFIX = d

BUILD_BASE_SOURCE_DIR = $$[BUILD_BASE_SOURCE_DIR]
BUILD_BASE_TARGET_DIR = $$[BUILD_BASE_TARGET_DIR]

PRECOMPILED_HEADER = $${BUILD_BASE_SOURCE_DIR}/src/pch.h

# for <config.h>
INCLUDEPATH += $${BUILD_BASE_TARGET_DIR}/src
