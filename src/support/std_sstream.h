// -*- C++ -*-
/**
 * \file std_sstream.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Lars Gullik Bjønnes
 * \author Jean-Marc Lasgouttes
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef STD_SSTREAM_H
#define STD_SSTREAM_H

// Since we will include a string header anyway, we'd better do it
// right now so that <string> is not loaded before lyxstring.h. (JMarc)
#include "support/std_string.h"

#ifdef HAVE_SSTREAM
#include <sstream>
#else
#include "support/sstream.h"
#endif

#endif // NOT STD_SSTREAM_H
