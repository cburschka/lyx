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


#include <string>

#ifdef USE_INCLUDED_STRING

using std::string;

#else

namespace lyx {

using std::string;

}

using lyx::string;

#endif

#endif // NOT STD_STRING_H
