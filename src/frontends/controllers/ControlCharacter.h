/**
 * \file ControlCharacter.h
 * Copyright 2001 The LyX Team.
 * See the file COPYING.
 *
 * \author Angus Leeming, a.leeming@.ac.uk
 */

#ifndef CONTROLCHARACTER_H
#define CONTROLCHARACTER_H

#ifdef __GNUG__
#pragma interface
#endif

#include "ControlDialogs.h"
#include "character.h"

/** A controller for Character dialogs.
 */
class ControlCharacter : public ControlDialog<ControlConnectBD>
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

private:
	/// Get changed parameters and Dispatch them to the kernel.
	virtual void apply();
	/// set the params before show or update.
	virtual void setParams();
	/// clean-up on hide.
	virtual void clearParams();

	///
	LyXFont * font_;
	///
	bool toggleall_;
};

#endif // CONTROLCHARACTER_H
