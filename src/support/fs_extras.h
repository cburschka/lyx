// -*- C++ -*-

#include <boost/filesystem/path.hpp>

namespace boost {
namespace filesystem {

bool is_readable(path const & ph);

bool is_writable(path const & ph);

bool is_readonly(path const & ph);

void copy_file(path const & source, path const & target, bool noclobber);

}
}
