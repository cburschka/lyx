// -*- C++ -*-
/**
 * \file ControlCharacter.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Angus Leeming
 *
 * Full author contact details are available in file CREDITS
 */

#ifndef CONTROLCHARACTER_H
#define CONTROLCHARACTER_H


#include "Dialog.h"
#include "character.h"


class ControlCharacter : public Dialog::Controller {
public:
	///
	ControlCharacter(Dialog &);
	///
	virtual void initialiseParams(string const &);
	///
	virtual void clearParams();
	///
	virtual void dispatchParams();
	///
	virtual bool isBufferDependent() const { return true; }

	///
	void setFamily(LyXFont::FONT_FAMILY);
	///
	void setSeries(LyXFont::FONT_SERIES);
	///
	void setShape(LyXFont::FONT_SHAPE);
	///
	void setSize(LyXFont::FONT_SIZE);
	///
	void setBar(frnt::FONT_STATE);
	///
	void setColor(LColor::color);
	///
	void setLanguage(string const &);
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
	frnt::FONT_STATE getBar() const;
	///
	LColor::color getColor() const;
	///
	string getLanguage() const;
	///
	bool getToggleAll() const;
private:
	///
	boost::scoped_ptr<LyXFont> font_;
	///
	bool toggleall_;
};

#endif // CONTROLCHARACTER_H
