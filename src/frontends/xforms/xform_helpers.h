#ifndef XFORMHELPERS_H
#define XFORMHELPERS_H

#ifdef __GNUG_
#pragma interface
#endif

#include <config.h>
#include "LString.h"

// Take a string and add breaks so that it fits into a desired label width, w
string formatted( string const &label, int w, int size, int style );

#endif
