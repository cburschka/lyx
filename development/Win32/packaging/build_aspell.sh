#! /bin/sh

grab_sources() {

wget ftp://ftp.gnu.org/gnu/aspell/aspell-0.60.4.tar.gz

}

unpack() {

gunzip aspell-0.60.4.tar.gz
tar xvf aspell-0.60.4.tar

}

build_it() {
(
    cd aspell-0.60.4

    # Add a missing #include.
    grep '^#  include "asc_ctype\.hpp"' common/file_util.cpp >/dev/null || {
	sed '/^#ifdef WIN32 *$/,/^# *include <io\.h> *$/{
/<io\.h>/i\
#  include "asc_ctype.hpp"
}'  common/file_util.cpp > tmp

	cmp -s common/file_util.cpp tmp || {
	    diff -u common/file_util.cpp tmp
	    mv -i tmp common/file_util.cpp
	}
	rm -f tmp
    }

    ./configure \
	--enable-static \
	--disable-shared \
	--prefix=c:/Aspell || {
	echo 'configure failed' >&2
	exit 1
    }
    make
)
}

install_it() {
(
    cd aspell-0.60.4
    make install
)
}

build_it
install_it

# The end
