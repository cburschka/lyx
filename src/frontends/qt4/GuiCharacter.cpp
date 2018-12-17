/**
 * \file GuiCharacter.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Angus Leeming
 * \author Edwin Leuven
 * \author John Levon
 * \author Jürgen Spitzmüller
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
#include "ColorSet.h"
#include "Cursor.h"
#include "FuncRequest.h"
#include "Language.h"
#include "Paragraph.h"

#include "support/gettext.h"
#include "support/lstrings.h"

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
	shapes << ShapePair(qt_("Default"), INHERIT_SHAPE);
	shapes << ShapePair(qt_("Upright"), UP_SHAPE);
	shapes << ShapePair(qt_("Italic"), ITALIC_SHAPE);
	shapes << ShapePair(qt_("Slanted"), SLANTED_SHAPE);
	shapes << ShapePair(qt_("Small Caps"), SMALLCAPS_SHAPE);
	return shapes;
}


static QList<SizePair> sizeData()
{
	QList<SizePair> sizes;
	sizes << SizePair(qt_("No change"), FONT_SIZE_IGNORE);
	sizes << SizePair(qt_("Default"), FONT_SIZE_INHERIT);
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
	return sizes;
}


static QList<BarPair> barData()
{
	QList<BarPair> bars;
	bars << BarPair(qt_("No change"), IGNORE);
	bars << BarPair(qt_("Default"), INHERIT);
	bars << BarPair(qt_("(Without)[[underlining]]"), NONE);
	bars << BarPair(qt_("Single[[underlining]]"), UNDERBAR);
	bars << BarPair(qt_("Double[[underlining]]"), UULINE);
	bars << BarPair(qt_("Wavy"), UWAVE);
	return bars;
}


static QList<BarPair> strikeData()
{
	QList<BarPair> strike;
	strike << BarPair(qt_("No change"), IGNORE);
	strike << BarPair(qt_("Default"), INHERIT);
	strike << BarPair(qt_("(Without)[[strikethrough]]"), NONE);
	strike << BarPair(qt_("Single[[strikethrough]]"), STRIKEOUT);
	strike << BarPair(qt_("With /"), XOUT);
	return strike;
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
	series << SeriesPair(qt_("Default"),     INHERIT_SERIES);
	series << SeriesPair(qt_("Medium"),    MEDIUM_SERIES);
	series << SeriesPair(qt_("Bold"),      BOLD_SERIES);
	return series;
}


static QList<FamilyPair> familyData()
{
	QList<FamilyPair> families;
	families << FamilyPair(qt_("No change"),  IGNORE_FAMILY);
	families << FamilyPair(qt_("Default"),      INHERIT_FAMILY);
	families << FamilyPair(qt_("Roman"),      ROMAN_FAMILY);
	families << FamilyPair(qt_("Sans Serif"), SANS_FAMILY);
	families << FamilyPair(qt_("Typewriter"), TYPEWRITER_FAMILY);
	return families;
}


static QList<LanguagePair> languageData()
{
	QList<LanguagePair> list;
	// FIXME (Abdel 14/05/2008): it would be nice if we could use this model
	// directly in the language combo; but, as we need also the 'No Change' and
	// 'Default' items, this is not possible right now. Separating those two
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
	combo->addItem(qt_("Default"), "inherit");
	combo->addItem(qt_("(Without)[[color]]"), "none");
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
}

} // namespace

GuiCharacter::GuiCharacter(GuiView & lv)
	: GuiDialog(lv, "character", qt_("Text Style")),
	  font_(ignore_font, ignore_language), emph_(false), noun_(false)
{
	setupUi(this);

	connect(okPB, SIGNAL(clicked()), this, SLOT(slotOK()));
	connect(applyPB, SIGNAL(clicked()), this, SLOT(slotApply()));
	connect(closePB, SIGNAL(clicked()), this, SLOT(slotClose()));
	connect(autoapplyCB, SIGNAL(stateChanged(int)), this,
		SLOT(slotAutoApply()));

	connect(ulineCO, SIGNAL(currentIndexChanged(int)), this, SLOT(change_adaptor()));
	connect(strikeCO, SIGNAL(currentIndexChanged(int)), this, SLOT(change_adaptor()));
	connect(sizeCO, SIGNAL(currentIndexChanged(int)), this, SLOT(change_adaptor()));
	connect(familyCO, SIGNAL(currentIndexChanged(int)), this, SLOT(change_adaptor()));
	connect(seriesCO, SIGNAL(currentIndexChanged(int)), this, SLOT(change_adaptor()));
	connect(shapeCO, SIGNAL(currentIndexChanged(int)), this, SLOT(change_adaptor()));
	connect(colorCO, SIGNAL(currentIndexChanged(int)), this, SLOT(change_adaptor()));
	connect(langCO, SIGNAL(currentIndexChanged(int)), this, SLOT(change_adaptor()));

	family = familyData();
	series = seriesData();
	shape  = shapeData();
	size   = sizeData();
	bar    = barData();
	strike = strikeData();
	color  = colorData();
	qSort(color.begin(), color.end(), ColorSorter);

	language = languageData();
	language.prepend(LanguagePair(qt_("Default"), "reset"));
	language.prepend(LanguagePair(qt_("No change"), "ignore"));

	fillCombo(familyCO, family);
	fillCombo(seriesCO, series);
	fillCombo(sizeCO, size);
	fillCombo(shapeCO, shape);
	fillCombo(ulineCO, bar);
	fillCombo(strikeCO, strike);
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
	bc().addReadOnly(ulineCO);
	bc().addReadOnly(strikeCO);
	bc().addReadOnly(nounCB);
	bc().addReadOnly(emphCB);
	bc().addReadOnly(langCO);
	bc().addReadOnly(colorCO);
	bc().addReadOnly(autoapplyCB);

#ifdef Q_OS_MAC
	// On Mac it's common to have tool windows which are always in the
	// foreground and are hidden when the main window is not focused.
	setWindowFlags(Qt::Tool);
	autoapplyCB->setChecked(true);
#endif
}


void GuiCharacter::on_emphCB_clicked()
{
	// skip intermediate state at user click
	if (!emph_) {
		emphCB->setCheckState(Qt::Checked);
		emph_ = true;
	}
	change_adaptor();
}


void GuiCharacter::on_nounCB_clicked()
{
	// skip intermediate state at user click
	if (!noun_) {
		nounCB->setCheckState(Qt::Checked);
		noun_ = true;
	}
	change_adaptor();
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


namespace{
FontState getBar(FontInfo const & fi)
{
	if (fi.underbar() == FONT_ON)
		return UNDERBAR;

	if (fi.uuline() == FONT_ON)
		return UULINE;

	if (fi.uwave() == FONT_ON)
		return UWAVE;

	if (fi.underbar() == FONT_IGNORE)
		return IGNORE;

	return NONE;
}


FontState getStrike(FontInfo const & fi)
{
	if (fi.strikeout() == FONT_ON)
		return STRIKEOUT;

	if (fi.xout() == FONT_ON)
		return XOUT;

	if (fi.strikeout() == FONT_IGNORE)
		return IGNORE;

	return NONE;
}


Qt::CheckState getMarkupState(lyx::FontState fs)
{
	switch (fs) {
	case FONT_OFF:
		return Qt::Unchecked;
	case FONT_ON:
		return Qt::Checked;
	case FONT_TOGGLE:
	case FONT_INHERIT:
	case FONT_IGNORE:
	default:
		return Qt::PartiallyChecked;
	}
}

lyx::FontState setMarkupState(Qt::CheckState cs)
{
	switch (cs) {
	case Qt::Unchecked:
		return FONT_OFF;
	case Qt::Checked:
		return FONT_ON;
	case Qt::PartiallyChecked:
	default:
		return FONT_IGNORE;
	}
}

} // end namespace anon


void GuiCharacter::updateContents()
{
	if (bufferview()->cursor().selection()) {
		Font font = bufferview()->cursor().current_font;
		FontInfo fi = font.fontInfo();
		BufferParams const & bp = buffer().masterParams();

		// Check if each font attribute is constant for the selection range.
		DocIterator const from = bufferview()->cursor().selectionBegin();
		DocIterator const to = bufferview()->cursor().selectionEnd();
		for (DocIterator dit = from ; dit != to && !dit.atEnd(); ) {
			if (!dit.inTexted()) {
				dit.forwardPos();
				continue;
			}
			Paragraph const & par = dit.paragraph();
			pos_type const pos = dit.pos();
			Font tmp = par.getFont(bp, pos, font);
			if (font.language() != tmp.language())
				font.setLanguage(ignore_language);
			if (fi.family() != tmp.fontInfo().family())
				font.fontInfo().setFamily(IGNORE_FAMILY);
			if (fi.series() != tmp.fontInfo().series())
				font.fontInfo().setSeries(IGNORE_SERIES);
			if (fi.shape() != tmp.fontInfo().shape())
				font.fontInfo().setShape(IGNORE_SHAPE);
			if (fi.size() != tmp.fontInfo().size())
				font.fontInfo().setSize(FONT_SIZE_IGNORE);
			if (fi.emph() != tmp.fontInfo().emph())
				font.fontInfo().setEmph(FONT_IGNORE);
			if (fi.noun() != tmp.fontInfo().noun())
				font.fontInfo().setNoun(FONT_IGNORE);
			if (fi.color() != tmp.fontInfo().color())
				font.fontInfo().setColor(Color_ignore);
			if (fi.underbar() != tmp.fontInfo().underbar()
			    || fi.uuline() != tmp.fontInfo().uuline()
			    || fi.uwave() != tmp.fontInfo().uwave())
				setBar(font.fontInfo(), IGNORE);
			if (fi.strikeout() != tmp.fontInfo().strikeout()
			    || fi.xout() != tmp.fontInfo().xout())
				setStrike(font.fontInfo(), IGNORE);
			dit.forwardPos();
		}
		font_ = font;
	} else
		font_ = bufferview()->cursor().current_font;

	paramsToDialog(font_);
}


void GuiCharacter::setBar(FontInfo & fi, FontState val)
{
	switch (val) {
	case IGNORE:
		fi.setUnderbar(FONT_IGNORE);
		fi.setUuline(FONT_IGNORE);
		fi.setUwave(FONT_IGNORE);
		break;
	case UNDERBAR:
		setBar(fi, NONE);
		fi.setUnderbar(FONT_ON);
		break;
	case UULINE:
		setBar(fi, NONE);
		fi.setUuline(FONT_ON);
		break;
	case UWAVE:
		setBar(fi, NONE);
		fi.setUwave(FONT_ON);
		break;
	case INHERIT:
		fi.setUnderbar(FONT_INHERIT);
		fi.setUuline(FONT_INHERIT);
		fi.setUwave(FONT_INHERIT);
		break;
	case NONE:
		fi.setUnderbar(FONT_OFF);
		fi.setUuline(FONT_OFF);
		fi.setUwave(FONT_OFF);
		break;
	case XOUT:
	case STRIKEOUT:
	default:
		break;
	}
}


void GuiCharacter::setStrike(FontInfo & fi, FontState val)
{
	switch (val) {
	case IGNORE:
		fi.setStrikeout(FONT_IGNORE);
		fi.setXout(FONT_IGNORE);
		break;
	case STRIKEOUT:
		setStrike(fi, NONE);
		fi.setStrikeout(FONT_ON);
		break;
	case XOUT:
		setStrike(fi, NONE);
		fi.setXout(FONT_ON);
		break;
	case INHERIT:
		fi.setStrikeout(FONT_INHERIT);
		fi.setXout(FONT_INHERIT);
		break;
	case NONE:
		fi.setStrikeout(FONT_OFF);
		fi.setXout(FONT_OFF);
		break;
	case UNDERBAR:
	case UWAVE:
	case UULINE:
	default:
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
	ulineCO->setCurrentIndex(findPos2nd(bar, getBar(fi)));
	strikeCO->setCurrentIndex(findPos2nd(strike, getStrike(fi)));
	colorCO->setCurrentIndex(colorCO->findData(toqstr(lcolor.getLyXName(fi.color()))));
	emphCB->setCheckState(getMarkupState(fi.emph()));
	nounCB->setCheckState(getMarkupState(fi.noun()));
	emph_ = emphCB->checkState() == Qt::Checked;
	noun_ = nounCB->checkState() == Qt::Checked;

	// reset_language is a null pointer.
	QString const lang = (font.language() == reset_language)
		? "reset" : toqstr(font.language()->lang());
	langCO->setCurrentIndex(findPos2nd(language, lang));
}


void GuiCharacter::applyView()
{
	FontInfo & fi = font_.fontInfo();
	fi.setFamily(family[familyCO->currentIndex()].second);
	fi.setSeries(series[seriesCO->currentIndex()].second);
	fi.setShape(shape[shapeCO->currentIndex()].second);
	fi.setSize(size[sizeCO->currentIndex()].second);
	fi.setEmph(setMarkupState(emphCB->checkState()));
	fi.setNoun(setMarkupState(nounCB->checkState()));
	setBar(fi, bar[ulineCO->currentIndex()].second);
	setStrike(fi, strike[strikeCO->currentIndex()].second);
	fi.setColor(lcolor.getFromLyXName(fromqstr(colorCO->itemData(colorCO->currentIndex()).toString())));

	font_.setLanguage(languages.getLanguage(
		fromqstr(language[langCO->currentIndex()].second)));
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
	// Make sure that the bc is in the INITIAL state
	if (bc().policy().buttonStatus(ButtonPolicy::OKAY))
		bc().restore();
	return true;
}


void GuiCharacter::dispatchParams()
{
	dispatch(FuncRequest(getLfun(), font_.toString(false)));
}


void GuiCharacter::saveSession(QSettings & settings) const
{
	Dialog::saveSession(settings);
	settings.setValue(sessionKey() + "/autoapply", autoapplyCB->isChecked());
}


void GuiCharacter::restoreSession()
{
	Dialog::restoreSession();
	QSettings settings;
	autoapplyCB->setChecked(
		settings.value(sessionKey() + "/autoapply").toBool());
}


Dialog * createGuiCharacter(GuiView & lv) { return new GuiCharacter(lv); }


} // namespace frontend
} // namespace lyx

#include "moc_GuiCharacter.cpp"
