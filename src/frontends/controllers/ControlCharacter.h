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

#include "ControlConnections.h"
#include "lyxfont.h"
#include "LColor.h"

/** A controller for Character dialogs.
 */
class ControlCharacter : public ControlConnectBD
{
public:
	///
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
		LATEX_TOGGLE,
		///
		INHERIT
	};
	
	///
	typedef std::pair<string, LyXFont::FONT_FAMILY> FamilyPair;
	///
	typedef std::pair<string, LyXFont::FONT_SERIES> SeriesPair;
	///
	typedef std::pair<string, LyXFont::FONT_SHAPE>  ShapePair;
	///
	typedef std::pair<string, LyXFont::FONT_SIZE>   SizePair;
	///
	typedef std::pair<string, FONT_STATE> BarPair;
	///
	typedef std::pair<string, LColor::color> ColorPair;

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
	void setBar(FONT_STATE);
	///
	void setColor(LColor::color);
	///
	void setLanguage(string const &);
	///
	void setToggleAll(bool);

protected:
	/// Get changed parameters and Dispatch them to the kernel.
	virtual void apply();
	/// Show the dialog.
	virtual void show();
	/// Update the dialog.
	virtual void update();
	/// Hide the dialog.
	virtual void hide();

private:
	LyXFont * font_;
	bool toggleall_;
};

/// Helper functions
std::vector<ControlCharacter::FamilyPair> const getFamilyData();
///
std::vector<ControlCharacter::SeriesPair> const getSeriesData();
///
std::vector<ControlCharacter::ShapePair>  const getShapeData();
///
std::vector<ControlCharacter::SizePair>   const getSizeData();
///
std::vector<ControlCharacter::BarPair>    const getBarData();
///
std::vector<ControlCharacter::ColorPair>  const getColorData();
///
std::vector<string> const getLanguageData();

/** This class instantiates and makes available the GUI-specific
    ButtonController and View.
 */
template <class GUIview, class GUIbc>
class GUICharacter : public ControlCharacter {
public:
	///
	GUICharacter(LyXView &, Dialogs &);
	///
	virtual ButtonControllerBase & bc() { return bc_; }
	///
	virtual ViewBase & view() { return view_; }

private:
	///
	ButtonController<NoRepeatedApplyReadOnlyPolicy, GUIbc> bc_;
	///
	GUIview view_;
};

template <class GUIview, class GUIbc>
GUICharacter<GUIview, GUIbc>::GUICharacter(LyXView & lv, Dialogs & d)
	: ControlCharacter(lv, d),
	  view_(*this)
{}

#endif // CONTROLCHARACTER_H
