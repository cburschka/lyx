#!/bin/sh

#
# Script to extract only needed boost files using the bcp tool:
#
# http://www.boost.org/doc/libs/1_47_0/tools/bcp/doc/html/index.html
#
# Does also work with an outdated bcp version 
#

rm -rf needed
mkdir needed

bcp --boost=$PWD \
	boost/any.hpp \
	boost/assert.hpp \
	boost/bind.hpp \
	boost/crc.hpp \
	boost/cstdint.hpp \
	boost/format.hpp \
	boost/function.hpp \
	boost/functional.hpp \
	boost/lexical_cast.hpp \
	boost/next_prior.hpp \
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
	\
	needed


find boost -name \*.hpp | xargs rm
find libs  -name \*.cpp | xargs rm

cp -vR needed/boost .
cp -vR needed/libs .

rm -rf needed


