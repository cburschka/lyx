// -*- C++ -*-
/**
 * \file forms_gettext.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Angus Leeming
 *
 * Full author contact details are available in file CREDITS
 */

#ifndef FORMS_GETTEXT_H
#define FORMS_GETTEXT_H


#include "LString.h"

/// Extract shortcut from "<identifer>|<shortcut>" string
string const scex(string const &);

/// Extract identifier from "<identifer>|<shortcut>" string
string const idex(string const &);

#endif // FORMS_GETTEXT_H
