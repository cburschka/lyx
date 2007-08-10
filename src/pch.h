#include <config.h>

#include <boost/any.hpp>
#include <boost/array.hpp>
#include <boost/assert.hpp>
#include <boost/crc.hpp>
#include <boost/cregex.hpp>
#include <boost/current_function.hpp>
// #include <boost/format.hpp> // mult def symbols problem (_1, _2 etc)
#include <boost/function.hpp>
#include <boost/iterator/indirect_iterator.hpp>
#include <boost/optional.hpp>
#include <boost/regex.hpp>
#include <boost/scoped_array.hpp>
#include <boost/scoped_ptr.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/signals/connection.hpp>
#include <boost/signal.hpp>
#include <boost/signals/trackable.hpp>
#include <boost/version.hpp>

#if BOOST_VERSION < 103300
# include <boost/test/detail/nullstream.hpp>
#else
# include <boost/test/utils/nullstream.hpp>
#endif

#include <boost/tokenizer.hpp>
#include <boost/tuple/tuple.hpp>
#include <boost/utility.hpp>

#include <fcntl.h>
#ifdef HAVE_SYS_SELECT_H
# include <sys/select.h>
#endif
#ifdef HAVE_SYS_STAT_H
# include <sys/stat.h>
#endif
#ifdef HAVE_SYS_TIME_H
# include <sys/time.h>
#endif

#include <algorithm>
#include <cerrno>
#include <clocale>
#include <csignal>
#include <cstddef>
#include <cstdio>
#include <cstdlib>
#include <deque>
#include <exception>
#include <fstream>
#include <functional>
#include <iomanip>
#include <ios>
#include <iosfwd>
#include <iostream>
#include <iterator>
#include <list>
#include <map>
#include <queue>
#include <set>
#include <sstream>
#include <stack>
#include <string>
#include <utility>
#include <vector>
