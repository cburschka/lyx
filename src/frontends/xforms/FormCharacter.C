/**
 * \file FormCharacter.C
 * Copyright 2001 The LyX Team.
 * See the file COPYING.
 *
 * \author Edwin Leuven,  leuven@fee.uva.nl
 * \author Angus Leeming, a.leeming@ic.ac.uk
 */

#include <vector>

#ifdef __GNUG_
#pragma implementation
#endif

#include <config.h>

#include "xformsBC.h"
#include "ControlCharacter.h"
#include "FormCharacter.h"
#include "form_character.h"
#include "gettext.h"
#include "combox.h"
#include "helper_funcs.h"

using std::vector;
using std::back_inserter;
using std::transform;

typedef FormCB<ControlCharacter, FormDB<FD_form_character> > base_class;

FormCharacter::FormCharacter(ControlCharacter & c)
	: base_class(c, _("Character Layout"))
{}


void FormCharacter::ComboInputCB(int, void * v, Combox * combox)
{
	FormCharacter * pre = static_cast<FormCharacter*>(v);
	pre->InputCB(reinterpret_cast<FL_OBJECT *>(combox), 0);
}

void FormCharacter::build()
{
	dialog_.reset(build_character());

	vector<ControlCharacter::FamilyPair> const family = getFamilyData();
	vector<ControlCharacter::SeriesPair> const series = getSeriesData();
	vector<ControlCharacter::ShapePair>  const shape  = getShapeData();
	vector<ControlCharacter::SizePair>   const size   = getSizeData();
	vector<ControlCharacter::BarPair>    const bar    = getBarData();
	vector<ControlCharacter::ColorPair>  const color  = getColorData();
	vector<string> const language = getLanguageData();

	// Store the enums for later
	family_ = getSecond(family);
	series_ = getSecond(series);
	shape_ = getSecond(shape);
	size_ = getSecond(size);
	bar_ = getSecond(bar);
	color_ = getSecond(color);

	// create a string of entries " entry1 | entry2 | entry3 | entry4 "
	// with which to initialise the xforms choice object.
	string choice = " " + getStringFromVector(getFirst(family), " | ") +" ";
	fl_addto_choice(dialog_->choice_family, choice.c_str());

	choice = " " + getStringFromVector(getFirst(series), " | ") + " ";
	fl_addto_choice(dialog_->choice_series, choice.c_str());

	choice = " " + getStringFromVector(getFirst(shape), " | ") + " ";
	fl_addto_choice(dialog_->choice_shape, choice.c_str());

	choice = " " + getStringFromVector(getFirst(size), " | ") + " ";
	fl_addto_choice(dialog_->choice_size, choice.c_str());

	choice = " " + getStringFromVector(getFirst(bar), " | ") + " ";
	fl_addto_choice(dialog_->choice_bar, choice.c_str());

	choice = " " + getStringFromVector(getFirst(color), " | ") + " ";
	fl_addto_choice(dialog_->choice_color, choice.c_str());

	// xforms appears to need this to prevent a crash...
	// fl_addto_choice(dialog_->choice_language,
	//		_(" English %l| German | French "));

	// insert default language box manually
	fl_addto_form(dialog_->form);
		FL_OBJECT * ob = dialog_->choice_language;
		combo_language2_.reset(new Combox(FL_COMBOX_DROPLIST));
		combo_language2_->add(ob->x, ob->y, ob->w, ob->h, 250);
		combo_language2_->shortcut("#L", 1);
		combo_language2_->setcallback(ComboInputCB, this);
	fl_end_form();

	// build up the combox entries
	for (vector<string>::const_iterator cit = language.begin(); 
	     cit != language.end(); ++cit) {
		combo_language2_->addto(*cit);
	}
	combo_language2_->select_text(*language.begin());

	// Manage the ok, apply and cancel/close buttons
	bc().setApply(dialog_->button_apply);
	bc().setCancel(dialog_->button_close);
	bc().refresh();
	bc().addReadOnly(dialog_->check_toggle_all);
}


void FormCharacter::apply()
{
	int pos = fl_get_choice(dialog_->choice_family);
	controller().setFamily(family_[pos-1]);

	pos = fl_get_choice(dialog_->choice_series);
	controller().setSeries(series_[pos-1]);
   
	pos = fl_get_choice(dialog_->choice_shape);
	controller().setShape(shape_[pos-1]);

	pos = fl_get_choice(dialog_->choice_size);
	controller().setSize(size_[pos-1]);
   
	pos = fl_get_choice(dialog_->choice_bar);
	controller().setBar(bar_[pos-1]);
   
	pos = fl_get_choice(dialog_->choice_color);
	controller().setColor(color_[pos-1]);

	controller().setLanguage(combo_language2_->getline());

	bool const toggleall = fl_get_button(dialog_->check_toggle_all);
	controller().setToggleAll(toggleall);
}
