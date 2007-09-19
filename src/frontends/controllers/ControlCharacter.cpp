/**
 * \file ControlCharacter.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Angus Leeming
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "ControlCharacter.h"

#include "Buffer.h"
#include "BufferParams.h"
#include "bufferview_funcs.h"
#include "FuncRequest.h"
#include "Language.h"
#include "Color.h"

using lyx::bv_funcs::font2string;
using std::string;

namespace lyx {
namespace frontend {

ControlCharacter::ControlCharacter(Dialog & parent)
	: Controller(parent), font_(0), toggleall_(false), reset_lang_(false)
{}


ControlCharacter::~ControlCharacter()
{
	delete font_;
}


bool ControlCharacter::initialiseParams(string const &)
{
	// Do this the first time only.
	if (!font_)
		font_ = new Font(Font::ALL_IGNORE);

	// so that the user can press Ok
	if (getFamily()   != Font::IGNORE_FAMILY ||
	    getSeries()   != Font::IGNORE_SERIES ||
	    getShape()    != Font::IGNORE_SHAPE  ||
	    getSize()     != Font::IGNORE_SIZE ||
	    getBar()      != IGNORE ||
	    getColor()    != Color::ignore ||
	    font_->language() != ignore_language)
		dialog().setButtonsValid(true);

	return true;
}


void ControlCharacter::dispatchParams()
{
	// Nothing to dispatch. (Can be called from the Toolbar.)
	if (!font_)
		return;

	string data;
	if (font2string(*font_, toggleall_, data))
		dispatch(FuncRequest(getLfun(), data));
}


Font::FONT_FAMILY ControlCharacter::getFamily() const
{
	if (!font_)
		return Font::IGNORE_FAMILY;
	return font_->family();
}


void ControlCharacter::setFamily(Font::FONT_FAMILY val)
{
	font_->setFamily(val);
}


Font::FONT_SERIES ControlCharacter::getSeries() const
{
	if (!font_)
		return Font::IGNORE_SERIES;
	return font_->series();
}


void ControlCharacter::setSeries(Font::FONT_SERIES val)
{
	font_->setSeries(val);
}


Font::FONT_SHAPE ControlCharacter::getShape() const
{
	if (!font_)
		return Font::IGNORE_SHAPE;
	return font_->shape();
}


void ControlCharacter::setShape(Font::FONT_SHAPE val)
{
	font_->setShape(val);
}


Font::FONT_SIZE ControlCharacter::getSize() const
{
	if (!font_)
		return Font::IGNORE_SIZE;
	return font_->size();
}


void ControlCharacter::setSize(Font::FONT_SIZE val)
{
	font_->setSize(val);
}


FONT_STATE ControlCharacter::getBar() const
{
	if (!font_)
		return IGNORE;

	if (font_->emph() == Font::TOGGLE)
		return EMPH_TOGGLE;

	if (font_->underbar() == Font::TOGGLE)
		return UNDERBAR_TOGGLE;

	if (font_->noun() == Font::TOGGLE)
		return NOUN_TOGGLE;

	if (font_->emph() == Font::IGNORE &&
	    font_->underbar() == Font::IGNORE &&
	    font_->noun() == Font::IGNORE)
		return IGNORE;

	return INHERIT;
}


void ControlCharacter::setBar(FONT_STATE val)
{
	switch (val) {
	case IGNORE:
		font_->setEmph(Font::IGNORE);
		font_->setUnderbar(Font::IGNORE);
		font_->setNoun(Font::IGNORE);
		break;

	case EMPH_TOGGLE:
		font_->setEmph(Font::TOGGLE);
		break;

	case UNDERBAR_TOGGLE:
		font_->setUnderbar(Font::TOGGLE);
		break;

	case NOUN_TOGGLE:
		font_->setNoun(Font::TOGGLE);
		break;

	case INHERIT:
		font_->setEmph(Font::INHERIT);
		font_->setUnderbar(Font::INHERIT);
		font_->setNoun(Font::INHERIT);
		break;
	}
}


Color_color ControlCharacter::getColor() const
{
	if (!font_)
		return Color::ignore;

	return font_->color();
}


void ControlCharacter::setColor(Color_color val)
{
	switch (val) {
	case Color::ignore:
	case Color::none:
	case Color::black:
	case Color::white:
	case Color::red:
	case Color::green:
	case Color::blue:
	case Color::cyan:
	case Color::magenta:
	case Color::yellow:
	case Color::inherit:
		font_->setColor(val);
		break;
	default:
		break;
	}
}


string ControlCharacter::getLanguage() const
{
	if (reset_lang_)
		return "reset";
	if (font_ && font_->language())
		return font_->language()->lang();
	return "ignore";
}


void ControlCharacter::setLanguage(string const & val)
{
	if (val == "ignore")
		font_->setLanguage(ignore_language);
	else if (val == "reset") {
		reset_lang_ = true;
		// Ignored in getLanguage, but needed for dispatchParams
		font_->setLanguage(buffer().params().language);
	} else {
		font_->setLanguage(languages.getLanguage(val));
	}
}


bool ControlCharacter::getToggleAll() const
{
	return toggleall_;
}


void ControlCharacter::setToggleAll(bool t)
{
	toggleall_ = t;
}

} // namespace frontend
} // namespace lyx
