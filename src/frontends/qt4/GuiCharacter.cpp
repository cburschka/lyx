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
#include "BufferView.h"
#include "Cursor.h"
#include "FuncRequest.h"
#include "Language.h"
#include "Paragraph.h"


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

namespace {

template<typename T>
void fillCombo(QComboBox * combo, QList<T> list)
{
	typename QList<T>::const_iterator cit = list.begin();
	for (; cit != list.end(); ++cit)
		combo->addItem(cit->first);
}

}

GuiCharacter::GuiCharacter(GuiView & lv)
	: GuiDialog(lv, "character", qt_("Text Style")), font_(ignore_font, ignore_language),
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

	language = languageData();
	language.prepend(LanguagePair(qt_("Reset"), "reset"));
	language.prepend(LanguagePair(qt_("No change"), "ignore"));

	fillCombo(familyCO, family);
	fillCombo(seriesCO, series);
	fillCombo(sizeCO, size);
	fillCombo(shapeCO, shape);
	fillCombo(miscCO, bar);
	fillCombo(colorCO, color);
	fillCombo(langCO, language);

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
	if (!autoapplyCB->isChecked())
		return;
	if (bufferview()->cursor().selection()) {
		//FIXME: it would be better to check if each font attribute is constant
		// for the selection range.
		font_ = Font(ignore_font, ignore_language);
	} else
		font_ = bufferview()->cursor().current_font;

	paramsToDialog(font_);
}


static FontState getBar(FontInfo const & fi)
{
	if (fi.emph() == FONT_TOGGLE)
		return EMPH_TOGGLE;

	if (fi.underbar() == FONT_TOGGLE)
		return UNDERBAR_TOGGLE;

	if (fi.noun() == FONT_TOGGLE)
		return NOUN_TOGGLE;

	if (fi.emph() == FONT_IGNORE
	    && fi.underbar() == FONT_IGNORE
	    && fi.noun() == FONT_IGNORE)
		return IGNORE;

	return INHERIT;
}


static void setBar(FontInfo & fi, FontState val)
{
	switch (val) {
	case IGNORE:
		fi.setEmph(FONT_IGNORE);
		fi.setUnderbar(FONT_IGNORE);
		fi.setNoun(FONT_IGNORE);
		break;

	case EMPH_TOGGLE:
		fi.setEmph(FONT_TOGGLE);
		break;

	case UNDERBAR_TOGGLE:
		fi.setUnderbar(FONT_TOGGLE);
		break;

	case NOUN_TOGGLE:
		fi.setNoun(FONT_TOGGLE);
		break;

	case INHERIT:
		fi.setEmph(FONT_INHERIT);
		fi.setUnderbar(FONT_INHERIT);
		fi.setNoun(FONT_INHERIT);
		break;
	}
}


void GuiCharacter::paramsToDialog(Font const & font)
{
	FontInfo const & fi = font.fontInfo();
	familyCO->setCurrentIndex(findPos2nd(family, fi.family()));
	seriesCO->setCurrentIndex(findPos2nd(series, fi.series()));
	shapeCO->setCurrentIndex(findPos2nd(shape, fi.shape()));
	sizeCO->setCurrentIndex(findPos2nd(size, fi.size()));
	miscCO->setCurrentIndex(findPos2nd(bar, getBar(fi)));
	colorCO->setCurrentIndex(findPos2nd(color, fi.color()));

	QString const lang = (font.language() == ignore_language)
		? "ignore" : toqstr(font.language()->lang());
	langCO->setCurrentIndex(findPos2nd(language, lang));

	toggleallCB->setChecked(toggleall_);
}


void GuiCharacter::applyView()
{
	FontInfo & fi = font_.fontInfo();
	fi.setFamily(family[familyCO->currentIndex()].second);
	fi.setSeries(series[seriesCO->currentIndex()].second);
	fi.setShape(shape[shapeCO->currentIndex()].second);
	fi.setSize(size[sizeCO->currentIndex()].second);
	setBar(fi, bar[miscCO->currentIndex()].second);
	fi.setColor(color[colorCO->currentIndex()].second);

	font_.setLanguage(languages.getLanguage(
		fromqstr(language[langCO->currentIndex()].second)));

	toggleall_ = toggleallCB->isChecked();
}


bool GuiCharacter::initialiseParams(string const &)
{
	FontInfo & fi = font_.fontInfo();

	// so that the user can press Ok
	if (fi.family()    != IGNORE_FAMILY
	    || fi.series() != IGNORE_SERIES
	    || fi.shape()  != IGNORE_SHAPE
	    || fi.size()   != FONT_SIZE_IGNORE
	    || getBar(fi)  != IGNORE
	    || fi.color()  != Color_ignore
	    || font_.language() != ignore_language)
		setButtonsValid(true);

	paramsToDialog(font_);
	return true;
}


void GuiCharacter::dispatchParams()
{
	dispatch(FuncRequest(getLfun(), font_.toString(toggleall_)));
}


Dialog * createGuiCharacter(GuiView & lv) { return new GuiCharacter(lv); }


} // namespace frontend
} // namespace lyx

#include "GuiCharacter_moc.cpp"
