// -*- C++ -*-

#include <config.h>

#include <boost/array.hpp>
#include <boost/assert.hpp>
#include <boost/function.hpp>
#include <boost/scoped_ptr.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/signals/trackable.hpp>
#include <boost/signal.hpp>
#include <boost/tuple/tuple.hpp>
#include <boost/utility.hpp>

#include <algorithm>
#include <cmath>
#include <iomanip>
#include <limits.h>
#include <map>
#include <sstream>
#include <stack>
#include <string>
#include <utility>
#include <vector>

// Have I said anything about the braindeadness of QT lately?
// Just to remind you: signals slot emit as macros, pure lunacy!
// So we are not able to have any qt headers in the pch.

// #include <qt.h>

// #define qtsignal signal
// #define qtsignals signals
// #define qtemit emit
// #undef signal
// #undef signals
// #undef emit
