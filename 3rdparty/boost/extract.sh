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
	boost/bind.hpp \
	boost/crc.hpp \
	boost/cstdint.hpp \
	boost/function.hpp \
	boost/functional.hpp \
	boost/lexical_cast.hpp \
	boost/noncopyable.hpp \
	boost/regex.hpp \
	boost/scoped_array.hpp \
	boost/scoped_ptr.hpp \
	boost/shared_ptr.hpp \
	boost/signal.hpp \
	boost/signals/connection.hpp \
	boost/signals/trackable.hpp \
	boost/tokenizer.hpp \
	boost/tuple/tuple.hpp \
    boost/mpl/string.hpp \
    boost/mpl/fold.hpp \
    boost/mpl/size_t.hpp \
    boost/functional/hash.hpp \
	\
	needed


find boost -name \*.hpp | xargs rm
find libs  -name \*.cpp | xargs rm

cp -vR needed/boost .
cp -vR needed/libs .

rm -rf needed

# found by bcp but not needed by us
rm -rf libs/config
rm -rf libs/smart_ptr
rm -rf libs/signals/build
rm -rf libs/regex/build
rm -rf libs/regex/test



