// -*- C++ -*-
/* lxtl.h
 * LyX eXternalization Template Library
 * This file is part of
 * ====================================================== 
 *
 *           LyX, The Document Processor
 *
 *           Copyright 1995 Matthias Ettrich
 *           Copyright 1995-2000 The LyX Team.
 *
 *           This file Copyright 2000
 *           Allan Rae
 * ======================================================
 */

#ifndef LXTL_H
#define LXTL_H
#include <xtl/autobuf.h>
#include <xtl/objio.h>
#include <xtl/giop.h>
#include <xtl/xdr.h>

// XDR_format causes an abort that's hard to track down.  GDB says the abort
// occurs in code from a different function to the one being run before the
// abort!  (XTL-1.3.pl.11)
///
typedef GIOP_format<auto_mem_buffer> gui_format;

/*  Simplify the use of the XTL.  The caller is responsible for creating their
    own memory buffer.  The buffer type isn't a template parameter because I 
    need/want the forward declared buffer class in some other header files
    thereby avoiding an extra file dependency.
    ARRae 20000423
 */

/// Externalize a structure into a buffer.
template<class Input>
void getInMem(Input const & in, auto_mem_buffer & mb) {
	gui_format gf(mb);
	obj_output<gui_format> output(gf);
	output.simple(in);
	mb.rewind();
}

/// Internalize a structure from a buffer.
template<class Input>
void setFromMem(Input & in, auto_mem_buffer & mb) {
	gui_format gf(mb);
	obj_input<gui_format> input(gf);
	input.simple(in);
}


#endif
