#include <boost/config.hpp>
#include <boost/detail/workaround.hpp>
#include <boost/filesystem/config.hpp>
#include <boost/filesystem/convenience.hpp>
#include <boost/filesystem/exception.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/scoped_array.hpp>
#include <boost/throw_exception.hpp>

#include <cassert>
#include <cerrno>
#include <cstdio>      // for remove, rename
#include <cstring>
#include <cstring>  // SGI MIPSpro compilers need this
#include <cstring> // SGI MIPSpro compilers need this
#include <string>
#include <sys/stat.h>  // last_write_time() uses stat()
#include <vector>
