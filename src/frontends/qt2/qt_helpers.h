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

#include "lyxlength.h"
 
class LengthCombo;
class QLineEdit;
 
string makeFontName(string const & family, string const & foundry);
 
std::pair<string,string> parseFontName(string const & name);

/// method to get a LyXLength from widgets
string widgetsToLength(QLineEdit const * input, LengthCombo const * combo);
 
/// method to set widgets from a LyXLength
void lengthToWidgets(QLineEdit * input, LengthCombo * combo,
	string const & len, LyXLength::UNIT default_unit);
 
#endif // QTHELPERS_H
