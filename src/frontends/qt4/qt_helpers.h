// -*- C++ -*-
/**
 * \file qt_helpers.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Dekel Tsur
 * \author Richard Heck
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef QTHELPERS_H
#define QTHELPERS_H

#include "Length.h"
#include "support/docstring.h"
#include "support/qstring_helpers.h"

#include <QString>
#include <utility>

class QComboBox;
class QLineEdit;
class QCheckBox;

class LengthCombo;

namespace lyx {

std::string makeFontName(std::string const & family, std::string const & foundry);

std::pair<std::string,std::string> parseFontName(std::string const & name);

/// method to get a Length from widgets (LengthCombo)
std::string widgetsToLength(QLineEdit const * input, LengthCombo const * combo);
/// method to get a Length from widgets (QComboBox)
Length widgetsToLength(QLineEdit const * input, QComboBox const * combo);

//FIXME It would be nice if defaultUnit were a default argument
/// method to set widgets from a Length
void lengthToWidgets(QLineEdit * input, LengthCombo * combo,
	Length const & len, Length::UNIT default_unit);
/// method to set widgets from a string
void lengthToWidgets(QLineEdit * input, LengthCombo * combo,
	std::string const & len, Length::UNIT default_unit);
/// method to set widgets from a Length with optional "auto" if zero
void lengthAutoToWidgets(QLineEdit * input, LengthCombo * combo,
	Length const & len, Length::UNIT defaultUnit);

//FIXME setAutoTextCB should really take an argument, as indicated, that
//determines what text is to be written for "auto". But making
//that work involves more extensive revisions than we now want
//to make, since "auto" also appears in update_contents() (see
//GuiGraphics.cpp).
//The right way to do this, I think, would be to define a class
//checkedLengthSet (and a partnering labeledLengthSete) that encapsulated
//the checkbox, line edit, and length combo together, and then made e.g.
//lengthToWidgets, widgetsToLength, etc, all public methods of that class.
//Perhaps even the validator could be exposed through it.
/**
 * sets a checkbox-line edit-length combo group, using "text" if the
 * checkbox is unchecked and clearing the line edit if it previously
 * said "text".
*/
void setAutoTextCB(QCheckBox * checkBox, QLineEdit * lineEdit,
	LengthCombo * lengthCombo/*, string text = "auto"*/);


/// format a string to the given width
docstring const formatted(docstring const & text, int w = 80);


/**
 * qt_ - i18nize string and convert to QString
 *
 * Use this in qt4/ instead of _()
 */
QString const qt_(char const * str, const char * comment = 0);


/**
 * qt_ - i18nize string and convert to QString
 *
 * Use this in qt4/ instead of _()
 */
QString const qt_(std::string const & str);

} // namespace lyx

#endif // QTHELPERS_H
