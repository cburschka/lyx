/** 
 * \file FormCharacter.h
 * Copyright 2001 The LyX Team.
 * See the file COPYING.
 * 
 * \author Edwin Leuven, leuven@fee.uva.nl
 * \author Angus Leeming, a.leeming@.ac.uk
 */

#ifndef FORM_CHARACTER_H
#define FORM_CHARACTER_H

#include <boost/smart_ptr.hpp>

#ifdef __GNUG_
#pragma interface
#endif

#include "FormBase.h"
#include "lyxfont.h"          // for LyXFont enums
#include "ControlCharacter.h" // for ControlCharacter enum
#include "LColor.h"           // for LColor enum

class Combox;
struct FD_form_character;

/** 
 * This class provides an XForms implementation of the FormCharacter Dialog.
 * The character dialog allows users to change the character settings
 * in their documents.
 */
class FormCharacter
	: public FormCB<ControlCharacter, FormDB<FD_form_character> > {
public:
	///
	FormCharacter(ControlCharacter &);
private:

	/// Build the popup
	virtual void build();
   
	/// Apply from popup
	virtual void apply();
   
	/// Nothing to update...
	virtual void update() {}

	/** Callback method (used only to activate Apply button when
	    combox is changed */
	static void ComboInputCB(int, void *, Combox *);

	/// Type definition from the fdesign produced header file.
	FD_form_character * build_character();
	
	///
	boost::scoped_ptr<Combox> combo_language2_;
	///
	std::vector<LyXFont::FONT_FAMILY>         family_;
	///
	std::vector<LyXFont::FONT_SERIES>         series_;
	///
	std::vector<LyXFont::FONT_SHAPE>          shape_;
	///
	std::vector<LyXFont::FONT_SIZE>           size_;
	///
	std::vector<ControlCharacter::FONT_STATE> bar_;
	///
	std::vector<LColor::color>                color_;
};

#endif
