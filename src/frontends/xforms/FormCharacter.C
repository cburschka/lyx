/**
 * \file FormCharacter.C
 * Copyright 2001 The LyX Team.
 * See the file COPYING.
 *
 * \author Edwin Leuven,  leuven@fee.uva.nl
 * \author Angus Leeming, a.leeming@ic.ac.uk
 */

#include <vector>

#ifdef __GNUG__
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
#include "frnt_lang.h"

#include "support/lstrings.h"

using std::vector;
using std::find;

using namespace frnt;

typedef FormCB<ControlCharacter, FormDB<FD_form_character> > base_class;

FormCharacter::FormCharacter(ControlCharacter & c)
	: base_class(c, _("Character Layout"), false)
{}


void FormCharacter::ComboInputCB(int, void * v, Combox * combox)
{
	FormCharacter * pre = static_cast<FormCharacter*>(v);
	pre->InputCB(reinterpret_cast<FL_OBJECT *>(combox), 0);
}

void FormCharacter::build()
{
	dialog_.reset(build_character());

	vector<FamilyPair>   const family = getFamilyData();
	vector<SeriesPair>   const series = getSeriesData();
	vector<ShapePair>    const shape  = getShapeData();
	vector<SizePair>     const size   = getSizeData();
	vector<BarPair>      const bar    = getBarData();
	vector<ColorPair>    const color  = getColorData();
	vector<LanguagePair> const langs  = getLanguageData(true);

	// Store the identifiers for later
	family_ = getSecond(family);
	series_ = getSecond(series);
	shape_  = getSecond(shape);
	size_   = getSecond(size);
	bar_    = getSecond(bar);
	color_  = getSecond(color);
	lang_   = getSecond(langs);

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
	fl_addto_choice(dialog_->choice_language, "prevent crash");

	// insert default language box manually
	fl_addto_form(dialog_->form);
	FL_OBJECT * ob = dialog_->choice_language;
	fl_hide_object(dialog_->choice_language);

	combo_language2_.reset(new Combox(FL_COMBOX_DROPLIST));
	combo_language2_->add(ob->x, ob->y, ob->w, ob->h, 250);
	combo_language2_->shortcut("#L", 1);
	combo_language2_->setcallback(ComboInputCB, this);
	fl_end_form();

	// build up the combox entries
	vector<LanguagePair>::const_iterator it  = langs.begin();
	vector<LanguagePair>::const_iterator end = langs.end();
	for (; it != end; ++it) {
		combo_language2_->addto(it->first);
	}
	combo_language2_->select(1);

	// Manage the ok, apply and cancel/close buttons
	bc().setApply(dialog_->button_apply);
	bc().setCancel(dialog_->button_close);
	bc().addReadOnly(dialog_->check_toggle_all);
}


void FormCharacter::apply()
{
	if (!form()) return;

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

	pos = combo_language2_->get();
	controller().setLanguage(lang_[pos-1]);

	bool const toggleall = fl_get_button(dialog_->check_toggle_all);
	controller().setToggleAll(toggleall);
}


namespace {

template<class A>
typename vector<A>::size_type findPos(vector<A> const & vec, A const & val)
{
	typename vector<A>::const_iterator it =
		find(vec.begin(), vec.end(), val);
	if (it == vec.end())
		return 0;
	return it - vec.begin();
}

} // namespace anon


void FormCharacter::update()
{
	int pos = int(findPos(family_, controller().getFamily()));
	fl_set_choice(dialog_->choice_family, pos+1);

	pos = int(findPos(series_, controller().getSeries()));
	fl_set_choice(dialog_->choice_series, pos+1);

	pos = int(findPos(shape_, controller().getShape()));
	fl_set_choice(dialog_->choice_shape, pos+1);

	pos = int(findPos(size_, controller().getSize()));
	fl_set_choice(dialog_->choice_size, pos+1);

	pos = int(findPos(bar_, controller().getBar()));
	fl_set_choice(dialog_->choice_bar, pos+1);

	pos = int(findPos(color_, controller().getColor()));
	fl_set_choice(dialog_->choice_color, pos+1);

	pos = int(findPos(lang_, controller().getLanguage()));
	combo_language2_->select(pos+1);

	fl_set_button(dialog_->check_toggle_all, controller().getToggleAll());
}


ButtonPolicy::SMInput FormCharacter::input(FL_OBJECT *, long)
{
	ButtonPolicy::SMInput activate = ButtonPolicy::SMI_NOOP;

	int pos = fl_get_choice(dialog_->choice_family);
	if (family_[pos-1] != LyXFont::IGNORE_FAMILY)
		activate = ButtonPolicy::SMI_VALID;

	pos = fl_get_choice(dialog_->choice_series);
	if (series_[pos-1] != LyXFont::IGNORE_SERIES)
		activate = ButtonPolicy::SMI_VALID;

	pos = fl_get_choice(dialog_->choice_shape);
	if (shape_[pos-1] != LyXFont::IGNORE_SHAPE)
		activate = ButtonPolicy::SMI_VALID;

	pos = fl_get_choice(dialog_->choice_size);
	if (size_[pos-1] != LyXFont::IGNORE_SIZE)
		activate = ButtonPolicy::SMI_VALID;

	pos = fl_get_choice(dialog_->choice_bar);
	if (bar_[pos-1] != frnt::IGNORE)
		activate = ButtonPolicy::SMI_VALID;

	pos = fl_get_choice(dialog_->choice_color);
	if (color_[pos-1] != LColor::ignore)
		activate = ButtonPolicy::SMI_VALID;

	pos = combo_language2_->get();
	if (lang_[pos-1] != "No change")
		activate = ButtonPolicy::SMI_VALID;

	return activate;
}
