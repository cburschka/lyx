// -*- C++ -*-
/**
 * \file character.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Angus Leeming
 *
 * Full author contact details are available in file CREDITS
 */

#ifndef CHARACTERHELPERS_H
#define CHARACTERHELPERS_H

#ifdef __GNUG__
#pragma interface
#endif

#include "lyxfont.h"
#include "LColor.h"

#include <utility>
#include <vector>

/** Functions of use to the character GUI controller and view */
namespace frnt {
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
	std::vector<FamilyPair>   const getFamilyData();
	///
	std::vector<SeriesPair>   const getSeriesData();
	///
	std::vector<ShapePair>    const getShapeData();
	///
	std::vector<SizePair>     const getSizeData();
	///
	std::vector<BarPair>      const getBarData();
	///
	std::vector<ColorPair>    const getColorData();

} // namespace frnt

#endif // CHARACTERHELPERS
