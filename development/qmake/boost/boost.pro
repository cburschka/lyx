
include(../config.pri)

TARGET = lyxboost$${DEBUGSUFFIX}

DEFINES += BOOST_DISABLE_THREADS=1
DEFINES += BOOST_NO_WREGEX=1
DEFINES += BOOST_NO_WSTRING=1
#DEFINES += BOOST_USER_CONFIG="<config.h>"

#if !defined(ENABLE_ASSERTIONS)
#  define BOOST_DISABLE_ASSERTS 1
#endif
#define BOOST_ENABLE_ASSERT_HANDLER 1

INC += src
INC += boost

CPP += libs/regex/src/c_regex_traits.cpp
CPP += libs/regex/src/regex_traits_defaults.cpp
CPP += libs/regex/src/regex_debug.cpp
CPP += libs/regex/src/cpp_regex_traits.cpp
CPP += libs/regex/src/winstances.cpp
CPP += libs/regex/src/posix_api.cpp
CPP += libs/regex/src/wide_posix_api.cpp
CPP += libs/regex/src/w32_regex_traits.cpp
CPP += libs/regex/src/instances.cpp
CPP += libs/regex/src/fileiter.cpp
CPP += libs/regex/src/regex.cpp
CPP += libs/regex/src/regex_raw_buffer.cpp
CPP += libs/regex/src/cregex.cpp
CPP += libs/signals/src/named_slot_map.cpp
CPP += libs/signals/src/slot.cpp
CPP += libs/signals/src/trackable.cpp
CPP += libs/signals/src/signal_base.cpp
CPP += libs/signals/src/connection.cpp
CPP += libs/iostreams/src/mapped_file.cpp
CPP += libs/iostreams/src/file_descriptor.cpp
CPP += libs/iostreams/src/zlib.cpp
CPP += libs/filesystem/src/portability.cpp
CPP += libs/filesystem/src/exception.cpp
CPP += libs/filesystem/src/path.cpp
CPP += libs/filesystem/src/operations.cpp

for(FILE,CPP) { SOURCES += $${BUILD_BASE_SOURCE_DIR}/boost/$${FILE} }
for(PATH,INC) { INCLUDEPATH += $${BUILD_BASE_SOURCE_DIR}/$${PATH} }
