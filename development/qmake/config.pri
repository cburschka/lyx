
TEMPLATE = lib

CONFIG += debug_and_release
CONFIG += create_prl

DESTDIR = ../lib

CONFIG(release, debug|release) DEBUGSUFFIX =
CONFIG(debug, debug|release) DEBUGSUFFIX = d

BUILD_BASE_SOURCE_DIR = $$[BUILD_BASE_SOURCE_DIR]
BUILD_BASE_TARGET_DIR = $$[BUILD_BASE_TARGET_DIR]

# for <config.h>
INCLUDEPATH += $${BUILD_BASE_TARGET_DIR}/src
