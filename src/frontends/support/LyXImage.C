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

#ifdef __GNUG__
#pragma implementation
#endif

#include <config.h>
#include "LyXImage.h"


#ifndef X_DISPLAY_MISSING
	#include "LyXImage_X.C"
#elif defined(DO_WINDOWS)
	// Not implemented, for illustration only.
	#include "LyXImage_Windows.C"
#endif
