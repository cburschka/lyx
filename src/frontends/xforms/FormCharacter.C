/**
 * \file FormCharacter.C
 * Copyright 2001 The LyX Team.
 * See the file COPYING.
 *
 * \author Edwin Leuven, leuven@fee.uva.nl
 */

#include <config.h>

#ifdef __GNUG_
#pragma implementation
#endif

#include FORMS_H_LOCATION

#include "FormCharacter.h"
#include "form_character.h"
#include "bufferview_funcs.h"
#include "Dialogs.h"
#include "Liason.h"
#include "LyXView.h"
#include "buffer.h"
#include "lyxtext.h"
#include "language.h"
#include "combox.h"

#ifdef CXX_WORKING_NAMESPACES
using Liason::setMinibuffer;
#endif


FormCharacter::FormCharacter(LyXView * lv, Dialogs * d)
	: FormBaseBD(lv, d, _("Character Layout"),
		     new NoRepeatedApplyReadOnlyPolicy),
	dialog_(0), combo_language2_(0)
{
   // let the popup be shown
   // This is a permanent connection so we won't bother
   // storing a copy because we won't be disconnecting.
   d->showLayoutCharacter.connect(slot(this, &FormCharacter::show));
   // for LFUN_FREE
   d->setUserFreeFont.connect(slot(this, &FormCharacter::apply));
}


FormCharacter::~FormCharacter()
{
	// This must be done before the deletion of the dialog.
	delete combo_language2_;
	delete dialog_;
}


FL_FORM * FormCharacter::form() const
{
   if (dialog_) 
     return dialog_->form;
   return 0;
}


void FormCharacter::build()
{
   dialog_ = build_character();
   // Workaround dumb xforms sizing bug
   minw_ = form()->w;
   minh_ = form()->h;
   // Manage the ok, apply and cancel/close buttons
   bc_.setApply(dialog_->button_apply);
   bc_.setCancel(dialog_->button_close);
   bc_.refresh();
   bc_.addReadOnly (dialog_->check_toggle_all);
   
   fl_addto_choice(dialog_->choice_family,
		   _(" No change %l| Roman | Sans Serif | Typewriter %l| Reset "));
   fl_addto_choice(dialog_->choice_series,
		   _(" No change %l| Medium | Bold %l| Reset "));
   fl_addto_choice(dialog_->choice_shape,
		   _(" No change %l| Upright | Italic | Slanted | Small Caps "
		     "%l| Reset "));
   fl_addto_choice(dialog_->choice_size,
		   _(" No change %l| Tiny | Smallest | Smaller | Small "
		     "| Normal | Large | Larger | Largest | Huge | Huger "
		     "%l| Increase | Decrease | Reset "));
   fl_addto_choice(dialog_->choice_bar,
		   _(" No change %l| Emph | Underbar | Noun | LaTeX mode %l| Reset "));
   fl_addto_choice(dialog_->choice_color,
		   _(" No change %l| No color | Black | White | Red | Green "
		     "| Blue | Cyan | Magenta | Yellow %l| Reset "));
   fl_addto_choice(dialog_->choice_language,
		   _(" English %l| German | French "));

   // insert default language box manually
   fl_addto_form(dialog_->form);
   FL_OBJECT * ob = dialog_->choice_language;
   combo_language2_ = new Combox(FL_COMBOX_DROPLIST);
   combo_language2_->add(ob->x, ob->y, ob->w, ob->h, 250);
   combo_language2_->shortcut("#L", 1);
   fl_end_form();

   // build up the combox entries
   combo_language2_->addline(_("No change"));
   combo_language2_->addline(_("Reset"));
   for (Languages::const_iterator cit = languages.begin(); 
	cit != languages.end(); ++cit) {
	combo_language2_->addto((*cit).second.lang());
   }
}


void FormCharacter::apply()
{
   if (!lv_->view()->available() || !dialog_)
     return;
   
   LyXFont font(LyXFont::ALL_IGNORE);
   
   int pos = fl_get_choice(dialog_->choice_family);
   switch (pos) {
      case 1: font.setFamily(LyXFont::IGNORE_FAMILY); break;
      case 2: font.setFamily(LyXFont::ROMAN_FAMILY); break;
      case 3: font.setFamily(LyXFont::SANS_FAMILY); break;
      case 4: font.setFamily(LyXFont::TYPEWRITER_FAMILY); break;
      case 5: font.setFamily(LyXFont::INHERIT_FAMILY); break;
   }
   
   pos = fl_get_choice(dialog_->choice_series);
   switch (pos) {
      case 1: font.setSeries(LyXFont::IGNORE_SERIES); break;
      case 2: font.setSeries(LyXFont::MEDIUM_SERIES); break;
      case 3: font.setSeries(LyXFont::BOLD_SERIES); break;
      case 4: font.setSeries(LyXFont::INHERIT_SERIES); break;
   }
   
   pos = fl_get_choice(dialog_->choice_shape);
   switch (pos) {
      case 1: font.setShape(LyXFont::IGNORE_SHAPE); break;
      case 2: font.setShape(LyXFont::UP_SHAPE); break;
      case 3: font.setShape(LyXFont::ITALIC_SHAPE); break;
      case 4: font.setShape(LyXFont::SLANTED_SHAPE); break;
      case 5: font.setShape(LyXFont::SMALLCAPS_SHAPE); break;
      case 6: font.setShape(LyXFont::INHERIT_SHAPE); break;
   }
   
   pos = fl_get_choice(dialog_->choice_size);
   switch (pos) {
      case 1: font.setSize(LyXFont::IGNORE_SIZE); break;
      case 2: font.setSize(LyXFont::SIZE_TINY); break;
      case 3: font.setSize(LyXFont::SIZE_SCRIPT); break;
      case 4: font.setSize(LyXFont::SIZE_FOOTNOTE); break;
      case 5: font.setSize(LyXFont::SIZE_SMALL); break;
      case 6: font.setSize(LyXFont::SIZE_NORMAL); break;
      case 7: font.setSize(LyXFont::SIZE_LARGE); break;
      case 8: font.setSize(LyXFont::SIZE_LARGER); break;
      case 9: font.setSize(LyXFont::SIZE_LARGEST); break;
      case 10: font.setSize(LyXFont::SIZE_HUGE); break;
      case 11: font.setSize(LyXFont::SIZE_HUGER); break;
      case 12: font.setSize(LyXFont::INCREASE_SIZE); break;
      case 13: font.setSize(LyXFont::DECREASE_SIZE); break;
      case 14: font.setSize(LyXFont::INHERIT_SIZE); break;
   }
   
   pos = fl_get_choice(dialog_->choice_bar);
   switch (pos) {
      case 1: font.setEmph(LyXFont::IGNORE);
      font.setUnderbar(LyXFont::IGNORE);
      font.setNoun(LyXFont::IGNORE);
      font.setLatex(LyXFont::IGNORE);
      break;
      case 2: font.setEmph(LyXFont::TOGGLE); break;
      case 3: font.setUnderbar(LyXFont::TOGGLE); break;
      case 4: font.setNoun(LyXFont::TOGGLE); break;
      case 5: font.setLatex(LyXFont::TOGGLE); break;
      case 6: font.setEmph(LyXFont::INHERIT);
      font.setUnderbar(LyXFont::INHERIT);
      font.setNoun(LyXFont::INHERIT);
      font.setLatex(LyXFont::INHERIT);
      break;
   }
   
   pos = fl_get_choice(dialog_->choice_color);
   switch (pos) {
      case 1: font.setColor(LColor::ignore); break;
      case 2: font.setColor(LColor::none); break;
      case 3: font.setColor(LColor::black); break;
      case 4: font.setColor(LColor::white); break;
      case 5: font.setColor(LColor::red); break;
      case 6: font.setColor(LColor::green); break;
      case 7: font.setColor(LColor::blue); break;
      case 8: font.setColor(LColor::cyan); break;
      case 9: font.setColor(LColor::magenta); break;
      case 10: font.setColor(LColor::yellow); break;
      case 11: font.setColor(LColor::inherit); break;
   }
   
   int const choice = combo_language2_->get();
   if (choice == 1)
     font.setLanguage(ignore_language);
   else if (choice == 2)
     font.setLanguage(lv_->buffer()->params.language);
   else
     font.setLanguage(languages.getLanguage(combo_language2_->getline()));

   
   bool toggleall = fl_get_button(dialog_->check_toggle_all);
   ToggleAndShow(lv_->view(), font, toggleall);
   lv_->view()->setState();
   lv_->buffer()->markDirty();
   setMinibuffer(lv_, _("Character set"));
}


void FormCharacter::update()
{
    if (!dialog_)
        return;
   
    bc_.readOnly(lv_->buffer()->isReadonly());
}
