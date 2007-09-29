// -*- C++ -*-
/**
 * \file ControlCharacter.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Angus Leeming
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef CONTROLCHARACTER_H
#define CONTROLCHARACTER_H

#include "Dialog.h"
#include "Font.h"

namespace lyx {

class Color_color;

namespace frontend {

enum FONT_STATE {
	///
	IGNORE,
	///
	EMPH_TOGGLE,
	///
	UNDERBAR_TOGGLE,
	///
	NOUN_TOGGLE,
	///
	INHERIT
};

class ControlCharacter : public Controller
{
public:
	///
	ControlCharacter(Dialog &);
	///
	virtual bool initialiseParams(std::string const & data);
	///
	virtual void clearParams() {}
	///
	virtual void dispatchParams();
	///
	virtual bool isBufferDependent() const { return true; }
	///
	virtual kb_action getLfun() const { return LFUN_FONT_FREE_UPDATE; }

	///
	void setFamily(Font::FONT_FAMILY);
	///
	void setSeries(Font::FONT_SERIES);
	///
	void setShape(Font::FONT_SHAPE);
	///
	void setSize(Font::FONT_SIZE);
	///
	void setBar(FONT_STATE);
	///
	void setColor(Color_color);
	///
	void setLanguage(std::string const &);
	///
	void setToggleAll(bool);

	///
	Font::FONT_FAMILY getFamily() const;
	///
	Font::FONT_SERIES getSeries() const;
	///
	Font::FONT_SHAPE getShape() const;
	///
	Font::FONT_SIZE getSize() const;
	///
	FONT_STATE getBar() const;
	///
	Color_color getColor() const;
	///
	std::string getLanguage() const;
	///
	bool getToggleAll() const;
private:
	///
	Font font_;
	///
	bool toggleall_;
	/// If true the language should be reset.
	/// If false the language of font_ is used.
	bool reset_lang_;
};

} // namespace frontend
} // namespace lyx

#endif // CONTROLCHARACTER_H
