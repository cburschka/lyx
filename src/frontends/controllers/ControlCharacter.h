// -*- C++ -*-
/**
 * \file ControlCharacter.h
 * Copyright 2001 The LyX Team.
 * See the file COPYING.
 *
 * \author Angus Leeming, a.leeming@.ac.uk
 */

#ifndef CONTROLCHARACTER_H
#define CONTROLCHARACTER_H

#include <boost/smart_ptr.hpp>

#ifdef __GNUG__
#pragma interface
#endif

#include "ControlDialog_impl.h"
#include "character.h"

/** A controller for Character dialogs.
 */
class ControlCharacter : public ControlDialogBD
{
public:
	///
	ControlCharacter(LyXView &, Dialogs &);

	///
	void setFamily(LyXFont::FONT_FAMILY);
	///
	void setSeries(LyXFont::FONT_SERIES);
	///
	void setShape(LyXFont::FONT_SHAPE);
	///
	void setSize(LyXFont::FONT_SIZE);
	///
	void setBar(character::FONT_STATE);
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
	character::FONT_STATE getBar() const;
	///
	LColor::color getColor() const;
	///
	string getLanguage() const;
	///
	bool getToggleAll() const;

private:
	/// Get changed parameters and Dispatch them to the kernel.
	virtual void apply();
	/// set the params before show or update.
	virtual void setParams();

	///
	boost::scoped_ptr<LyXFont> font_;
	///
	bool toggleall_;
};

#endif // CONTROLCHARACTER_H
