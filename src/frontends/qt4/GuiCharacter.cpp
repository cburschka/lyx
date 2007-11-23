/**
 * \file GuiCharacter.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Angus Leeming
 * \author Edwin Leuven
 * \author John Levon
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "GuiCharacter.h"

#include "qt_helpers.h"
#include "Font.h"
#include "Buffer.h"
#include "BufferParams.h"
#include "FuncRequest.h"
#include "Language.h"

#include <QCloseEvent>

using std::vector;
using std::string;


namespace lyx {
namespace frontend {

static vector<ShapePair> const getShapeData()
{
	vector<ShapePair> shape(6);

	ShapePair pr;

	pr.first = qt_("No change");
	pr.second = IGNORE_SHAPE;
	shape[0] = pr;

	pr.first = qt_("Upright");
	pr.second = UP_SHAPE;
	shape[1] = pr;

	pr.first = qt_("Italic");
	pr.second = ITALIC_SHAPE;
	shape[2] = pr;

	pr.first = qt_("Slanted");
	pr.second = SLANTED_SHAPE;
	shape[3] = pr;

	pr.first = qt_("Small Caps");
	pr.second = SMALLCAPS_SHAPE;
	shape[4] = pr;

	pr.first = qt_("Reset");
	pr.second = INHERIT_SHAPE;
	shape[5] = pr;

	return shape;
}


static vector<SizePair> const getSizeData()
{
	vector<SizePair> size(14);

	SizePair pr;

	pr.first = qt_("No change");
	pr.second = FONT_SIZE_IGNORE;
	size[0] = pr;

	pr.first = qt_("Tiny");
	pr.second = FONT_SIZE_TINY;
	size[1] = pr;

	pr.first = qt_("Smallest");
	pr.second = FONT_SIZE_SCRIPT;
	size[2] = pr;

	pr.first = qt_("Smaller");
	pr.second = FONT_SIZE_FOOTNOTE;
	size[3] = pr;

	pr.first = qt_("Small");
	pr.second = FONT_SIZE_SMALL;
	size[4] = pr;

	pr.first = qt_("Normal");
	pr.second = FONT_SIZE_NORMAL;
	size[5] = pr;

	pr.first = qt_("Large");
	pr.second = FONT_SIZE_LARGE;
	size[6] = pr;

	pr.first = qt_("Larger");
	pr.second = FONT_SIZE_LARGER;
	size[7] = pr;

	pr.first = qt_("Largest");
	pr.second = FONT_SIZE_LARGEST;
	size[8] = pr;

	pr.first = qt_("Huge");
	pr.second = FONT_SIZE_HUGE;
	size[9] = pr;

	pr.first = qt_("Huger");
	pr.second = FONT_SIZE_HUGER;
	size[10] = pr;

	pr.first = qt_("Increase");
	pr.second = FONT_SIZE_INCREASE;
	size[11] = pr;

	pr.first = qt_("Decrease");
	pr.second = FONT_SIZE_DECREASE;
	size[12] = pr;

	pr.first = qt_("Reset");
	pr.second = FONT_SIZE_INHERIT;
	size[13] = pr;

	return size;
}


static vector<BarPair> const getBarData()
{
	vector<BarPair> bar(5);

	BarPair pr;

	pr.first = qt_("No change");
	pr.second = IGNORE;
	bar[0] = pr;

	pr.first = qt_("Emph");
	pr.second = EMPH_TOGGLE;
	bar[1] = pr;

	pr.first = qt_("Underbar");
	pr.second = UNDERBAR_TOGGLE;
	bar[2] = pr;

	pr.first = qt_("Noun");
	pr.second = NOUN_TOGGLE;
	bar[3] = pr;

	pr.first = qt_("Reset");
	pr.second = INHERIT;
	bar[4] = pr;

	return bar;
}


static vector<ColorPair> const getColorData()
{
	vector<ColorPair> color(11);

	ColorPair pr;

	pr.first = qt_("No change");
	pr.second = Color_ignore;
	color[0] = pr;

	pr.first = qt_("No color");
	pr.second = Color_none;
	color[1] = pr;

	pr.first = qt_("Black");
	pr.second = Color_black;
	color[2] = pr;

	pr.first = qt_("White");
	pr.second = Color_white;
	color[3] = pr;

	pr.first = qt_("Red");
	pr.second = Color_red;
	color[4] = pr;

	pr.first = qt_("Green");
	pr.second = Color_green;
	color[5] = pr;

	pr.first = qt_("Blue");
	pr.second = Color_blue;
	color[6] = pr;

	pr.first = qt_("Cyan");
	pr.second = Color_cyan;
	color[7] = pr;

	pr.first = qt_("Magenta");
	pr.second = Color_magenta;
	color[8] = pr;

	pr.first = qt_("Yellow");
	pr.second = Color_yellow;
	color[9] = pr;

	pr.first = qt_("Reset");
	pr.second = Color_inherit;
	color[10] = pr;

	return color;
}


static vector<SeriesPair> const getSeriesData()
{
	vector<SeriesPair> series(4);

	SeriesPair pr;

	pr.first = qt_("No change");
	pr.second = IGNORE_SERIES;
	series[0] = pr;

	pr.first = qt_("Medium");
	pr.second = MEDIUM_SERIES;
	series[1] = pr;

	pr.first = qt_("Bold");
	pr.second = BOLD_SERIES;
	series[2] = pr;

	pr.first = qt_("Reset");
	pr.second = INHERIT_SERIES;
	series[3] = pr;

	return series;
}


static vector<FamilyPair> const getFamilyData()
{
	vector<FamilyPair> family(5);

	FamilyPair pr;

	pr.first = qt_("No change");
	pr.second = IGNORE_FAMILY;
	family[0] = pr;

	pr.first = qt_("Roman");
	pr.second = ROMAN_FAMILY;
	family[1] = pr;

	pr.first = qt_("Sans Serif");
	pr.second = SANS_FAMILY;
	family[2] = pr;

	pr.first = qt_("Typewriter");
	pr.second = TYPEWRITER_FAMILY;
	family[3] = pr;

	pr.first = qt_("Reset");
	pr.second = INHERIT_FAMILY;
	family[4] = pr;

	return family;
}


GuiCharacter::GuiCharacter(GuiView & lv)
	: GuiDialog(lv, "character"), font_(ignore_font),
	  toggleall_(false), reset_lang_(false)
{
	setupUi(this);
	setViewTitle(_("Text Style"));

	connect(okPB, SIGNAL(clicked()), this, SLOT(slotOK()));
	connect(applyPB, SIGNAL(clicked()), this, SLOT(slotApply()));
	connect(closePB, SIGNAL(clicked()), this, SLOT(slotClose()));

	connect(miscCO, SIGNAL(activated(int)), this, SLOT(change_adaptor()));
	connect(sizeCO, SIGNAL(activated(int)), this, SLOT(change_adaptor()));
	connect(familyCO, SIGNAL(activated(int)), this, SLOT(change_adaptor()));
	connect(seriesCO, SIGNAL(activated(int)), this, SLOT(change_adaptor()));
	connect(shapeCO, SIGNAL(activated(int)), this, SLOT(change_adaptor()));
	connect(colorCO, SIGNAL(activated(int)), this, SLOT(change_adaptor()));
	connect(langCO, SIGNAL(activated(int)), this, SLOT(change_adaptor()));
	connect(toggleallCB, SIGNAL(clicked()), this, SLOT(change_adaptor()));

	family = getFamilyData();
	series = getSeriesData();
	shape  = getShapeData();
	size   = getSizeData();
	bar    = getBarData();
	color  = getColorData();
	language = getLanguageData(true);

	for (vector<FamilyPair>::const_iterator cit = family.begin();
		cit != family.end(); ++cit) {
		familyCO->addItem(cit->first);
	}

	for (vector<SeriesPair>::const_iterator cit = series.begin();
		cit != series.end(); ++cit) {
		seriesCO->addItem(cit->first);
	}
	for (vector<ShapePair>::const_iterator cit = shape.begin();
		cit != shape.end(); ++cit) {
		shapeCO->addItem(cit->first);
	}
	for (vector<SizePair>::const_iterator cit = size.begin();
		cit != size.end(); ++cit) {
		sizeCO->addItem(cit->first);
	}
	for (vector<BarPair>::const_iterator cit = bar.begin();
		cit != bar.end(); ++cit) {
		miscCO->addItem(cit->first);
	}
	for (vector<ColorPair>::const_iterator cit = color.begin();
		cit != color.end(); ++cit) {
		colorCO->addItem(cit->first);
	}
	for (vector<LanguagePair>::const_iterator cit = language.begin();
		cit != language.end(); ++cit) {
		langCO->addItem(toqstr(cit->first));
	}

	bc().setPolicy(ButtonPolicy::OkApplyCancelReadOnlyPolicy);
	bc().setOK(okPB);
	bc().setApply(applyPB);
	bc().setCancel(closePB);
	bc().addReadOnly(familyCO);
	bc().addReadOnly(seriesCO);
	bc().addReadOnly(sizeCO);
	bc().addReadOnly(shapeCO);
	bc().addReadOnly(miscCO);
	bc().addReadOnly(langCO);
	bc().addReadOnly(colorCO);
	bc().addReadOnly(toggleallCB);
	bc().addReadOnly(autoapplyCB);

// FIXME: hack to work around resizing bug in Qt >= 4.2
// bug verified with Qt 4.2.{0-3} (JSpitzm)
#if QT_VERSION >= 0x040200
	// qt resizes the comboboxes only after show(), so ...
	QDialog::show();
#endif
}


void GuiCharacter::change_adaptor()
{
	changed();

	if (!autoapplyCB->isChecked())
		return;

	// to be really good here, we should set the combos to the values of
	// the current text, and make it appear as "no change" if the values
	// stay the same between applys. Might be difficult though wrt to a
	// moved cursor - jbl
	slotApply();
	familyCO->setCurrentIndex(0);
	seriesCO->setCurrentIndex(0);
	sizeCO->setCurrentIndex(0);
	shapeCO->setCurrentIndex(0);
	miscCO->setCurrentIndex(0);
	langCO->setCurrentIndex(0);
	colorCO->setCurrentIndex(0);
}


void GuiCharacter::closeEvent(QCloseEvent * e)
{
	slotClose();
	GuiDialog::closeEvent(e);
}


template<class A, class B>
static int findPos2nd(vector<std::pair<A, B> > const & vec, B const & val)
{
	typedef typename vector<std::pair<A, B> >::const_iterator
		const_iterator;

	for (const_iterator cit = vec.begin(); cit != vec.end(); ++cit)
		if (cit->second == val)
			return int(cit - vec.begin());

	return 0;
}


void GuiCharacter::updateContents()
{
	familyCO->setCurrentIndex(findPos2nd(family, getFamily()));
	seriesCO->setCurrentIndex(findPos2nd(series, getSeries()));
	shapeCO->setCurrentIndex(findPos2nd(shape, getShape()));
	sizeCO->setCurrentIndex(findPos2nd(size, getSize()));
	miscCO->setCurrentIndex(findPos2nd(bar, getBar()));
	colorCO->setCurrentIndex(findPos2nd(color, getColor()));
	langCO->setCurrentIndex(findPos2nd(language, getLanguage()));

	toggleallCB->setChecked(toggleall_);
}


void GuiCharacter::applyView()
{
	setFamily(family[familyCO->currentIndex()].second);
	setSeries(series[seriesCO->currentIndex()].second);
	setShape(shape[shapeCO->currentIndex()].second);
	setSize(size[sizeCO->currentIndex()].second);
	setBar(bar[miscCO->currentIndex()].second);
	setColor(color[colorCO->currentIndex()].second);
	setLanguage(language[langCO->currentIndex()].second);

	toggleall_ = toggleallCB->isChecked();
}


bool GuiCharacter::initialiseParams(string const &)
{
	// so that the user can press Ok
	if (getFamily()    != IGNORE_FAMILY
	    || getSeries() != IGNORE_SERIES
	    || getShape()  != IGNORE_SHAPE
	    || getSize()   != FONT_SIZE_IGNORE
	    || getBar()    != IGNORE
	    || getColor()  != Color_ignore
	    || font_.language() != ignore_language)
		setButtonsValid(true);

	return true;
}


void GuiCharacter::dispatchParams()
{
	dispatch(FuncRequest(getLfun(), font_.toString(toggleall_)));
}


FontFamily GuiCharacter::getFamily() const
{
	return font_.fontInfo().family();
}


void GuiCharacter::setFamily(FontFamily val)
{
	font_.fontInfo().setFamily(val);
}


FontSeries GuiCharacter::getSeries() const
{
	return font_.fontInfo().series();
}


void GuiCharacter::setSeries(FontSeries val)
{
	font_.fontInfo().setSeries(val);
}


FontShape GuiCharacter::getShape() const
{
	return font_.fontInfo().shape();
}


void GuiCharacter::setShape(FontShape val)
{
	font_.fontInfo().setShape(val);
}


FontSize GuiCharacter::getSize() const
{
	return font_.fontInfo().size();
}


void GuiCharacter::setSize(FontSize val)
{
	font_.fontInfo().setSize(val);
}


FontState GuiCharacter::getBar() const
{
	if (font_.fontInfo().emph() == FONT_TOGGLE)
		return EMPH_TOGGLE;

	if (font_.fontInfo().underbar() == FONT_TOGGLE)
		return UNDERBAR_TOGGLE;

	if (font_.fontInfo().noun() == FONT_TOGGLE)
		return NOUN_TOGGLE;

	if (font_.fontInfo().emph() == FONT_IGNORE
	    && font_.fontInfo().underbar() == FONT_IGNORE
	    && font_.fontInfo().noun() == FONT_IGNORE)
		return IGNORE;

	return INHERIT;
}


void GuiCharacter::setBar(FontState val)
{
	switch (val) {
	case IGNORE:
		font_.fontInfo().setEmph(FONT_IGNORE);
		font_.fontInfo().setUnderbar(FONT_IGNORE);
		font_.fontInfo().setNoun(FONT_IGNORE);
		break;

	case EMPH_TOGGLE:
		font_.fontInfo().setEmph(FONT_TOGGLE);
		break;

	case UNDERBAR_TOGGLE:
		font_.fontInfo().setUnderbar(FONT_TOGGLE);
		break;

	case NOUN_TOGGLE:
		font_.fontInfo().setNoun(FONT_TOGGLE);
		break;

	case INHERIT:
		font_.fontInfo().setEmph(FONT_INHERIT);
		font_.fontInfo().setUnderbar(FONT_INHERIT);
		font_.fontInfo().setNoun(FONT_INHERIT);
		break;
	}
}


ColorCode GuiCharacter::getColor() const
{
	return font_.fontInfo().color();
}


void GuiCharacter::setColor(ColorCode val)
{
	switch (val) {
	case Color_ignore:
	case Color_none:
	case Color_black:
	case Color_white:
	case Color_red:
	case Color_green:
	case Color_blue:
	case Color_cyan:
	case Color_magenta:
	case Color_yellow:
	case Color_inherit:
		font_.fontInfo().setColor(val);
		break;
	default:
		break;
	}
}


string GuiCharacter::getLanguage() const
{
	if (reset_lang_)
		return "reset";
	if (font_.language())
		return font_.language()->lang();
	return "ignore";
}


void GuiCharacter::setLanguage(string const & val)
{
	if (val == "ignore")
		font_.setLanguage(ignore_language);
	else if (val == "reset") {
		reset_lang_ = true;
		// Ignored in getLanguage, but needed for dispatchParams
		font_.setLanguage(buffer().params().language);
	} else {
		font_.setLanguage(languages.getLanguage(val));
	}
}


Dialog * createGuiCharacter(GuiView & lv) { return new GuiCharacter(lv); }


} // namespace frontend
} // namespace lyx

#include "GuiCharacter_moc.cpp"
