// -*- C++ -*-
/* This file is part of
 * ======================================================
 *
 *           LyX, The Document Processor
 *
 *           Copyright 2001 The LyX Team.
 *
 * ======================================================
 *
 * \file character.h
 * \author Angus Leeming <a.leeming@ic.ac.uk>
 */

#ifndef CHARACTERHELPERS_H
#define CHARACTERHELPERS_H

#ifdef __GNUG__
#pragma interface
#endif

#include "lyxfont.h"
#include "LColor.h"

/** Functions of use to the character GUI controller and view */
namespace character {
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
	std::vector<FamilyPair> const getFamilyData();
	///
	std::vector<SeriesPair> const getSeriesData();
	///
	std::vector<ShapePair>  const getShapeData();
	///
	std::vector<SizePair>   const getSizeData();
	///
	std::vector<BarPair>    const getBarData();
	///
	std::vector<ColorPair>  const getColorData();
	///
	std::vector<string> const getLanguageData();

} // namespace character

#endif // CHARACTERHELPERS
