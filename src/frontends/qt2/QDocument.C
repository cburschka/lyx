/**
 * \file QDocument.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Edwin Leuven
 *
 * Full author contact details are available in file CREDITS
 */

#include <config.h>

#include "qt_helpers.h"

#include "ControlDocument.h"
#include "QDocument.h"
#include "QDocumentDialog.h"
#include "Qt2BC.h"

#include "language.h"
#include "helper_funcs.h" // getSecond()
#include "insets/insetquotes.h"
#include "frnt_lang.h"
#include "lyxrc.h" // defaultUnit
#include "tex-strings.h" // tex_graphics
#include "support/lstrings.h" // tostr()
#include "support/filetools.h" // LibFileSearch()
#include "lyxtextclasslist.h"
#include "vspace.h"
#include "bufferparams.h"
#include "qt_helpers.h"

#include <qpushbutton.h>
#include <qmultilineedit.h>
#include <qradiobutton.h>
#include <qcombobox.h>
#include <qcheckbox.h>
#include <qspinbox.h>
#include <qlineedit.h>
#include <qstringlist.h>
#include "lengthcombo.h"

#include "QBrowseBox.h"

#include <vector>

using std::vector;

typedef Qt2CB<ControlDocument, Qt2DB<QDocumentDialog> > base_class;


QDocument::QDocument()
	: base_class(qt_("Document Settings"))
{
	vector<frnt::LanguagePair> const langs = frnt::getLanguageData(false);
	lang_ = getSecond(langs);
}


void QDocument::build_dialog()
{
	dialog_.reset(new QDocumentDialog(this));

	// biblio
	dialog_->biblioModule->citeStyleCO->insertItem(qt_("Author-year"));
	dialog_->biblioModule->citeStyleCO->insertItem(qt_("Numerical"));
	dialog_->biblioModule->citeStyleCO->setCurrentItem(0);

	// language & quotes
	vector<frnt::LanguagePair> const langs = frnt::getLanguageData(false);
	vector<frnt::LanguagePair>::const_iterator lit  = langs.begin();
	vector<frnt::LanguagePair>::const_iterator lend = langs.end();
	for (; lit != lend; ++lit) {
		dialog_->langModule->languageCO->insertItem(
			toqstr(lit->first));
	}

	char const * items[] = {"default", "auto", "latin1", "latin2",
			     "latin3", "latin4", "latin5", "latin9",
			     "koi8-r", "koi8-u", "cp866", "cp1251",
			     "iso88595", "pt154", 0};
	dialog_->langModule->encodingCO->insertStrList(items);

	dialog_->langModule->quoteStyleCO->insertItem(qt_("``text''"));
	dialog_->langModule->quoteStyleCO->insertItem(qt_("''text''"));
	dialog_->langModule->quoteStyleCO->insertItem(qt_(",,text``"));
	dialog_->langModule->quoteStyleCO->insertItem(qt_(",,text''"));
	dialog_->langModule->quoteStyleCO->insertItem(qt_("<<text>>"));
	dialog_->langModule->quoteStyleCO->insertItem(qt_(">>text<<"));

	// packages
	for (int n = 0; tex_graphics[n][0]; ++n) {
		QString enc = tex_graphics[n];
		dialog_->packagesModule->psdriverCO->insertItem(enc);
	}

	// paper
	QComboBox * cb = dialog_->paperModule->papersizeCO;
	cb->insertItem(qt_("Default"));
       	cb->insertItem(qt_("Custom"));
       	cb->insertItem(qt_("US letter"));
       	cb->insertItem(qt_("US legal"));
	cb->insertItem(qt_("US executive"));
	cb->insertItem(qt_("A3"));
	cb->insertItem(qt_("A4"));
	cb->insertItem(qt_("A5"));
	cb->insertItem(qt_("B3"));
	cb->insertItem(qt_("B4"));
	cb->insertItem(qt_("B5"));

	// layout
	for (LyXTextClassList::const_iterator cit = textclasslist.begin();
	     cit != textclasslist.end(); ++cit) {
		if (cit->isTeXClassAvailable()) {
			dialog_->layoutModule->classCO->insertItem(toqstr(cit->description()));
                } else {
			dialog_->layoutModule->classCO->insertItem(toqstr("Unavailable: " +
									  cit->description()));
		}
	}

	for (int n = 0; tex_fonts[n][0]; ++n) {
		QString font = tex_fonts[n];
		dialog_->layoutModule->fontsCO->insertItem(font);
	}

	dialog_->layoutModule->fontsizeCO->insertItem(qt_("default"));
	dialog_->layoutModule->fontsizeCO->insertItem(qt_("10"));
	dialog_->layoutModule->fontsizeCO->insertItem(qt_("11"));
	dialog_->layoutModule->fontsizeCO->insertItem(qt_("12"));

	dialog_->layoutModule->skipCO->insertItem(qt_("SmallSkip"));
	dialog_->layoutModule->skipCO->insertItem(qt_("MedSkip"));
	dialog_->layoutModule->skipCO->insertItem(qt_("BigSkip"));
	dialog_->layoutModule->skipCO->insertItem(qt_("Length"));

	dialog_->layoutModule->pagestyleCO->insertItem(qt_("default"));
	dialog_->layoutModule->pagestyleCO->insertItem(qt_("empty"));
	dialog_->layoutModule->pagestyleCO->insertItem(qt_("plain"));
	dialog_->layoutModule->pagestyleCO->insertItem(qt_("headings"));
	dialog_->layoutModule->pagestyleCO->insertItem(qt_("fancy"));
	
	dialog_->layoutModule->lspacingCO->insertItem(
		qt_("Single"), Spacing::Single);
	dialog_->layoutModule->lspacingCO->insertItem(
		qt_("OneHalf"), Spacing::Onehalf);
	dialog_->layoutModule->lspacingCO->insertItem(
		qt_("Double"), Spacing::Double);
	dialog_->layoutModule->lspacingCO->insertItem(
		qt_("Custom"), Spacing::Other);

	// margins
	dialog_->setMargins(0);

	// Manage the restore, ok, apply, restore and cancel/close buttons
	bc().setOK(dialog_->okPB);
	bc().setApply(dialog_->applyPB);
	bc().setCancel(dialog_->closePB);
	bc().setRestore(dialog_->restorePB);
}


void QDocument::apply()
{
	BufferParams & params = controller().params();

	// preamble
	params.preamble =
		fromqstr(dialog_->preambleModule->preambleMLE->text());

	// biblio
	params.use_natbib =
		dialog_->biblioModule->natbibCB->isChecked();
	params.use_numerical_citations  =
		dialog_->biblioModule->citeStyleCO->currentItem();

	// language & quotes
	if (dialog_->langModule->singleQuoteRB->isChecked())
		params.quotes_times = InsetQuotes::SingleQ;
	else
		params.quotes_times = InsetQuotes::DoubleQ;

	params.inputenc =
		fromqstr(dialog_->langModule->encodingCO->currentText());

	InsetQuotes::quote_language lga = InsetQuotes::EnglishQ;
	switch (dialog_->langModule->quoteStyleCO->currentItem()) {
	case 0:
		lga = InsetQuotes::EnglishQ;
		break;
	case 1:
		lga = InsetQuotes::SwedishQ;
		break;
	case 2:
		lga = InsetQuotes::GermanQ;
		break;
	case 3:
		lga = InsetQuotes::PolishQ;
		break;
	case 4:
		lga = InsetQuotes::FrenchQ;
		break;
	case 5:
		lga = InsetQuotes::DanishQ;
		break;
	}
	params.quotes_language = lga;

	int const pos = dialog_->langModule->languageCO->currentItem();
	params.language = languages.getLanguage(lang_[pos]);

	// numbering
	params.tocdepth =
		dialog_->numberingModule->tocDepthSB->value();
	params.secnumdepth =
		dialog_->numberingModule->sectionnrDepthSB->value();

	// bullets
	params.user_defined_bullets[0] = dialog_->bulletsModule->getBullet(0);
	params.user_defined_bullets[1] = dialog_->bulletsModule->getBullet(1);
	params.user_defined_bullets[2] = dialog_->bulletsModule->getBullet(2);
	params.user_defined_bullets[3] = dialog_->bulletsModule->getBullet(3);

	// packages
	params.graphicsDriver =
		fromqstr(dialog_->packagesModule->psdriverCO->currentText());

	params.use_amsmath =
		dialog_->packagesModule->amsCB->isChecked();

	// layout
	params.textclass =
		dialog_->layoutModule->classCO->currentItem();
		
	params.fonts =
		fromqstr(dialog_->layoutModule->fontsCO->currentText());

	params.fontsize =
		fromqstr(dialog_->layoutModule->fontsizeCO->currentText());

	params.pagestyle =
		fromqstr(dialog_->layoutModule->pagestyleCO->currentText());
		
	switch (dialog_->layoutModule->lspacingCO->currentItem()) {
	case 0:
		params.spacing.set(Spacing::Single);
		break;
	case 1:
		params.spacing.set(Spacing::Onehalf);
		break;
	case 2:
		params.spacing.set(Spacing::Double);
		break;
	case 3:
		params.spacing.set(Spacing::Other,
				   dialog_->layoutModule->
				   lspacingLE->text().toFloat()
				   );
		break;
	}

	if (dialog_->layoutModule->indentRB->isChecked())
		params.paragraph_separation = BufferParams::PARSEP_INDENT;
	else
		params.paragraph_separation = BufferParams::PARSEP_SKIP;

	switch (dialog_->layoutModule->skipCO->currentItem()) {
	case 0:
		params.setDefSkip(VSpace(VSpace::SMALLSKIP));
		break;
	case 1:
		params.setDefSkip(VSpace(VSpace::MEDSKIP));
		break;
	case 2:
		params.setDefSkip(VSpace(VSpace::BIGSKIP));
		break;
	case 3:
	{
		LyXLength::UNIT unit =
			dialog_->layoutModule->skipLengthCO->
			currentLengthItem();
		double length =
			dialog_->layoutModule->skipLE->text().toDouble();
		VSpace vs = VSpace(LyXGlueLength(LyXLength(length,unit)));
		params.setDefSkip(vs);
		break;
	}
	default:
		// DocumentDefskipCB assures that this never happens
		// so Assert then !!!  - jbl
		params.setDefSkip(VSpace(VSpace::MEDSKIP));
		break;
	}

	params.options =
		fromqstr(dialog_->layoutModule->optionsLE->text());

	params.float_placement =
		fromqstr(dialog_->layoutModule->floatPlacementLE->text());

	// paper
	params.papersize2 =
		dialog_->paperModule->papersizeCO->currentItem();

	// custom, A3, B3 and B4 paper sizes need geometry
	int psize = dialog_->paperModule->papersizeCO->currentItem();
	bool geom_papersize = (psize == 1 || psize == 5 || psize == 8 || psize == 9);

	params.paperwidth = widgetsToLength(dialog_->paperModule->paperwidthLE,
		dialog_->paperModule->paperwidthUnitCO);

	params.paperheight = widgetsToLength(dialog_->paperModule->paperheightLE,
		dialog_->paperModule->paperheightUnitCO);

	if (dialog_->paperModule->twoColumnCB->isChecked())
		params.columns = 2;
	else
		params.columns = 1;

	if (dialog_->paperModule->facingPagesCB->isChecked())
		params.sides = LyXTextClass::TwoSides;
	else
		params.sides = LyXTextClass::OneSide;

	if (dialog_->paperModule->landscapeRB->isChecked())
		params.orientation = BufferParams::ORIENTATION_LANDSCAPE;
	else
		params.orientation = BufferParams::ORIENTATION_PORTRAIT;

	// margins
	params.use_geometry =
		(dialog_->marginsModule->marginCO->currentItem() == 1
		|| geom_papersize);

	int margin = dialog_->marginsModule->marginCO->currentItem();
	if (margin > 0) {
		margin = margin - 1;
	}
	params.paperpackage = char(margin);
	
	// set params.papersize from params.papersize2 
	// and params.paperpackage
	params.setPaperStuff();

	MarginsModuleBase const * m(dialog_->marginsModule);

	params.leftmargin = widgetsToLength(m->innerLE, m->innerUnit);

	params.topmargin = widgetsToLength(m->topLE, m->topUnit);

	params.rightmargin = widgetsToLength(m->outerLE, m->outerUnit);

	params.bottommargin = widgetsToLength(m->bottomLE, m->bottomUnit);

	params.headheight = widgetsToLength(m->headheightLE, m->headheightUnit);

	params.headsep = widgetsToLength(m->headsepLE, m->headsepUnit);

	params.footskip = widgetsToLength(m->footskipLE, m->footskipUnit);
}


namespace {

/** Return the position of val in the vector if found.
    If not found, return 0.
 */
template<class A>
typename std::vector<A>::size_type
findPos(std::vector<A> const & vec, A const & val)
{
	typename std::vector<A>::const_iterator it =
		std::find(vec.begin(), vec.end(), val);
	if (it == vec.end())
		return 0;
	return std::distance(vec.begin(), it);
}

} // namespace anom


void QDocument::update_contents()
{
	if (!dialog_.get())
		return;

	BufferParams const & params = controller().params();

	// set the default unit
	// FIXME: move to controller
	LyXLength::UNIT defaultUnit = LyXLength::CM;
	switch (lyxrc.default_papersize) {
		case BufferParams::PAPER_DEFAULT: break;

		case BufferParams::PAPER_USLETTER:
		case BufferParams::PAPER_LEGALPAPER:
		case BufferParams::PAPER_EXECUTIVEPAPER:
			defaultUnit = LyXLength::IN;
			break;

		case BufferParams::PAPER_A3PAPER:
		case BufferParams::PAPER_A4PAPER:
		case BufferParams::PAPER_A5PAPER:
		case BufferParams::PAPER_B5PAPER:
			defaultUnit = LyXLength::CM;
			break;
	}

	// preamble
	QString preamble = toqstr(params.preamble);
	dialog_->preambleModule->preambleMLE->setText(preamble);

	// biblio
	dialog_->biblioModule->natbibCB->setChecked(
		params.use_natbib);
	dialog_->biblioModule->citeStyleCO->setCurrentItem(
		params.use_numerical_citations ? 1 : 0);

	// language & quotes
	dialog_->langModule->singleQuoteRB->setChecked(
		params.quotes_times == InsetQuotes::SingleQ);
	dialog_->langModule->doubleQuoteRB->setChecked(
		params.quotes_times == InsetQuotes::DoubleQ);

	int const pos = int(findPos(lang_,
				    params.language->lang()));
	dialog_->langModule->languageCO->setCurrentItem(pos);

	dialog_->langModule->quoteStyleCO->setCurrentItem(
		params.quotes_language);
		
	char const * enc[] = {
		"default" , "auto" , "latin1" , "latin2" , "latin3" ,
		"latin4" , "latin5" , "latin9" , "koi8-r" , "koi8-u" ,
		"cp866" , "cp1251" , "iso88595" , "pt154" };
	for (size_t i = 0; i < sizeof(enc)/sizeof(char *); ++i) {
		if (params.inputenc == enc[i])
			dialog_->langModule->encodingCO->setCurrentItem(i);
	}

	// numbering
	dialog_->numberingModule->tocDepthSB->setValue(
		params.tocdepth);
	dialog_->numberingModule->sectionnrDepthSB->setValue(
		params.secnumdepth);

	// bullets
	dialog_->bulletsModule->setBullet(0,params.user_defined_bullets[0]);
	dialog_->bulletsModule->setBullet(1,params.user_defined_bullets[1]);
	dialog_->bulletsModule->setBullet(2,params.user_defined_bullets[2]);
	dialog_->bulletsModule->setBullet(3,params.user_defined_bullets[3]);

	// packages
	QString text = toqstr(params.graphicsDriver);
	int nitem = dialog_->packagesModule->psdriverCO->count();
	for (int n = 0; n < nitem ; ++n) {
		QString enc = tex_graphics[n];
		if (enc == text) {
			dialog_->packagesModule->psdriverCO->setCurrentItem(n);
		}
	}


	dialog_->packagesModule->amsCB->setChecked(
		params.use_amsmath);

	switch (params.spacing.getSpace()) {
		case Spacing::Other: nitem = 3; break;
		case Spacing::Double: nitem = 2; break;
		case Spacing::Onehalf: nitem = 1; break;
		case Spacing::Default: case Spacing::Single: nitem = 0; break;
	}


	// layout
	dialog_->layoutModule->classCO->setCurrentItem(params.textclass);

	dialog_->updateFontsize(controller().textClass().opt_fontsize(),
				params.fontsize);

	dialog_->updatePagestyle(controller().textClass().opt_pagestyle(),
				 params.pagestyle);

	for (int n = 0; tex_fonts[n][0]; ++n) {
		if (tex_fonts[n] == params.fonts) {
			dialog_->layoutModule->fontsCO->setCurrentItem(n);
			break;
		}
	}

	dialog_->layoutModule->lspacingCO->setCurrentItem(nitem);
	if (params.spacing.getSpace() == Spacing::Other) {
		dialog_->layoutModule->lspacingLE->setText(
			toqstr(tostr(params.spacing.getValue())));
		dialog_->setLSpacing(3);
	}

	if (params.paragraph_separation
	    == BufferParams::PARSEP_INDENT) {
		dialog_->layoutModule->indentRB->setChecked(true);
	} else {
		dialog_->layoutModule->skipRB->setChecked(true);
	}

	int skip = 0;
	switch (params.getDefSkip().kind()) {
	case VSpace::SMALLSKIP:
		skip = 0;
		break;
	case VSpace::MEDSKIP:
		skip = 1;
		break;
	case VSpace::BIGSKIP:
		skip = 2;
		break;
	case VSpace::LENGTH:
	{
		skip = 3;
		string const length = params.getDefSkip().asLyXCommand();
		dialog_->layoutModule->skipLengthCO->setCurrentItem(LyXLength(length).unit());
		dialog_->layoutModule->skipLE->setText(toqstr(tostr(LyXLength(length).value())));
		break;
	}
	default:
		skip = 0;
		break;
	}
	dialog_->layoutModule->skipCO->setCurrentItem(skip);
	dialog_->setSkip(skip);

	if (!params.options.empty()) {
		dialog_->layoutModule->optionsLE->setText(
			toqstr(params.options));
	} else {
		dialog_->layoutModule->optionsLE->setText("");
	}

	dialog_->layoutModule->floatPlacementLE->setText(toqstr(params.float_placement));

	// paper
	int const psize = params.papersize2;
	dialog_->paperModule->papersizeCO->setCurrentItem(psize);
	dialog_->setMargins(psize);
	dialog_->setCustomPapersize(psize);

	bool const landscape =
		params.orientation == BufferParams::ORIENTATION_LANDSCAPE;
	dialog_->paperModule->landscapeRB->setChecked(landscape);
	dialog_->paperModule->portraitRB->setChecked(!landscape);

	dialog_->paperModule->facingPagesCB->setChecked(
		params.sides == LyXTextClass::TwoSides);

	dialog_->paperModule->twoColumnCB->setChecked(
		params.columns == 2);


	lengthToWidgets(dialog_->paperModule->paperwidthLE,
		dialog_->paperModule->paperwidthUnitCO, params.paperwidth, defaultUnit);

	lengthToWidgets(dialog_->paperModule->paperheightLE,
		dialog_->paperModule->paperheightUnitCO, params.paperheight, defaultUnit);

	// margins

	MarginsModuleBase * m(dialog_->marginsModule);

	int item = params.paperpackage;
	if (params.use_geometry) {
		item = 1;
	} else if (item > 0) {
		item = item + 1;
	}
	m->marginCO->setCurrentItem(item);
	dialog_->setCustomMargins(item);

	lengthToWidgets(m->topLE, m->topUnit,
		params.topmargin, defaultUnit);

	lengthToWidgets(m->bottomLE, m->bottomUnit,
		params.bottommargin, defaultUnit);

	lengthToWidgets(m->innerLE, m->innerUnit,
		params.leftmargin, defaultUnit);

	lengthToWidgets(m->outerLE, m->outerUnit,
		params.rightmargin, defaultUnit);

	lengthToWidgets(m->headheightLE, m->headheightUnit,
		params.headheight, defaultUnit);

	lengthToWidgets(m->headsepLE, m->headsepUnit,
		params.headsep, defaultUnit);

	lengthToWidgets(m->footskipLE, m->footskipUnit,
		params.footskip, defaultUnit);
}


void QDocument::saveDocDefault()
{
	// we have to apply the params first
	apply();
	controller().saveAsDefault();
}


void QDocument::useClassDefaults()
{
	BufferParams & params = controller().params();

	params.textclass = dialog_->layoutModule->classCO->currentItem();
	params.useClassDefaults();
	update_contents();
}
