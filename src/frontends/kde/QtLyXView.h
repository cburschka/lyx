/**
 * \file QtLyXView.h
 * Copyright 2001 the LyX Team
 * Read the file COPYING
 *
 * \author John Levon
 */

/* Qt has a messy namespace which conflicts with 'emit' in
 * Timeout. Yep, they #define emit to nothing.
 */
#undef emit
#include "LyXView.h"
