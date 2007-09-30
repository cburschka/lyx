// -*- C++ -*-
#include <config.h>

#include <boost/assert.hpp>
#include <boost/crc.hpp>
// #include <boost/format.hpp> // mult def symbols problem (_1,_2 etc)
#include <boost/regex.hpp>
#include <boost/scoped_array.hpp>
#include <boost/scoped_ptr.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/signal.hpp>
#include <boost/signals/trackable.hpp>
#include <boost/version.hpp>

#if BOOST_VERSION < 103300
# include <boost/test/detail/nullstream.hpp>
#else
# include <boost/test/utils/nullstream.hpp>
#endif

#include <boost/tokenizer.hpp>
#include <boost/noncopyable.hpp>

#include <fcntl.h>
#ifdef HAVE_SYS_STAT_H
# include <sys/stat.h>
#endif
#ifdef HAVE_SYS_TYPES_H
# include <sys/types.h>
#endif
#include <time.h>
#ifdef HAVE_SYS_SOCKET_H
# include <sys/socket.h>
#endif
#ifndef _WIN32
# include <sys/un.h>
# include <sys/wait.h>
# ifdef HAVE_UNISTD_H
#  include <unistd.h>
# endif
#endif

#include <algorithm>
#include <cctype>
#include <cerrno>
#include <csignal>
#include <cstddef>
#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <deque>
#include <fstream>
#include <functional>
#include <iostream>
#include <istream>
#include <iterator>
#include <list>
#include <map>
#include <ostream>
#include <queue>
#include <sstream>
#include <string>
#include <utility>
#include <vector>
