// -*- C++ -*-
/* This file is part of
 * ======================================================
 *
 *           LyX, The Document Processor
 *
 *	    Copyright 1995 Matthias Ettrich
 *          Copyright 1995-2001 The LyX Team.
 *
 * ====================================================== */

#ifndef LSSTREAM_H
#define LSSTREAM_H

// Since we will include a string header anyway, we'd better do it
// right now so that <string> is not loaded before lyxstring.h. (JMarc)
#include "LString.h"

#ifdef HAVE_SSTREAM
#include <sstream>
#else
#include "support/sstream.h"
#endif

using std::istringstream;
using std::ostringstream;
using std::stringstream;

#endif
