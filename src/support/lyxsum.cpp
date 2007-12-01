/**
 * \file lyxsum.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Lars Gullik Bjønnes
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "support/lyxlib.h"
#include "support/debug.h"
#include "support/FileName.h"

#include <boost/crc.hpp>

#include <algorithm>
#include <iomanip>

using std::endl;
using std::string;

// OK, this is ugly, but it is the only workaround I found to compile
// with gcc (any version) on a system which uses a non-GNU toolchain.
// The problem is that gcc uses a weak symbol for a particular
// instantiation and that the system linker usually does not
// understand those weak symbols (seen on HP-UX, tru64, AIX and
// others). Thus we force an explicit instanciation of this particular
// template (JMarc)
template struct boost::detail::crc_table_t<32, 0x04C11DB7, true>;

// Various implementations of lyx::sum(), depending on what methods
// are available. Order is faster to slowest.
#if defined(HAVE_MMAP) && defined(HAVE_MUNMAP)

#ifdef HAVE_SYS_TYPES_H
# include <sys/types.h>
#endif
#ifdef HAVE_SYS_STAT_H
# include <sys/stat.h>
#endif
#include <fcntl.h>
#ifdef HAVE_UNISTD_H
# include <unistd.h>
#endif

#include <sys/mman.h>


namespace lyx {
namespace support {

unsigned long sum(char const * file)
{
	//LYXERR(Debug::FILES, "lyx::sum() using mmap (lightning fast)");

	int fd = open(file, O_RDONLY);
	if (!fd)
		return 0;

	struct stat info;
	fstat(fd, &info);

	void * mm = mmap(0, info.st_size, PROT_READ,
			 MAP_PRIVATE, fd, 0);
	// Some platforms have the wrong type for MAP_FAILED (compaq cxx).
	if (mm == reinterpret_cast<void*>(MAP_FAILED)) {
		close(fd);
		return 0;
	}

	char * beg = static_cast<char*>(mm);
	char * end = beg + info.st_size;

	boost::crc_32_type crc;
	crc.process_block(beg, end);
	unsigned long result = crc.checksum();

	munmap(mm, info.st_size);
	close(fd);

	return result;
}

} // namespace support
} // namespace lyx

#else // No mmap

#include <fstream>
#include <iterator>


namespace {

template<typename InputIterator>
inline
unsigned long do_crc(InputIterator first, InputIterator last)
{
	boost::crc_32_type crc;
	crc = std::for_each(first, last, crc);
	return crc.checksum();
}

} // namespace anon


namespace lyx {
namespace support {

using std::ifstream;
#if HAVE_DECL_ISTREAMBUF_ITERATOR
using std::istreambuf_iterator;

unsigned long sum(char const * file)
{
	//LYXERR(Debug::FILES, "lyx::sum() using istreambuf_iterator (fast)");

	ifstream ifs(file, std::ios_base::in | std::ios_base::binary);
	if (!ifs)
		return 0;

	istreambuf_iterator<char> beg(ifs);
	istreambuf_iterator<char> end;

	return do_crc(beg,end);
}
#else

using std::istream_iterator;
using std::ios;

unsigned long sum(char const * file)
{
	//LYXERR(Debug::FILES, "lyx::sum() using istream_iterator (slow as a snail)");

	// a directory may be passed here so we need to test it. (bug 3622)
	if (file.isDirectory())
		return 0;

	ifstream ifs(file, std::ios_base::in | std::ios_base::binary);
	if (!ifs)
		return 0;

	ifs.unsetf(ios::skipws);
	istream_iterator<char> beg(ifs);
	istream_iterator<char> end;

	return do_crc(beg,end);
}
#endif

} // namespace support
} // namespace lyx

#endif // mmap
