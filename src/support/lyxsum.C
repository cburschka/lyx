/* This file is part of
 * ======================================================
 * 
 *           LyX, The Document Processor
 *           Copyright 2001 The LyX Team.
 *
 * ======================================================
 */


#include <config.h>

#include <algorithm>
#include <boost/crc.hpp>

#include "support/lyxlib.h"


// Various implementations of lyx::sum(), depending on what methods
// are available. Order is faster to slowest.
#if defined(HAVE_MMAP) && defined(HAVE_MUNMAP)
#ifdef WITH_WARNINGS
#warning lyx::sum() using mmap (lightning fast)
#endif

// Make sure we get modern version of mmap and friends with void*,
// not `compatibility' version with caddr_t.
#define _POSIX_C_SOURCE 199506L

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>

unsigned long lyx::sum(string const & file)
{
	int fd = open(file.c_str(), O_RDONLY);
	if(!fd)
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

} // namespace

#if HAVE_DECL_ISTREAMBUF_ITERATOR
#ifdef WITH_WARNINGS
#warning lyx::sum() using istreambuf_iterator (fast)
#endif
unsigned long lyx::sum(string const & file)
{
	std::ifstream ifs(file.c_str());
	if (!ifs) return 0;
	
	std::istreambuf_iterator<char> beg(ifs);
	std::istreambuf_iterator<char> end;
	
	return do_crc(beg,end);
}
#else
#ifdef WITH_WARNINGS
#warning lyx::sum() using istream_iterator (slow as a snail)
#endif
unsigned long lyx::sum(string const & file)
{
	std::ifstream ifs(file.c_str());
	if (!ifs) return 0;
	
	ifs.unsetf(std::ios::skipws);
	std::istream_iterator<char> beg(ifs);
	std::istream_iterator<char> end;
	
	return do_crc(beg,end);
}
#endif
#endif // mmap
