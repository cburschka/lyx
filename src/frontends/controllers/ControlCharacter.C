/**
 * \file ControlCharacter.C
 * Copyright 2001 The LyX Team.
 * See the file COPYING.
 *
 * \author Angus Leeming, a.leeming@.ac.uk
 */

#include <vector>

#ifdef __GNUG__
#pragma implementation
#endif

#include <config.h>

#include "ControlCharacter.h"
#include "bufferview_funcs.h"
#include "Dialogs.h"
#include "Liason.h"
#include "LyXView.h"
#include "buffer.h"
#include "lyxtext.h"
#include "language.h"

using Liason::setMinibuffer;
using SigC::slot;
using std::vector;

ControlCharacter::ControlCharacter(LyXView & lv, Dialogs & d)
	: ControlConnectBD(lv, d), font_(0), toggleall_(false)
{
	d.showLayoutCharacter.connect(slot(this, &ControlCharacter::show));
	d.setUserFreeFont.connect(slot(this, &ControlCharacter::apply));
}


void ControlCharacter::show()
{
	if (!lv_.view()->available()) return;

	if (font_) delete font_;
	font_ = new LyXFont(LyXFont::ALL_IGNORE);

	bc().readOnly(isReadonly());
	view().show();
}


void ControlCharacter::update()
{
	if (!lv_.view()->available()) return;

	if (font_) delete font_;
	font_ = new LyXFont(LyXFont::ALL_IGNORE);

	bc().readOnly(isReadonly());
	view().update();
}


void ControlCharacter::hide()
{
	if (font_) {
		delete font_;
		font_ = 0;
	}

	disconnect();
	view().hide();
}


void ControlCharacter::apply()
{
	if (!lv_.view()->available())
		return;
   
	view().apply();

	ToggleAndShow(lv_.view(), *font_, toggleall_);
	lv_.view()->setState();
	lv_.buffer()->markDirty();
	setMinibuffer(&lv_, _("Character set"));
}


void ControlCharacter::setFamily(LyXFont::FONT_FAMILY val)
{
	font_->setFamily(val);
}

void ControlCharacter::setSeries(LyXFont::FONT_SERIES val)
{
	font_->setSeries(val);
}

void ControlCharacter::setShape(LyXFont::FONT_SHAPE val)
{
	font_->setShape(val);
}

void ControlCharacter::setSize(LyXFont::FONT_SIZE val)
{
	font_->setSize(val);
}

void ControlCharacter::setBar(ControlCharacter::FONT_STATE val)
{
	switch (val) {
	case IGNORE:
		font_->setEmph(LyXFont::IGNORE);
		font_->setUnderbar(LyXFont::IGNORE);
		font_->setNoun(LyXFont::IGNORE);
		font_->setLatex(LyXFont::IGNORE);
		break;

	case EMPH_TOGGLE:
		font_->setEmph(LyXFont::TOGGLE);
		break;

	case UNDERBAR_TOGGLE:
		font_->setUnderbar(LyXFont::TOGGLE);
		break;

	case NOUN_TOGGLE:
		font_->setNoun(LyXFont::TOGGLE);
		break;

	case LATEX_TOGGLE:
		font_->setLatex(LyXFont::TOGGLE);
		break;

	case INHERIT:
		font_->setEmph(LyXFont::INHERIT);
		font_->setUnderbar(LyXFont::INHERIT);
		font_->setNoun(LyXFont::INHERIT);
		font_->setLatex(LyXFont::INHERIT);
		break;
	}
}

void ControlCharacter::setColor(LColor::color val)
{
	switch (val) {
	case LColor::ignore:
	case LColor::none:
	case LColor::black:
	case LColor::white:
	case LColor::red:
	case LColor::green:
	case LColor::blue:
	case LColor::cyan:
	case LColor::magenta:
	case LColor::yellow:
	case LColor::inherit:
		font_->setColor(val);
		break;

	default:
		break;
	}
}

 	
void ControlCharacter::setLanguage(string const & val)
{
	if (val == _("No change"))
		font_->setLanguage(ignore_language);

	else if ( val == _("Reset"))
		font_->setLanguage(lv_.buffer()->params.language);

	else
		font_->setLanguage(languages.getLanguage(val));
}

void ControlCharacter::setToggleAll(bool t)
{
	toggleall_ = t;
}


vector<ControlCharacter::FamilyPair> const getFamilyData()
{
	vector<ControlCharacter::FamilyPair> family(5);

	ControlCharacter::FamilyPair pr;
	pr.first = _("No change");  pr.second = LyXFont::IGNORE_FAMILY;
	family[0] = pr;
	pr.first = _("Roman");      pr.second = LyXFont::ROMAN_FAMILY;
	family[1] = pr;
	pr.first = _("Sans Serif"); pr.second = LyXFont::SANS_FAMILY;
	family[2] = pr;
	pr.first = _("Typewriter"); pr.second = LyXFont::TYPEWRITER_FAMILY;
	family[3] = pr;
	pr.first = _("Reset");      pr.second = LyXFont::INHERIT_FAMILY;
	family[4] = pr;

	return family;
}


vector<ControlCharacter::SeriesPair> const getSeriesData()
{
	vector<ControlCharacter::SeriesPair> series(4);

	ControlCharacter::SeriesPair pr;
	pr.first = _("No change"); pr.second = LyXFont::IGNORE_SERIES;
	series[0] = pr;
	pr.first = _("Medium");    pr.second = LyXFont::MEDIUM_SERIES;
	series[1] = pr;
	pr.first = _("Bold");      pr.second = LyXFont::BOLD_SERIES;
	series[2] = pr;
	pr.first = _("Reset");     pr.second = LyXFont::INHERIT_SERIES;
	series[3] = pr;
	
	return series;
}


vector<ControlCharacter::ShapePair> const getShapeData()
{
	vector<ControlCharacter::ShapePair> shape(6);

	ControlCharacter::ShapePair pr;
	pr.first = _("No change");  pr.second = LyXFont::IGNORE_SHAPE;
	shape[0] = pr;
	pr.first = _("Upright");    pr.second = LyXFont::UP_SHAPE;
	shape[1] = pr;
	pr.first = _("Italic");     pr.second = LyXFont::ITALIC_SHAPE;
	shape[2] = pr;
	pr.first = _("Slanted");    pr.second = LyXFont::SLANTED_SHAPE;
	shape[3] = pr;
	pr.first = _("Small Caps"); pr.second = LyXFont::SMALLCAPS_SHAPE;
	shape[4] = pr;
	pr.first = _("Reset");      pr.second = LyXFont::INHERIT_SHAPE;
	shape[5] = pr;
	
	return shape;
}


vector<ControlCharacter::SizePair> const getSizeData()
{
	vector<ControlCharacter::SizePair> size(14);

	ControlCharacter::SizePair pr;
	pr.first = _("No change"); pr.second = LyXFont::IGNORE_SIZE;
	size[0] = pr;
	pr.first = _("Tiny");      pr.second = LyXFont::SIZE_TINY;
	size[1] = pr;
	pr.first = _("Smallest");  pr.second = LyXFont::SIZE_SCRIPT;
	size[2] = pr;
	pr.first = _("Smaller");   pr.second = LyXFont::SIZE_FOOTNOTE;
	size[3] = pr;
	pr.first = _("Small");     pr.second = LyXFont::SIZE_SMALL;
	size[4] = pr;
	pr.first = _("Normal");    pr.second = LyXFont::SIZE_NORMAL;
	size[5] = pr;
	pr.first = _("Large");     pr.second = LyXFont::SIZE_LARGE;
	size[6] = pr;
	pr.first = _("Larger");    pr.second = LyXFont::SIZE_LARGER;
	size[7] = pr;
	pr.first = _("Largest");   pr.second = LyXFont::SIZE_LARGEST;
	size[8] = pr;
	pr.first = _("Huge");      pr.second = LyXFont::SIZE_HUGE;
	size[9] = pr;
	pr.first = _("Huger");     pr.second = LyXFont::SIZE_HUGER;
	size[10] = pr;
	pr.first = _("Increase");  pr.second = LyXFont::INCREASE_SIZE;
	size[11] = pr;
	pr.first = _("Decrease");  pr.second = LyXFont::DECREASE_SIZE;
	size[12] = pr;
	pr.first = _("Reset");     pr.second = LyXFont::INHERIT_SIZE;
	size[13] = pr;
	
	return size;
}


vector<ControlCharacter::BarPair> const getBarData()
{
	vector<ControlCharacter::BarPair> bar(6);

	ControlCharacter::BarPair pr;
	pr.first = _("No change");  pr.second = ControlCharacter::IGNORE;
	bar[0] = pr;
	pr.first = _("Emph");       pr.second = ControlCharacter::EMPH_TOGGLE;
	bar[1] = pr;
	pr.first = _("Underbar");   pr.second = ControlCharacter::UNDERBAR_TOGGLE;
	bar[2] = pr;
	pr.first = _("Noun");       pr.second = ControlCharacter::NOUN_TOGGLE;
	bar[3] = pr;
	pr.first = _("LaTeX mode"); pr.second = ControlCharacter::LATEX_TOGGLE;
	bar[4] = pr;
	pr.first = _("Reset");      pr.second = ControlCharacter::INHERIT;
	bar[5] = pr;
	
	return bar;
}


vector<ControlCharacter::ColorPair> const getColorData()
{
	vector<ControlCharacter::ColorPair> color(11);

	ControlCharacter::ColorPair pr;
	pr.first = _("No change"); pr.second = LColor::ignore;
	color[0] = pr;
	pr.first = _("No color");  pr.second = LColor::none;
	color[1] = pr;
	pr.first = _("Black");     pr.second = LColor::black;
	color[2] = pr;
	pr.first = _("White");     pr.second = LColor::white;
	color[3] = pr;
	pr.first = _("Red");       pr.second = LColor::red;
	color[4] = pr;
	pr.first = _("Green");     pr.second = LColor::green;
	color[5] = pr;
	pr.first = _("Blue");      pr.second = LColor::blue;
	color[6] = pr;
	pr.first = _("Cyan");      pr.second = LColor::cyan;
	color[7] = pr;
	pr.first = _("Magenta");   pr.second = LColor::magenta;
	color[8] = pr;
	pr.first = _("Yellow");    pr.second = LColor::yellow;
	color[9] = pr;
	pr.first = _("Reset");     pr.second = LColor::inherit;
	color[10] = pr;
	
	return color;
}


vector<string> const getLanguageData()
{
	vector<string> langs(languages.size() + 2);

	langs[0] = _("No change");
	langs[1] = _("Reset");

	vector<string>::size_type i = 1;
	for (Languages::const_iterator cit = languages.begin(); 
	     cit != languages.end(); ++cit) {
		langs[++i] = cit->second.lang();
	}

	return langs;
}
