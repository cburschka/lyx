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


#ifndef LOSTREAM_H
#define LOSTREAM_H 

#ifdef HAVE_OSTREAM
#include <ostream>
#else 
#include <iostream>
#endif

using std::ostream;
using std::endl; 
using std::flush;

#endif
