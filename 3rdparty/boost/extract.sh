#!/bin/bash

#
# Script to extract only needed boost files using the bcp tool:
#
# http://www.boost.org/doc/libs/1_47_0/tools/bcp/doc/html/index.html
#
# Does also work with an outdated bcp version 
#
# Usage: extract.sh <path to new boost version>
#

if [ -z $1 ]
then
    echo "Usage: extract.sh <path to new boost version>"
    exit 1
fi

rm -rf needed
mkdir needed

bcp --boost=$1 \
	boost/any.hpp \
	boost/assert.hpp \
	boost/crc.hpp \
	boost/cstdint.hpp \
	boost/lexical_cast.hpp \
	boost/regex.hpp \
	boost/signals2.hpp \
	boost/signals2/connection.hpp \
	boost/signals2/trackable.hpp \
	\
	needed


# we do not use the provided MSVC project files
find needed -name '*.vcpro*' | xargs rm

find boost -name \*.hpp | xargs rm
find libs  -name \*.cpp | xargs rm

cp -vR needed/boost .
cp -vR needed/libs .

rm -rf needed

# found by bcp but not needed by us
rm -rf boost/regex/icu.hpp
rm -rf boost/typeof
rm -rf libs/config
rm -rf libs/smart_ptr
rm -rf libs/regex/build
rm -rf libs/regex/test
rm -rf libs/regex/src/icu.cpp

