#include <config.h>

#include "lyx_forms.h"
#include <X11/keysym.h>
#include <X11/X.h>
#include <X11/X.h> // Window
#include <X11/Xlib.h>
#include <X11/Xlib.h> // for Pixmap
#include <X11/Xlib.h> // for Pixmap, GC
#include <X11/xpm.h>

#include <boost/assert.hpp>
#include <boost/regex.hpp>
#include <boost/scoped_array.hpp>
#include <boost/scoped_ptr.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/signal.hpp>
#include <boost/signals/connection.hpp>
#include <boost/signals/trackable.hpp>
#include <boost/tuple/tuple.hpp>

#include <algorithm>
#include <cmath>
#include <fcntl.h>
#include <fstream>
#include <iomanip>
#include <list>
#include <map>
#include <sstream>
#include <string>
#include <utility>
#include <vector>
