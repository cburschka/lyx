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
//#include "lengthcombo.h"

#include "support/types.h"

#include <vector>

class LengthCombo;
class QComboBox;
class QLineEdit;
class QString;
class QChar;

std::string makeFontName(std::string const & family, std::string const & foundry);

std::pair<std::string,std::string> parseFontName(std::string const & name);

/// method to get a LyXLength from widgets (LengthCombo)
std::string widgetsToLength(QLineEdit const * input, LengthCombo const * combo);
/// method to get a LyXLength from widgets (QComboBox)
LyXLength widgetsToLength(QLineEdit const * input, QComboBox const * combo);

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
 * toqstr - convert ucs4 into QString
 *
 * QString uses ucs2 (a.k.a utf16) internally.
 */
QString const ucs4_to_qstring(lyx::char_type const * str, size_t ls);

QString const ucs4_to_qstring(std::vector<lyx::char_type> const & ucs4);

std::vector<lyx::char_type> qstring_to_ucs4(QString const & qstr);

void qstring_to_ucs4(QString const & qstr, std::vector<lyx::char_type> & ucs4);

lyx::char_type const qchar_to_ucs4(QChar const & qchar);

QChar const ucs4_to_qchar(lyx::char_type const & ucs4);


/**
 * qt_ - i18nize string and convert to unicode
 *
 * Use this in qt4/ instead of qt_()
 */
QString const qt_(char const * str);


/**
 * qt_ - i18nize string and convert to unicode
 *
 * Use this in qt4/ instead of qt_()
 */
QString const qt_(std::string const & str);


/**
 * fromqstr - convert QString into std::string in locale
 *
 * Return the QString encoded in the locale
 */
std::string const fromqstr(QString const & str);

#endif // QTHELPERS_H
