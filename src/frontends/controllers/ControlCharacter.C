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

#include "ViewBase.h"
#include "ButtonControllerBase.h"
#include "ControlCharacter.h"
#include "buffer.h"
#include "Dialogs.h"
#include "Liason.h"
#include "LyXView.h"
#include "bufferview_funcs.h" // ToggleAndShow
#include "gettext.h"
#include "language.h"

using Liason::setMinibuffer;
using SigC::slot;
using std::vector;

ControlCharacter::ControlCharacter(LyXView & lv, Dialogs & d)
	: ControlDialog<ControlConnectBD>(lv, d),
	  font_(0), toggleall_(false)
{
	d_.showLayoutCharacter.connect(slot(this, &ControlCharacter::show));
	d_.setUserFreeFont.connect(slot(this, &ControlCharacter::apply));
}


void ControlCharacter::setParams()
{
	if (font_) delete font_;
	font_ = new LyXFont(LyXFont::ALL_IGNORE);
}


void ControlCharacter::clearParams()
{
	if (font_) {
		delete font_;
		font_ = 0;
	}
}


void ControlCharacter::apply()
{
	if (!(font_ && lv_.view()->available()))
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

void ControlCharacter::setBar(character::FONT_STATE val)
{
	switch (val) {
	case character::IGNORE:
		font_->setEmph(LyXFont::IGNORE);
		font_->setUnderbar(LyXFont::IGNORE);
		font_->setNoun(LyXFont::IGNORE);
		font_->setLatex(LyXFont::IGNORE);
		break;

	case character::EMPH_TOGGLE:
		font_->setEmph(LyXFont::TOGGLE);
		break;

	case character::UNDERBAR_TOGGLE:
		font_->setUnderbar(LyXFont::TOGGLE);
		break;

	case character::NOUN_TOGGLE:
		font_->setNoun(LyXFont::TOGGLE);
		break;

	case character::LATEX_TOGGLE:
		font_->setLatex(LyXFont::TOGGLE);
		break;

	case character::INHERIT:
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
