// -*- C++ -*-
/* This file is part of
 * ======================================================
 *
 *           LyX, The Document Processor
 *
 *           Copyright 1995 Matthias Ettrich
 *           Copyright 1995-2001 The LyX Team.
 *
 * ====================================================== */

#ifndef LYXLAYOUT_PTR_FWD_H
#define LYXLAYOUT_PTR_FWD_H

#include <boost/shared_ptr.hpp>

class LyXLayout;

/// Global typedef
typedef boost::shared_ptr<LyXLayout> LyXLayout_ptr;

#endif
