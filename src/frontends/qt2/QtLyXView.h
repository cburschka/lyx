// -*- C++ -*-
/**
 * \file QtLyXView.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author John Levon
 *
 * Full author contact details are available in file CREDITS
 */

/* Qt has a messy namespace which conflicts with 'emit' in
 * src/Timeout.h. Uggghh.
 */

#undef emit
#undef signals
#include "frontends/LyXView.h"
