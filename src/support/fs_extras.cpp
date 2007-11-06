// -*- C++ -*-
/* \file fs_extras.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Lars Gullik Bjønnes
 *
 * Full author contact details are available in file CREDITS.
 */


#include <config.h>

#include "fs_extras.h"

#include <boost/filesystem/config.hpp>
#include <boost/detail/workaround.hpp>
#include <boost/throw_exception.hpp>

#ifdef HAVE_SYS_TYPES_H
# include <sys/types.h>
#endif
#ifdef HAVE_SYS_STAT_H
# include <sys/stat.h>
#endif
#include <cerrno>
#include <fcntl.h>


// BOOST_POSIX or BOOST_WINDOWS specify which API to use.
# if !defined( BOOST_WINDOWS ) && !defined( BOOST_POSIX )
#   if defined(_WIN32) || defined(__WIN32__) || defined(WIN32) || defined(__CYGWIN__)
#     define BOOST_WINDOWS
#   else
#     define BOOST_POSIX
#   endif
# endif

#if defined (BOOST_WINDOWS)
# define WIN32_LEAN_AND_MEAN
# include <windows.h>
#endif

namespace fs = boost::filesystem;

namespace boost {
namespace filesystem {

void copy_file(path const & source, path const & target, bool noclobber)
{

#ifdef BOOST_POSIX
	int const infile = ::open(source.string().c_str(), O_RDONLY);
	if (infile == -1) {
		boost::throw_exception(
			filesystem_path_error(
				"boost::filesystem::copy_file",
				source, target,
				fs::lookup_errno(errno)));
	}

		struct stat source_stat;
		int const ret = ::fstat(infile, &source_stat);
		if (ret == -1) {
		int err = errno;
		::close(infile);
		boost::throw_exception(filesystem_path_error(
			   "boost::filesystem::copy_file",
			   source, target,
			   fs::lookup_errno(err)));
	}

	int const flags = O_WRONLY | O_CREAT | (noclobber ? O_EXCL : O_TRUNC);

	int const outfile = ::open(target.string().c_str(), flags, source_stat.st_mode);
	if (outfile == -1) {
		int err = errno;
		::close(infile);
		boost::throw_exception(
			filesystem_path_error(
				"boost::filesystem::copy_file",
				source, target,
				fs::lookup_errno(err)));
	}

	std::size_t const buf_sz = 32768;
	char buf[buf_sz];
	ssize_t in = -1;
	ssize_t out = -1;

	while (true) {
		in = ::read(infile, buf, buf_sz);
		if (in == -1) {
			break;
		} else if (in == 0) {
			break;
		} else {
			out = ::write(outfile, buf, in);
			if (out == -1) {
				break;
			}
		}
	}

	int err = errno;

	::close(infile);
	::close(outfile);

	if (in == -1 || out == -1)
		boost::throw_exception(
			filesystem_path_error(
				"boost::filesystem::copy_file",
				source, target,
				fs::lookup_errno(err)));
#endif
#ifdef BOOST_WINDOWS
	if (::CopyFile(source.string().c_str(), target.string().c_str(), noclobber) == 0) {
		// CopyFile is probably not setting errno so this is most
		// likely wrong.
		boost::throw_exception(
			filesystem_path_error(
				"boost::filesystem::copy_file",
				source, target,
				fs::lookup_error_code(errno)));
	}
#endif
}

}
}
