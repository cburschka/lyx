// -*- C++ -*-

#include <config.h>

#include "fs_extras.h"

#include <boost/filesystem/config.hpp>
#include <boost/filesystem/exception.hpp>
#include <boost/detail/workaround.hpp>
#include <boost/throw_exception.hpp>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

// BOOST_POSIX or BOOST_WINDOWS specify which API to use.
# if !defined( BOOST_WINDOWS ) && !defined( BOOST_POSIX )
#   if defined(_WIN32) || defined(__WIN32__) || defined(WIN32) || defined(__CYGWIN__)
#     define BOOST_WINDOWS
#   else
#     define BOOST_POSIX
#   endif
# endif

namespace fs = boost::filesystem;

namespace boost {
namespace filesystem {

bool is_readable(path const & ph)
{
#ifdef BOOST_POSIX
	return ::access(ph.string().c_str(), R_OK) == 0;
#endif
}


bool is_writable(path const & ph)
{
#ifdef BOOST_POSIX
	return ::access(ph.string().c_str(), W_OK) == 0;
#endif
}


bool is_readonly(path const & ph)
{
#ifdef BOOST_POSIX
	return is_readable(ph) && !is_writable(ph);
#endif
}


void copy_file(path const & source, path const & target, bool noclobber)
{

#ifdef BOOST_POSIX
	int const infile = ::open(source.string().c_str(), O_RDONLY);
        if (infile == -1) {
		boost::throw_exception(
			filesystem_error(
				"boost::filesystem::copy_file",
				source, target,
				fs::detail::system_error_code()));
	}

        struct stat source_stat;
        int const ret = ::fstat(infile, &source_stat);
        if (ret == -1) {
                ::close(infile);
		boost::throw_exception(
			filesystem_error(
				"boost::filesystem::copy_file",
				source, target,
				fs::detail::system_error_code()));
        }

        int const flags = O_WRONLY | O_CREAT | (noclobber ? O_EXCL : O_TRUNC);

        int const outfile = ::open(target.string().c_str(), flags, source_stat.st_mode);
        if (outfile == -1) {
                ::close(infile);
		boost::throw_exception(
			filesystem_error(
				"boost::filesystem::copy_file",
				source, target,
				fs::detail::system_error_code()));
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

        ::close(infile);
        ::close(outfile);

        if (in == -1 || out == -1)
		boost::throw_exception(
			filesystem_error(
				"boost::filesystem::copy_file",
				source, target,
				fs::detail::system_error_code()));
#endif
}


}
}
