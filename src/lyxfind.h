// -*- C++ -*-
#ifndef LYXFIND_H
#define LYXFIND_H

#ifdef __GNUG__
#pragma interface
#endif

#include "LString.h"
#include "paragraph.h"

class BufferView;

int LyXReplace(BufferView * bv, string const &, string const &,
               bool, bool = true, bool = false,
               bool = false, bool = false);

bool LyXFind(BufferView *,
             string const & searchstr, bool forward,
	     bool frominset = false, bool casesens = true, 
	     bool matchwrd = false);

#endif
