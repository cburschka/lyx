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

#include <config.h>
#include <vector>

#ifdef __GNUG__
#pragma implementation
#endif

#include "gettext.h"
#include "character.h"
#include "language.h"

using std::vector;

namespace character {

vector<FamilyPair> const getFamilyData()
{
	vector<FamilyPair> family(5);

	FamilyPair pr;
	pr.first = _("No change");  pr.second = LyXFont::IGNORE_FAMILY;
	family[0] = pr;
	pr.first = _("Roman");      pr.second = LyXFont::ROMAN_FAMILY;
	family[1] = pr;
	pr.first = _("Sans Serif"); pr.second = LyXFont::SANS_FAMILY;
	family[2] = pr;
	pr.first = _("Typewriter"); pr.second = LyXFont::TYPEWRITER_FAMILY;
	family[3] = pr;
	pr.first = _("Reset");      pr.second = LyXFont::INHERIT_FAMILY;
	family[4] = pr;

	return family;
}


vector<SeriesPair> const getSeriesData()
{
	vector<SeriesPair> series(4);

	SeriesPair pr;
	pr.first = _("No change"); pr.second = LyXFont::IGNORE_SERIES;
	series[0] = pr;
	pr.first = _("Medium");    pr.second = LyXFont::MEDIUM_SERIES;
	series[1] = pr;
	pr.first = _("Bold");      pr.second = LyXFont::BOLD_SERIES;
	series[2] = pr;
	pr.first = _("Reset");     pr.second = LyXFont::INHERIT_SERIES;
	series[3] = pr;

	return series;
}


vector<ShapePair> const getShapeData()
{
	vector<ShapePair> shape(6);

	ShapePair pr;
	pr.first = _("No change");  pr.second = LyXFont::IGNORE_SHAPE;
	shape[0] = pr;
	pr.first = _("Upright");    pr.second = LyXFont::UP_SHAPE;
	shape[1] = pr;
	pr.first = _("Italic");     pr.second = LyXFont::ITALIC_SHAPE;
	shape[2] = pr;
	pr.first = _("Slanted");    pr.second = LyXFont::SLANTED_SHAPE;
	shape[3] = pr;
	pr.first = _("Small Caps"); pr.second = LyXFont::SMALLCAPS_SHAPE;
	shape[4] = pr;
	pr.first = _("Reset");      pr.second = LyXFont::INHERIT_SHAPE;
	shape[5] = pr;

	return shape;
}


vector<SizePair> const getSizeData()
{
	vector<SizePair> size(14);

	SizePair pr;
	pr.first = _("No change"); pr.second = LyXFont::IGNORE_SIZE;
	size[0] = pr;
	pr.first = _("Tiny");      pr.second = LyXFont::SIZE_TINY;
	size[1] = pr;
	pr.first = _("Smallest");  pr.second = LyXFont::SIZE_SCRIPT;
	size[2] = pr;
	pr.first = _("Smaller");   pr.second = LyXFont::SIZE_FOOTNOTE;
	size[3] = pr;
	pr.first = _("Small");     pr.second = LyXFont::SIZE_SMALL;
	size[4] = pr;
	pr.first = _("Normal");    pr.second = LyXFont::SIZE_NORMAL;
	size[5] = pr;
	pr.first = _("Large");     pr.second = LyXFont::SIZE_LARGE;
	size[6] = pr;
	pr.first = _("Larger");    pr.second = LyXFont::SIZE_LARGER;
	size[7] = pr;
	pr.first = _("Largest");   pr.second = LyXFont::SIZE_LARGEST;
	size[8] = pr;
	pr.first = _("Huge");      pr.second = LyXFont::SIZE_HUGE;
	size[9] = pr;
	pr.first = _("Huger");     pr.second = LyXFont::SIZE_HUGER;
	size[10] = pr;
	pr.first = _("Increase");  pr.second = LyXFont::INCREASE_SIZE;
	size[11] = pr;
	pr.first = _("Decrease");  pr.second = LyXFont::DECREASE_SIZE;
	size[12] = pr;
	pr.first = _("Reset");     pr.second = LyXFont::INHERIT_SIZE;
	size[13] = pr;

	return size;
}


vector<BarPair> const getBarData()
{
	vector<BarPair> bar(5);

	BarPair pr;
	pr.first = _("No change");  pr.second = IGNORE;
	bar[0] = pr;
	pr.first = _("Emph");       pr.second = EMPH_TOGGLE;
	bar[1] = pr;
	pr.first = _("Underbar");   pr.second = UNDERBAR_TOGGLE;
	bar[2] = pr;
	pr.first = _("Noun");       pr.second = NOUN_TOGGLE;
	bar[3] = pr;
	pr.first = _("Reset");      pr.second = INHERIT;
	bar[4] = pr;

	return bar;
}


vector<ColorPair> const getColorData()
{
	vector<ColorPair> color(11);

	ColorPair pr;
	pr.first = _("No change"); pr.second = LColor::ignore;
	color[0] = pr;
	pr.first = _("No color");  pr.second = LColor::none;
	color[1] = pr;
	pr.first = _("Black");     pr.second = LColor::black;
	color[2] = pr;
	pr.first = _("White");     pr.second = LColor::white;
	color[3] = pr;
	pr.first = _("Red");       pr.second = LColor::red;
	color[4] = pr;
	pr.first = _("Green");     pr.second = LColor::green;
	color[5] = pr;
	pr.first = _("Blue");      pr.second = LColor::blue;
	color[6] = pr;
	pr.first = _("Cyan");      pr.second = LColor::cyan;
	color[7] = pr;
	pr.first = _("Magenta");   pr.second = LColor::magenta;
	color[8] = pr;
	pr.first = _("Yellow");    pr.second = LColor::yellow;
	color[9] = pr;
	pr.first = _("Reset");     pr.second = LColor::inherit;
	color[10] = pr;

	return color;
}


vector<string> const getLanguageData()
{
	vector<string> langs(languages.size() + 2);

	langs[0] = _("No change");
	langs[1] = _("Reset");

	vector<string>::size_type i = 1;
	for (Languages::const_iterator cit = languages.begin();
	     cit != languages.end(); ++cit) {
		langs[++i] = cit->second.lang();
	}

	return langs;
}

} // namespace character
