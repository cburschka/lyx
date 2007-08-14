
TEMPLATE = lib

CONFIG += debug_and_release
#CONFIG += no_include_pwd
#CONFIG += create_prl

DEFINES += QT_NO_KEYWORDS

DESTDIR = ../lib

QT = 

CONFIG(release, debug|release) DEBUGSUFFIX =
CONFIG(debug, debug|release) DEBUGSUFFIX = d

BUILD_BASE_SOURCE_DIR = $$[BUILD_BASE_SOURCE_DIR]
BUILD_BASE_TARGET_DIR = $$[BUILD_BASE_TARGET_DIR]

# for <config.h>
INCLUDEPATH += $${BUILD_BASE_TARGET_DIR}/src

#QMAKE_CXXFLAGS += -include $${BUILD_BASE_SOURCE_DIR}/src/pch.h
#QMAKE_CXXFLAGS += -include $${BUILD_BASE_TARGET_DIR}/src/pch.h.gch
#QMAKE_CXXFLAGS_USE_PRECOMPILE = -include ${QMAKE_PCH_OUTPUT_BASE}/

#QMAKE_CXXFLAGS_USE_PRECOMPILE = -include $${BUILD_BASE_TARGET_DIR}/src/pch.h.gch
#QMAKE_CXXFLAGS_USE_PRECOMPILE += -Winvalid-pch
#QMAKE_CXXFLAGS_USE_PRECOMPILE += -Wmissing-include-dirs



#CONFIG += precompile_header

#PRECOMPILED_HEADER = $${BUILD_BASE_SOURCE_DIR}/src/pch.h

#QMAKE_CXXFLAGS = -include $${BUILD_BASE_SOURCE_DIR}/src/pch.h
#QMAKE_CXXFLAGS += -Winvalid-pch
#QMAKE_CXXFLAGS += -Wmissing-include-dirs

