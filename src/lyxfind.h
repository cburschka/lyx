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
               bool const &, bool const & = true, bool const & = false,
               bool const & = false);

bool LyXFind(BufferView *,
             string const & searchstr, bool const & forward,
			 bool const & frominset = false,
             bool const & casesens = true, bool const & matchwrd = false);

#endif
