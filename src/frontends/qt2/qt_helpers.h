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


#include "LString.h"

#include <utility>

#include "lyxlength.h"
 
class LengthCombo;
class QLineEdit;
class QString;
 
string makeFontName(string const & family, string const & foundry);
 
std::pair<string,string> parseFontName(string const & name);

/// method to get a LyXLength from widgets
string widgetsToLength(QLineEdit const * input, LengthCombo const * combo);
 
/// method to set widgets from a LyXLength
void lengthToWidgets(QLineEdit * input, LengthCombo * combo,
	string const & len, LyXLength::UNIT default_unit);
 
/**
 * toqstr - convert char * into unicode
 *
 * Use this whenever there's a user-visible string that is encoded
 * for the locale (menus, dialogs etc.)
 */
QString const toqstr(char const * str);


/**
 * toqstr - convert string into unicode
 *
 * Use this whenever there's a user-visible string that is encoded
 * for the locale (menus, dialogs etc.)
 */
QString const toqstr(string const & str);


/**
 * qt_ - i18nize string and convert to unicode
 *
 * Use this in qt2/ instead of qt_()
 */
QString const qt_(char const * str);


/**
 * qt_ - i18nize string and convert to unicode
 *
 * Use this in qt2/ instead of qt_()
 */
QString const qt_(string const & str);


/**
 * fromqstr - convert QString into std::string in locale
 *
 * Return the QString encoded in the locale
 */
string const fromqstr(QString const & str);

#endif // QTHELPERS_H
