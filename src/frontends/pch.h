#include <config.h>

#include <boost/function.hpp>
#include <boost/scoped_ptr.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/signal.hpp>
#include <boost/signals/trackable.hpp>
#include <boost/utility.hpp>

#ifdef HAVE_SYS_TIME_H
# include <sys/time.h>
#endif
#ifdef HAVE_UNISTD_H
# include <unistd.h>
#endif

#include <limits>
#include <map>
#include <string>
#include <utility>
#include <vector>
