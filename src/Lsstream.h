// -*- C++ -*-
/* This file is part of
 * ======================================================
 * 
 *           LyX, The Document Processor
 * 	 
 *	    Copyright 1995 Matthias Ettrich
 *          Copyright 1995-2000 The LyX Team.
 *
 * ====================================================== */

#ifndef LSSTREAM_H
#define LSSTREAM_H 

#ifdef HAVE_SSTREAM
#include <sstream>
using std::istringstream;
using std::ostringstream;
#else
#include "support/sstream.h"
#endif

#endif
