#include <boost/config.hpp>
#include <boost/detail/workaround.hpp>
#include <boost/scoped_array.hpp>
#include <boost/signals/connection.hpp>
#include <boost/signals/detail/signal_base.hpp>
#include <boost/signals/slot.hpp>
#include <boost/signals/trackable.hpp>
#include <boost/throw_exception.hpp>

#include <algorithm>
#include <cassert>
#include <cctype>
#include <cerrno>
#include <climits>
#include <clocale>
#include <cwchar>
#include <iostream>
#include <list>
#include <locale>
#include <map>
#include <new>
#include <ostream>
#include <set>
#include <stdexcept>
#include <string>
#include <vector>

#include <sys/stat.h>  // last_write_time() uses stat()
