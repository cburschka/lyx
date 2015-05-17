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

#include "GuiApplication.h"
#include "qt_helpers.h"

#include "Font.h"
#include "Buffer.h"
#include "BufferParams.h"
#include "BufferView.h"
#include "Color.h"
#include "ColorCache.h"
#include "Cursor.h"
#include "FuncRequest.h"
#include "Language.h"
#include "Paragraph.h"

#include "support/gettext.h"

#include <QAbstractItemModel>
#include <QComboBox>
#include <QModelIndex>
#include <QSettings>
#include <QVariant>

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
	bars << BarPair(qt_("Double underbar"),  UULINE_TOGGLE);
	bars << BarPair(qt_("Wavy underbar"),  UWAVE_TOGGLE);
	bars << BarPair(qt_("Strikeout"),  STRIKEOUT_TOGGLE);
	bars << BarPair(qt_("Noun"),      NOUN_TOGGLE);
	bars << BarPair(qt_("Reset"),     INHERIT);
	return bars;
}


static QList<ColorCode> colorData()
{
	QList<ColorCode> colors;
	colors << Color_black;
	colors << Color_blue;
	colors << Color_brown;
	colors << Color_cyan;
	colors << Color_darkgray;
	colors << Color_gray;
	colors << Color_green;
	colors << Color_lightgray;
	colors << Color_lime;
	colors << Color_magenta;
	colors << Color_olive;
	colors << Color_orange;
	colors << Color_pink;
	colors << Color_purple;
	colors << Color_red;
	colors << Color_teal;
	colors << Color_violet;
	colors << Color_white;
	colors << Color_yellow;
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


static QList<LanguagePair> languageData()
{
	QList<LanguagePair> list;
	// FIXME (Abdel 14/05/2008): it would be nice if we could use this model
	// directly in the language combo; but, as we need also the 'No Change' and
	// 'Reset' items, this is not possible right now. Separating those two
	// entries in radio buttons would be a better GUI IMHO.
	QAbstractItemModel * language_model = guiApp->languageModel();
	// Make sure the items are sorted.
	language_model->sort(0);

	for (int i = 0; i != language_model->rowCount(); ++i) {
		QModelIndex index = language_model->index(i, 0);
		list << LanguagePair(index.data(Qt::DisplayRole).toString(),
			index.data(Qt::UserRole).toString());
	}
	return list;
}


namespace {

template<typename T>
void fillCombo(QComboBox * combo, QList<T> const & list)
{
	typename QList<T>::const_iterator cit = list.begin();
	for (; cit != list.end(); ++cit)
		combo->addItem(cit->first);
}

template<typename T>
void fillComboColor(QComboBox * combo, QList<T> const & list)
{
	// at first add the 2 colors "No change" and "No color"
	combo->addItem(qt_("No change"), "ignore");
	combo->addItem(qt_("No color"), "none");
	// now add the real colors
	QPixmap coloritem(32, 32);
	QColor color;
	QList<ColorCode>::const_iterator cit = list.begin();
	for (; cit != list.end(); ++cit) {
		QString const lyxname = toqstr(lcolor.getLyXName(*cit));
		QString const guiname = toqstr(translateIfPossible(lcolor.getGUIName(*cit)));
		color = QColor(guiApp->colorCache().get(*cit, false));
		coloritem.fill(color);
		combo->addItem(QIcon(coloritem), guiname, lyxname);
	}
	//the last color is "Reset"
	combo->addItem(qt_("Reset"), "inherit");
}

}

GuiCharacter::GuiCharacter(GuiView & lv)
	: GuiDialog(lv, "character", qt_("Text Style")), font_(ignore_font, ignore_language),
	  toggleall_(false)
{
	setupUi(this);

	connect(okPB, SIGNAL(clicked()), this, SLOT(slotOK()));
	connect(applyPB, SIGNAL(clicked()), this, SLOT(slotApply()));
	connect(closePB, SIGNAL(clicked()), this, SLOT(slotClose()));
	connect(autoapplyCB, SIGNAL(stateChanged(int)), this,
		SLOT(slotAutoApply()));

	connect(miscCO, SIGNAL(activated(int)), this, SLOT(change_adaptor()));
	connect(sizeCO, SIGNAL(activated(int)), this, SLOT(change_adaptor()));
	connect(familyCO, SIGNAL(activated(int)), this, SLOT(change_adaptor()));
	connect(seriesCO, SIGNAL(activated(int)), this, SLOT(change_adaptor()));
	connect(shapeCO, SIGNAL(activated(int)), this, SLOT(change_adaptor()));
	connect(colorCO, SIGNAL(activated(int)), this, SLOT(change_adaptor()));
	connect(langCO, SIGNAL(activated(int)), this, SLOT(change_adaptor()));
	connect(toggleallCB, SIGNAL(clicked()), this, SLOT(change_adaptor()));

	family = familyData();
	series = seriesData();
	shape  = shapeData();
	size   = sizeData();
	bar    = barData();
	color  = colorData();
	sort(color.begin(), color.end(), ColorSorter());

	language = languageData();
	language.prepend(LanguagePair(qt_("Reset"), "reset"));
	language.prepend(LanguagePair(qt_("No change"), "ignore"));

	fillCombo(familyCO, family);
	fillCombo(seriesCO, series);
	fillCombo(sizeCO, size);
	fillCombo(shapeCO, shape);
	fillCombo(miscCO, bar);
	fillComboColor(colorCO, color);
	fillCombo(langCO, language);

	bc().setPolicy(ButtonPolicy::OkApplyCancelAutoReadOnlyPolicy);
	bc().setOK(okPB);
	bc().setApply(applyPB);
	bc().setCancel(closePB);
	bc().setAutoApply(autoapplyCB);
	bc().addReadOnly(familyCO);
	bc().addReadOnly(seriesCO);
	bc().addReadOnly(sizeCO);
	bc().addReadOnly(shapeCO);
	bc().addReadOnly(miscCO);
	bc().addReadOnly(langCO);
	bc().addReadOnly(colorCO);
	bc().addReadOnly(toggleallCB);
	bc().addReadOnly(autoapplyCB);

#ifdef Q_OS_MAC
	// On Mac it's common to have tool windows which are always in the
	// foreground and are hidden when the main window is not focused.
	setWindowFlags(Qt::Tool);
	autoapplyCB->setChecked(true);
#endif

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
	if (!autoapplyCB->isChecked()) {
		bc().setValid(true);
		return;
	}
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

	if (fi.strikeout() == FONT_TOGGLE)
		return STRIKEOUT_TOGGLE;

	if (fi.uuline() == FONT_TOGGLE)
		return UULINE_TOGGLE;

	if (fi.uwave() == FONT_TOGGLE)
		return UWAVE_TOGGLE;

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
		fi.setStrikeout(FONT_IGNORE);
		fi.setUuline(FONT_IGNORE);
		fi.setUwave(FONT_IGNORE);
		fi.setNoun(FONT_IGNORE);
		break;

	case EMPH_TOGGLE:
		setBar(fi, INHERIT);
		fi.setEmph(FONT_TOGGLE);
		break;

	case UNDERBAR_TOGGLE:
		setBar(fi, INHERIT);
		fi.setUnderbar(FONT_TOGGLE);
		break;

	case STRIKEOUT_TOGGLE:
		setBar(fi, INHERIT);
		fi.setStrikeout(FONT_TOGGLE);
		break;

	case UULINE_TOGGLE:
		setBar(fi, INHERIT);
		fi.setUuline(FONT_TOGGLE);
		break;

	case UWAVE_TOGGLE:
		setBar(fi, INHERIT);
		fi.setUwave(FONT_TOGGLE);
		break;

	case NOUN_TOGGLE:
		setBar(fi, INHERIT);
		fi.setNoun(FONT_TOGGLE);
		break;

	case INHERIT:
		fi.setEmph(FONT_INHERIT);
		fi.setUnderbar(FONT_INHERIT);
		fi.setStrikeout(FONT_INHERIT);
		fi.setUuline(FONT_INHERIT);
		fi.setUwave(FONT_INHERIT);
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
	colorCO->setCurrentIndex(colorCO->findData(toqstr(lcolor.getLyXName(fi.color()))));

	// reset_language is a null pointer.
	QString const lang = (font.language() == reset_language)
		? "reset" : toqstr(font.language()->lang());
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
	fi.setColor(lcolor.getFromLyXName(fromqstr(colorCO->itemData(colorCO->currentIndex()).toString())));

	font_.setLanguage(languages.getLanguage(
		fromqstr(language[langCO->currentIndex()].second)));

	toggleall_ = toggleallCB->isChecked();
}


bool GuiCharacter::initialiseParams(string const &)
{
	if (autoapplyCB->isChecked())
		return true;

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


void GuiCharacter::saveSession() const
{
	Dialog::saveSession();
	QSettings settings;
	settings.setValue(sessionKey() + "/toggleall", toggleallCB->isChecked());
	settings.setValue(sessionKey() + "/autoapply", autoapplyCB->isChecked());
}


void GuiCharacter::restoreSession()
{
	Dialog::restoreSession();
	QSettings settings;
	toggleallCB->setChecked(
		settings.value(sessionKey() + "/toggleall").toBool());
	autoapplyCB->setChecked(
		settings.value(sessionKey() + "/autoapply").toBool());
}


Dialog * createGuiCharacter(GuiView & lv) { return new GuiCharacter(lv); }


} // namespace frontend
} // namespace lyx

#include "moc_GuiCharacter.cpp"
