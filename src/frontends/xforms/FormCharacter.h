// -*- C++ -*-
/**
 * \file FormCharacter.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Edwin Leuven
 * \author Angus Leeming
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef FORM_CHARACTER_H
#define FORM_CHARACTER_H

#include "FormDialogView.h"
#include "ControlCharacter.h" // for ControlCharacter enum

struct LColor_color;
struct FD_character;

/**
 * This class provides an XForms implementation of the Character Dialog.
 * The character dialog allows users to change the character settings
 * in their documents.
 */
class FormCharacter
	: public FormController<ControlCharacter, FormView<FD_character> > {
public:
	///
	FormCharacter(Dialog &);
private:

	/// Apply from dialog
	virtual void apply();

	/// Build the dialog
	virtual void build();

	/// Update the dialog.
	virtual void update();

	/// Filter the inputs on callback from xforms
	virtual ButtonPolicy::SMInput input(FL_OBJECT *, long);

	///
	std::vector<LyXFont::FONT_FAMILY>  family_;
	///
	std::vector<LyXFont::FONT_SERIES>  series_;
	///
	std::vector<LyXFont::FONT_SHAPE>   shape_;
	///
	std::vector<LyXFont::FONT_SIZE>    size_;
	///
	std::vector<frnt::FONT_STATE>      bar_;
	///
	std::vector<LColor_color>            color_;
	///
	std::vector<std::string>                lang_;
};

#endif
