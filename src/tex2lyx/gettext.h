// -*- C++ -*-
/**
 * \file gettext.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Lars Gullik Bjønnes
 * \author Jean-Marc Lasgouttes
 *
 * Full author contact details are available in file CREDITS
 */
#ifndef GETTEXT_H
#define GETTEXT_H

#include "LString.h"

///
string const _(string const &);

#define N_(str) (str)              // for detecting static strings

///
void locale_init();

#endif
