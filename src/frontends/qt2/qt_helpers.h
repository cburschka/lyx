// -*- C++ -*-
/**
 * \file qt_helpers.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Dekel Tsur
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef QTHELPERS_H
#define QTHELPERS_H

#include <utility>

#include "lyxlength.h"

class LengthCombo;
class QLineEdit;
class QString;

std::string makeFontName(std::string const & family, std::string const & foundry);

std::pair<std::string,std::string> parseFontName(std::string const & name);

/// method to get a LyXLength from widgets
std::string widgetsToLength(QLineEdit const * input, LengthCombo const * combo);

/// method to set widgets from a LyXLength
void lengthToWidgets(QLineEdit * input, LengthCombo * combo,
	std::string const & len, LyXLength::UNIT default_unit);

/// format a string to the given width
std::string const formatted(std::string const & text, int w = 80);

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
QString const toqstr(std::string const & str);


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
QString const qt_(std::string const & str);


/**
 * fromqstr - convert QString into std::string in locale
 *
 * Return the QString encoded in the locale
 */
std::string const fromqstr(QString const & str);

#endif // QTHELPERS_H
