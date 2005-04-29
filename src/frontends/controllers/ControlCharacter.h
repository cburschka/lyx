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
#include "character.h"


class LColor_color;

namespace lyx {
namespace frontend {

class ControlCharacter : public Dialog::Controller {
public:
	///
	ControlCharacter(Dialog &);
	///
	virtual bool initialiseParams(std::string const & data);
	///
	virtual void clearParams();
	///
	virtual void dispatchParams();
	///
	virtual bool isBufferDependent() const { return true; }
	///
	virtual kb_action getLfun() const { return LFUN_FREEFONT_UPDATE; }

	///
	void setFamily(LyXFont::FONT_FAMILY);
	///
	void setSeries(LyXFont::FONT_SERIES);
	///
	void setShape(LyXFont::FONT_SHAPE);
	///
	void setSize(LyXFont::FONT_SIZE);
	///
	void setBar(FONT_STATE);
	///
	void setColor(LColor_color);
	///
	void setLanguage(std::string const &);
	///
	void setToggleAll(bool);

	///
	LyXFont::FONT_FAMILY getFamily() const;
	///
	LyXFont::FONT_SERIES getSeries() const;
	///
	LyXFont::FONT_SHAPE getShape() const;
	///
	LyXFont::FONT_SIZE getSize() const;
	///
	FONT_STATE getBar() const;
	///
	LColor_color getColor() const;
	///
	std::string getLanguage() const;
	///
	bool getToggleAll() const;
private:
	///
	boost::scoped_ptr<LyXFont> font_;
	///
	bool toggleall_;
};

} // namespace frontend
} // namespace lyx

#endif // CONTROLCHARACTER_H
