
TEMPLATE = lib

CONFIG += debug_and_release
CONFIG += create_prl

DESTDIR = ../lib

BUILD_BASE_SOURCE_DIR = /suse/usr/src/lyx/trunk
BUILD_BASE_TARGET_DIR = /suse/usr/src/lyx/qmake

#message(BUILD_BASE_SOURCE_DIR: $${BUILD_BASE_SOURCE_DIR})
#message(BUILD_BASE_TARGET_DIR: $${BUILD_BASE_TARGET_DIR})

# for <config.h>
INCLUDEPATH += $${BUILD_BASE_TARGET_DIR}/src
