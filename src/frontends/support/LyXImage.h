// -*- C++ -*-
/* This file is part of
 * =================================================
 * 
 *          LyX, The Document Processor
 *          Copyright 1995 Matthias Ettrich.
 *          Copyright 1995-2000 The LyX Team.
 *
 *          This file Copyright 2000 Baruch Even
 * ================================================= */

#ifndef LYXIMAGE_H
#define LYXIMAGE_H

// We need it to know what version to use.
#include <config.h>

#ifndef X_DISPLAY_MISSING
	#include "LyXImage_X.h"
#elif defined(DO_WINDOWS)
	// Not implemented, for illustration only.
	#include "LyXImage_Windows.h"
#endif

#endif
