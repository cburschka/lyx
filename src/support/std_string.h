// -*- C++ -*-
/**
 * \file std_string.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Lars Gullik Bjønnes
 * \author Jean-Marc Lasgouttes
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef STD_STRING_H
#define STD_STRING_H

#if 0
#ifndef _CONFIG_H
#error The <config.h> header should always be included before std_string.h
#endif
#endif

#ifndef USE_INCLUDED_STRING

#include <string>
using std::string;
#define STRCONV(STR) STR
#else

#ifdef __STRING__
#error The <string> header has been included before std_string.h
#else
#define __STRING__
#endif
#include "support/lyxstring.h"
using lyx::string;
#define STRCONV(STR) STR.c_str()
#endif

#endif // NOT STD_STRING_H
