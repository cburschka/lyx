// -*- C++ -*-
/** 
 * \file FormCharacter.h
 * Copyright 2001 The LyX Team.
 * See the file COPYING.
 * 
 * \author Edwin Leuven, leuven@fee.uva.nl
 */

#ifndef FORM_CHARACTER_H
#define FORM_CHARACTER_H

#include <boost/smart_ptr.hpp>

#include "FormBaseDeprecated.h"
#include "xformsBC.h"

#ifdef __GNUG_
#pragma interface
#endif

class LyXView;
class Dialogs;
class Combox;
struct FD_form_character;

/** 
 * This class provides an XForms implementation of the FormCharacter Dialog.
 * The character dialog allows users to change the character settings
 * in their documents.
 */
class FormCharacter : public FormBaseBD {
public:
	///
	FormCharacter(LyXView *, Dialogs *);
private:
	
	/// Pointer to the actual instantiation of the ButtonController.
	virtual xformsBC & bc();

	/// Build the popup
	virtual void build();
   
	/// Apply from popup
	virtual void apply();
   
	/// Update the popup.
	virtual void update();
   
	/// Pointer to the actual instantiation of the xform's form
	virtual FL_FORM * form() const;

	///
	FD_form_character * build_character();
	
	/// Real GUI implementation.
	boost::scoped_ptr<FD_form_character> dialog_;

	///
	boost::scoped_ptr<Combox> combo_language2_;

	/// The ButtonController
	ButtonController<NoRepeatedApplyReadOnlyPolicy, xformsBC> bc_;
};


inline
xformsBC & FormCharacter::bc()
{
	return bc_;
}
#endif
