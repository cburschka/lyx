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

#include "FormBase.h"

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
   ///
   ~FormCharacter();

private:
	
   /// Build the popup
   virtual void build();
   
   /// Apply from popup
   virtual void apply();
   
   /// Update the popup.
   virtual void update();
   
   /// Pointer to the actual instantiation of the xforms form
   virtual FL_FORM * form() const;

   FD_form_character * build_character();
	
   /// Real GUI implementation.
   FD_form_character * dialog_;
   
   Combox * combo_language2_;
};

#endif
