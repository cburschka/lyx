/**
 * \file FormCharacter.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Edwin Leuven
 * \author Angus Leeming
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "FormCharacter.h"
#include "forms/form_character.h"

#include "xforms_helpers.h"
#include "xformsBC.h"

#include "controllers/frnt_lang.h"
#include "controllers/helper_funcs.h"

#include "support/lstrings.h"

#include "lyx_forms.h"
#include "combox.h"

using frnt::BarPair;
using frnt::ColorPair;
using frnt::FamilyPair;
using frnt::getBarData;
using frnt::getColorData;
using frnt::getFamilyData;
using frnt::getLanguageData;
using frnt::getSeriesData;
using frnt::getShapeData;
using frnt::getSizeData;
using frnt::LanguagePair;
using frnt::SeriesPair;
using frnt::ShapePair;
using frnt::SizePair;

using lyx::support::getStringFromVector;

using std::vector;


typedef FormController<ControlCharacter, FormView<FD_character> > base_class;

FormCharacter::FormCharacter(Dialog & parent)
	: base_class(parent, _("Text Style"), false)
{}


void FormCharacter::build()
{
	dialog_.reset(build_character(this));

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
	string choice = ' ' + getStringFromVector(getFirst(family), " | ") + ' ';
	fl_addto_choice(dialog_->choice_family, choice.c_str());

	choice = ' ' + getStringFromVector(getFirst(series), " | ") + ' ';
	fl_addto_choice(dialog_->choice_series, choice.c_str());

	choice = ' ' + getStringFromVector(getFirst(shape), " | ") + ' ';
	fl_addto_choice(dialog_->choice_shape, choice.c_str());

	choice = ' ' + getStringFromVector(getFirst(size), " | ") + ' ';
	fl_addto_choice(dialog_->choice_size, choice.c_str());

	choice = ' ' + getStringFromVector(getFirst(bar), " | ") + ' ';
	fl_addto_choice(dialog_->choice_bar, choice.c_str());

	choice = ' ' + getStringFromVector(getFirst(color), " | ") + ' ';
	fl_addto_choice(dialog_->choice_color, choice.c_str());

	choice = ' ' + getStringFromVector(getFirst(langs), " | ") + ' ';
	fl_addto_combox(dialog_->combox_language, choice.c_str());
	fl_set_combox_browser_height(dialog_->combox_language, 250);

	// Manage the ok, apply and cancel/close buttons
	bcview().setApply(dialog_->button_apply);
	bcview().setCancel(dialog_->button_close);
	bcview().addReadOnly(dialog_->check_toggle_all);
}


void FormCharacter::apply()
{
	if (!form()) return;

	int pos = fl_get_choice(dialog_->choice_family);
	controller().setFamily(family_[pos - 1]);

	pos = fl_get_choice(dialog_->choice_series);
	controller().setSeries(series_[pos - 1]);

	pos = fl_get_choice(dialog_->choice_shape);
	controller().setShape(shape_[pos - 1]);

	pos = fl_get_choice(dialog_->choice_size);
	controller().setSize(size_[pos - 1]);

	pos = fl_get_choice(dialog_->choice_bar);
	controller().setBar(bar_[pos - 1]);

	pos = fl_get_choice(dialog_->choice_color);
	controller().setColor(color_[pos - 1]);

	pos = fl_get_combox(dialog_->combox_language);
	controller().setLanguage(lang_[pos - 1]);

	bool const toggleall = fl_get_button(dialog_->check_toggle_all);
	controller().setToggleAll(toggleall);
}


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
	fl_set_combox(dialog_->combox_language, pos+1);

	fl_set_button(dialog_->check_toggle_all, controller().getToggleAll());
}


ButtonPolicy::SMInput FormCharacter::input(FL_OBJECT *, long)
{
	ButtonPolicy::SMInput activate = ButtonPolicy::SMI_NOOP;

	int pos = fl_get_choice(dialog_->choice_family);
	if (family_[pos - 1] != LyXFont::IGNORE_FAMILY)
		activate = ButtonPolicy::SMI_VALID;

	pos = fl_get_choice(dialog_->choice_series);
	if (series_[pos - 1] != LyXFont::IGNORE_SERIES)
		activate = ButtonPolicy::SMI_VALID;

	pos = fl_get_choice(dialog_->choice_shape);
	if (shape_[pos - 1] != LyXFont::IGNORE_SHAPE)
		activate = ButtonPolicy::SMI_VALID;

	pos = fl_get_choice(dialog_->choice_size);
	if (size_[pos - 1] != LyXFont::IGNORE_SIZE)
		activate = ButtonPolicy::SMI_VALID;

	pos = fl_get_choice(dialog_->choice_bar);
	if (bar_[pos - 1] != frnt::IGNORE)
		activate = ButtonPolicy::SMI_VALID;

	pos = fl_get_choice(dialog_->choice_color);
	if (color_[pos - 1] != LColor::ignore)
		activate = ButtonPolicy::SMI_VALID;

	pos = fl_get_combox(dialog_->combox_language);
	if (lang_[pos - 1] != "No change")
		activate = ButtonPolicy::SMI_VALID;

	return activate;
}
