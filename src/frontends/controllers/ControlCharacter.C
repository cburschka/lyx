/**
 * \file ControlCharacter.C
 * See the file COPYING.
 *
 * \author Angus Leeming 
 *
 * Full author contact details are available in file CREDITS
 */

#include <config.h>

#ifdef __GNUG__
#pragma implementation
#endif

#include "ControlCharacter.h"

#include "ViewBase.h"
#include "ButtonControllerBase.h"

#include "buffer.h"
#include "bufferview_funcs.h" // ToggleAndShow
#include "gettext.h"
#include "language.h"

#include "frontends/Liason.h"
#include "frontends/LyXView.h"

using Liason::setMinibuffer;


ControlCharacter::ControlCharacter(LyXView & lv, Dialogs & d)
	: ControlDialogBD(lv, d),
	  font_(0), toggleall_(false)
{}


void ControlCharacter::setParams()
{
	// Do this the first time only. Used as a flag for whether or not the
	// view has been built
	if (!font_.get())
		font_.reset(new LyXFont(LyXFont::ALL_IGNORE));

	// so that the user can press Ok
	if (getFamily()   != LyXFont::IGNORE_FAMILY ||
	    getSeries()   != LyXFont::IGNORE_SERIES ||
	    getShape()    != LyXFont::IGNORE_SHAPE  ||
	    getSize()     != LyXFont::IGNORE_SIZE ||
	    getBar()      != frnt::IGNORE ||
	    getColor()    != LColor::ignore ||
	    font_->language() != ignore_language)
		bc().valid();
}


void ControlCharacter::apply()
{
	// Nothing to apply. (Can be called from the Toolbar.)
	if (!font_.get())
		return;

	// Apply from the view if it's visible. Otherwise, use the stored values
	if (bufferIsAvailable())
		view().apply();

	toggleAndShow(bufferview(), *(font_.get()), toggleall_);
	lv_.view_state_changed();
	buffer()->markDirty();
	setMinibuffer(&lv_, _("Character set"));
}


LyXFont::FONT_FAMILY ControlCharacter::getFamily() const
{
	if (!font_.get())
		return LyXFont::IGNORE_FAMILY;
	return font_->family();
}


void ControlCharacter::setFamily(LyXFont::FONT_FAMILY val)
{
	font_->setFamily(val);
}


LyXFont::FONT_SERIES ControlCharacter::getSeries() const
{
	if (!font_.get())
		return LyXFont::IGNORE_SERIES;
	return font_->series();
}


void ControlCharacter::setSeries(LyXFont::FONT_SERIES val)
{
	font_->setSeries(val);
}


LyXFont::FONT_SHAPE ControlCharacter::getShape() const
{
	if (!font_.get())
		return LyXFont::IGNORE_SHAPE;
	return font_->shape();
}


void ControlCharacter::setShape(LyXFont::FONT_SHAPE val)
{
	font_->setShape(val);
}


LyXFont::FONT_SIZE ControlCharacter::getSize() const
{
	if (!font_.get())
		return LyXFont::IGNORE_SIZE;
	return font_->size();
}


void ControlCharacter::setSize(LyXFont::FONT_SIZE val)
{
	font_->setSize(val);
}


frnt::FONT_STATE ControlCharacter::getBar() const
{
	if (!font_.get())
		return frnt::IGNORE;

	if (font_->emph() == LyXFont::TOGGLE)
		return frnt::EMPH_TOGGLE;

	if (font_->underbar() == LyXFont::TOGGLE)
		return frnt::UNDERBAR_TOGGLE;

	if (font_->noun() == LyXFont::TOGGLE)
		return frnt::NOUN_TOGGLE;

	if (font_->emph() == LyXFont::IGNORE &&
	    font_->underbar() == LyXFont::IGNORE &&
	    font_->noun() == LyXFont::IGNORE)
		return frnt::IGNORE;

	return frnt::INHERIT;
}


void ControlCharacter::setBar(frnt::FONT_STATE val)
{
	switch (val) {
	case frnt::IGNORE:
		font_->setEmph(LyXFont::IGNORE);
		font_->setUnderbar(LyXFont::IGNORE);
		font_->setNoun(LyXFont::IGNORE);
		break;

	case frnt::EMPH_TOGGLE:
		font_->setEmph(LyXFont::TOGGLE);
		break;

	case frnt::UNDERBAR_TOGGLE:
		font_->setUnderbar(LyXFont::TOGGLE);
		break;

	case frnt::NOUN_TOGGLE:
		font_->setNoun(LyXFont::TOGGLE);
		break;

	case frnt::INHERIT:
		font_->setEmph(LyXFont::INHERIT);
		font_->setUnderbar(LyXFont::INHERIT);
		font_->setNoun(LyXFont::INHERIT);
		break;
	}
}


LColor::color ControlCharacter::getColor() const
{
	if (!font_.get())
		return LColor::ignore;

	return font_->color();
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


string ControlCharacter::getLanguage() const
{
	if (font_.get() && font_->language())
		return font_->language()->lang();
	return "ignore";
}


void ControlCharacter::setLanguage(string const & val)
{
	if (val == "ignore")
		font_->setLanguage(ignore_language);

	else if (val == "reset")
		font_->setLanguage(buffer()->params.language);

	else
		font_->setLanguage(languages.getLanguage(val));
}


bool ControlCharacter::getToggleAll() const
{
	return toggleall_;
}


void ControlCharacter::setToggleAll(bool t)
{
	toggleall_ = t;
}
