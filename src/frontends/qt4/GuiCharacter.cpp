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


using namespace std;

namespace lyx {
namespace frontend {

static QList<ShapePair> shapeData()
{
	QList<ShapePair> shapes;
	shapes << ShapePair(qt_("No change"), IGNORE_SHAPE);
	shapes << ShapePair(qt_("Upright"), UP_SHAPE);
	shapes << ShapePair(qt_("Italic"), ITALIC_SHAPE);
	shapes << ShapePair(qt_("Slanted"), SLANTED_SHAPE);
	shapes << ShapePair(qt_("Small Caps"), SMALLCAPS_SHAPE);
	shapes << ShapePair(qt_("Reset"), INHERIT_SHAPE);
	return shapes;
}


static QList<SizePair> sizeData()
{
	QList<SizePair> sizes;
	sizes << SizePair(qt_("No change"), FONT_SIZE_IGNORE);
	sizes << SizePair(qt_("Tiny"), FONT_SIZE_TINY);
	sizes << SizePair(qt_("Smallest"), FONT_SIZE_SCRIPT);
	sizes << SizePair(qt_("Smaller"), FONT_SIZE_FOOTNOTE);
	sizes << SizePair(qt_("Small"), FONT_SIZE_SMALL);
	sizes << SizePair(qt_("Normal"), FONT_SIZE_NORMAL);
	sizes << SizePair(qt_("Large"), FONT_SIZE_LARGE);
	sizes << SizePair(qt_("Larger"), FONT_SIZE_LARGER);
	sizes << SizePair(qt_("Largest"), FONT_SIZE_LARGEST);
	sizes << SizePair(qt_("Huge"), FONT_SIZE_HUGE);
	sizes << SizePair(qt_("Huger"), FONT_SIZE_HUGER);
	sizes << SizePair(qt_("Increase"), FONT_SIZE_INCREASE);
	sizes << SizePair(qt_("Decrease"), FONT_SIZE_DECREASE);
	sizes << SizePair(qt_("Reset"), FONT_SIZE_INHERIT);
	return sizes;
}


static QList<BarPair> barData()
{
	QList<BarPair> bars;
	bars << BarPair(qt_("No change"), IGNORE);
	bars << BarPair(qt_("Emph"),      EMPH_TOGGLE);
	bars << BarPair(qt_("Underbar"),  UNDERBAR_TOGGLE);
	bars << BarPair(qt_("Noun"),      NOUN_TOGGLE);
	bars << BarPair(qt_("Reset"),     INHERIT);
	return bars;
}


static QList<ColorPair> colorData()
{
	QList<ColorPair> colors;
	colors << ColorPair(qt_("No change"), Color_ignore);
	colors << ColorPair(qt_("No color"), Color_none);
	colors << ColorPair(qt_("Black"), Color_black);
	colors << ColorPair(qt_("White"), Color_white);
	colors << ColorPair(qt_("Red"), Color_red);
	colors << ColorPair(qt_("Green"), Color_green);
	colors << ColorPair(qt_("Blue"), Color_blue);
	colors << ColorPair(qt_("Cyan"), Color_cyan);
	colors << ColorPair(qt_("Magenta"), Color_magenta);
	colors << ColorPair(qt_("Yellow"), Color_yellow);
	colors << ColorPair(qt_("Reset"), Color_inherit);
	return colors;
}


static QList<SeriesPair> seriesData()
{
	QList<SeriesPair> series;
	series << SeriesPair(qt_("No change"), IGNORE_SERIES);
	series << SeriesPair(qt_("Medium"),    MEDIUM_SERIES);
	series << SeriesPair(qt_("Bold"),      BOLD_SERIES);
	series << SeriesPair(qt_("Reset"),     INHERIT_SERIES);
	return series;
}


static QList<FamilyPair> familyData()
{
	QList<FamilyPair> families;
	families << FamilyPair(qt_("No change"),  IGNORE_FAMILY);
	families << FamilyPair(qt_("Roman"),      ROMAN_FAMILY);
	families << FamilyPair(qt_("Sans Serif"), SANS_FAMILY);
	families << FamilyPair(qt_("Typewriter"), TYPEWRITER_FAMILY);
	families << FamilyPair(qt_("Reset"),      INHERIT_FAMILY);
	return families;
}


GuiCharacter::GuiCharacter(GuiView & lv)
	: GuiDialog(lv, "character", qt_("Text Style")), font_(ignore_font),
	  toggleall_(false), reset_lang_(false)
{
	setupUi(this);

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

#ifdef Q_WS_MACX
	// On Mac it's common to have tool windows which are always in the
	// foreground and are hidden when the main window is not focused.
	setWindowFlags(Qt::Tool);
	autoapplyCB->setChecked(true);
#endif

	family = familyData();
	series = seriesData();
	shape  = shapeData();
	size   = sizeData();
	bar    = barData();
	color  = colorData();
	language = languageData(true);

	for (QList<FamilyPair>::const_iterator cit = family.begin();
		cit != family.end(); ++cit) {
		familyCO->addItem(cit->first);
	}

	for (QList<SeriesPair>::const_iterator cit = series.begin();
		cit != series.end(); ++cit) {
		seriesCO->addItem(cit->first);
	}
	for (QList<ShapePair>::const_iterator cit = shape.begin();
		cit != shape.end(); ++cit) {
		shapeCO->addItem(cit->first);
	}
	for (QList<SizePair>::const_iterator cit = size.begin();
		cit != size.end(); ++cit) {
		sizeCO->addItem(cit->first);
	}
	for (QList<BarPair>::const_iterator cit = bar.begin();
		cit != bar.end(); ++cit) {
		miscCO->addItem(cit->first);
	}
	for (QList<ColorPair>::const_iterator cit = color.begin();
		cit != color.end(); ++cit) {
		colorCO->addItem(cit->first);
	}
	for (QList<LanguagePair>::const_iterator cit = language.begin();
		cit != language.end(); ++cit) {
		langCO->addItem(cit->first);
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


template<class P, class B>
static int findPos2nd(QList<P> const & vec, B const & val)
{
	for (int i = 0; i != vec.size(); ++i)
		if (vec[i].second == val)
			return i;
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


QString GuiCharacter::getLanguage() const
{
	if (reset_lang_)
		return "reset";
	if (font_.language())
		return toqstr(font_.language()->lang());
	return "ignore";
}


void GuiCharacter::setLanguage(QString const & val)
{
	if (val == "ignore")
		font_.setLanguage(ignore_language);
	else if (val == "reset") {
		reset_lang_ = true;
		// Ignored in getLanguage, but needed for dispatchParams
		font_.setLanguage(buffer().params().language);
	} else {
		font_.setLanguage(languages.getLanguage(fromqstr(val)));
	}
}


Dialog * createGuiCharacter(GuiView & lv) { return new GuiCharacter(lv); }


} // namespace frontend
} // namespace lyx

#include "GuiCharacter_moc.cpp"
