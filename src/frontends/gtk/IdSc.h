// -*- C++ -*-
/**
 * \file IdSc.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Huang Ying
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef ID_SC_H
#define ID_SC_H

#include "support/std_string.h"

namespace id_sc
{

/// Extract shortcut from "<identifer>|<shortcut>" string
string const shortcut(string const &);

/// Extract identifier from "<identifer>|<shortcut>" string
string const id(string const &);

}

#endif
