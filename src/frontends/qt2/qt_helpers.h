// -*- C++ -*-
/**
 * \file qt_helpers.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Dekel Tsur
 *
 * Full author contact details are available in file CREDITS
 */

#ifndef QTHELPERS_H
#define QTHELPERS_H

#ifdef __GNUG__
#pragma interface
#endif

#include "LString.h"

#include <utility>

string makeFontName(string const & family, string const & foundry);
std::pair<string,string> parseFontName(string const & name);

#endif // QTHELPERS_H
