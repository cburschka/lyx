/**
 * \file QDocument.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Edwin Leuven
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "debug.h"
#include "qt_helpers.h"

#include "ControlDocument.h"
#include "QDocument.h"
#include "QDocumentDialog.h"

#include "bufferparams.h"
#include "language.h"
#include "helper_funcs.h" // getSecond()
#include "frnt_lang.h"
#include "lyxrc.h" // defaultUnit
#include "tex-strings.h" // tex_graphics
#include "support/tostr.h"
#include "support/lstrings.h"
#include "lyxtextclasslist.h"
#include "floatplacement.h"
#include "LColor.h"

#include <qpushbutton.h>
#include <qmultilineedit.h>
#include <qradiobutton.h>
#include <qcheckbox.h>
#include <qslider.h>
#include <qlineedit.h>
#include "lengthcombo.h"


using lyx::support::bformat;
using lyx::support::getVectorFromString;

using std::vector;
using std::string;


typedef Qt2CB<ControlDocument, Qt2DB<QDocumentDialog> > base_class;


namespace {

char const * encodings[] = { "LaTeX default", "latin1", "latin2",
	"latin3", "latin4", "latin5", "latin9",
	"koi8-r", "koi8-u", "cp866", "cp1251",
	"iso88595", "pt154", 0
};

}


QDocument::QDocument()
	: base_class(_("LyX: Document Settings")),
		lang_(getSecond(frnt::getLanguageData(false)))
{}


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

	int i = 0;
	while (encodings[i]) {
		dialog_->langModule->encodingCO->insertItem(qt_(encodings[i++]));
	}

	dialog_->langModule->quoteStyleCO->insertItem(qt_("``text''"));
	dialog_->langModule->quoteStyleCO->insertItem(qt_("''text''"));
	dialog_->langModule->quoteStyleCO->insertItem(qt_(",,text``"));
	dialog_->langModule->quoteStyleCO->insertItem(qt_(",,text''"));
	dialog_->langModule->quoteStyleCO->insertItem(qt_("<<text>>"));
	dialog_->langModule->quoteStyleCO->insertItem(qt_(">>text<<"));

	// packages
	for (int n = 0; tex_graphics[n][0]; ++n) {
		QString enc = tex_graphics[n];
		dialog_->latexModule->psdriverCO->insertItem(enc);
	}

	// paper
	QComboBox * cb = dialog_->pageLayoutModule->papersizeCO;
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
			dialog_->latexModule->classCO->insertItem(toqstr(cit->description()));
                } else {
			string item =
				bformat(_("Unavailable: %1$s"), cit->description());
			dialog_->latexModule->classCO->insertItem(toqstr(item));
		}
	}

	for (int n = 0; tex_fonts[n][0]; ++n) {
		QString font = tex_fonts[n];
		dialog_->textLayoutModule->fontsCO->insertItem(font);
	}

	dialog_->textLayoutModule->fontsizeCO->insertItem(qt_("default"));
	dialog_->textLayoutModule->fontsizeCO->insertItem(qt_("10"));
	dialog_->textLayoutModule->fontsizeCO->insertItem(qt_("11"));
	dialog_->textLayoutModule->fontsizeCO->insertItem(qt_("12"));

	dialog_->textLayoutModule->skipCO->insertItem(qt_("SmallSkip"));
	dialog_->textLayoutModule->skipCO->insertItem(qt_("MedSkip"));
	dialog_->textLayoutModule->skipCO->insertItem(qt_("BigSkip"));
	dialog_->textLayoutModule->skipCO->insertItem(qt_("Length"));

	dialog_->pageLayoutModule->pagestyleCO->insertItem(qt_("default"));
	dialog_->pageLayoutModule->pagestyleCO->insertItem(qt_("empty"));
	dialog_->pageLayoutModule->pagestyleCO->insertItem(qt_("plain"));
	dialog_->pageLayoutModule->pagestyleCO->insertItem(qt_("headings"));
	dialog_->pageLayoutModule->pagestyleCO->insertItem(qt_("fancy"));

	dialog_->textLayoutModule->lspacingCO->insertItem(
		qt_("Single"), Spacing::Single);
	dialog_->textLayoutModule->lspacingCO->insertItem(
		qt_("OneHalf"), Spacing::Onehalf);
	dialog_->textLayoutModule->lspacingCO->insertItem(
		qt_("Double"), Spacing::Double);
	dialog_->textLayoutModule->lspacingCO->insertItem(
		qt_("Custom"), Spacing::Other);

	// margins
	dialog_->setMargins(0);

	// Manage the restore, ok, apply, restore and cancel/close buttons
	bcview().setOK(dialog_->okPB);
	bcview().setApply(dialog_->applyPB);
	bcview().setCancel(dialog_->closePB);
	bcview().setRestore(dialog_->restorePB);
}


void QDocument::showPreamble()
{
	dialog_->showPreamble();
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

	if (dialog_->langModule->defaultencodingCB->isChecked()) {
		params.inputenc = "auto";
	} else {
		int i = dialog_->langModule->encodingCO->currentItem();
		if (i == 0) {
			params.inputenc = "default";
		} else {
			params.inputenc = encodings[i];
		}
	}

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
	params.tocdepth = dialog_->numberingModule->tocSL->value();
	params.secnumdepth = dialog_->numberingModule->depthSL->value();

	// bullets
	params.user_defined_bullet(0) = dialog_->bulletsModule->getBullet(0);
	params.user_defined_bullet(1) = dialog_->bulletsModule->getBullet(1);
	params.user_defined_bullet(2) = dialog_->bulletsModule->getBullet(2);
	params.user_defined_bullet(3) = dialog_->bulletsModule->getBullet(3);

	// packages
	params.graphicsDriver =
		fromqstr(dialog_->latexModule->psdriverCO->currentText());

	if (dialog_->mathsModule->amsautoCB->isChecked()) {
		params.use_amsmath = BufferParams::AMS_AUTO;
	} else {
		if (dialog_->mathsModule->amsCB->isChecked())
			params.use_amsmath = BufferParams::AMS_ON;
		else
			params.use_amsmath = BufferParams::AMS_OFF;
	}

	// layout
	params.textclass =
		dialog_->latexModule->classCO->currentItem();

	params.fonts =
		fromqstr(dialog_->textLayoutModule->fontsCO->currentText());

	params.fontsize =
		fromqstr(dialog_->textLayoutModule->fontsizeCO->currentText());

	params.pagestyle =
		fromqstr(dialog_->pageLayoutModule->pagestyleCO->currentText());

	switch (dialog_->textLayoutModule->lspacingCO->currentItem()) {
	case 0:
		params.spacing().set(Spacing::Single);
		break;
	case 1:
		params.spacing().set(Spacing::Onehalf);
		break;
	case 2:
		params.spacing().set(Spacing::Double);
		break;
	case 3:
		params.spacing().set(Spacing::Other,
				   dialog_->textLayoutModule->
				   lspacingLE->text().toFloat()
				   );
		break;
	}

	if (dialog_->textLayoutModule->twoColumnCB->isChecked())
		params.columns = 2;
	else
		params.columns = 1;

	if (dialog_->textLayoutModule->indentRB->isChecked())
		params.paragraph_separation = BufferParams::PARSEP_INDENT;
	else
		params.paragraph_separation = BufferParams::PARSEP_SKIP;

	switch (dialog_->textLayoutModule->skipCO->currentItem()) {
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
			dialog_->textLayoutModule->skipLengthCO->
			currentLengthItem();
		double length =
			dialog_->textLayoutModule->skipLE->text().toDouble();
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
		fromqstr(dialog_->latexModule->optionsLE->text());

	params.float_placement = dialog_->floatModule->get();

	// paper
	params.papersize2 = VMARGIN_PAPER_TYPE(
		dialog_->pageLayoutModule->papersizeCO->currentItem());

	params.paperwidth = widgetsToLength(dialog_->pageLayoutModule->paperwidthLE,
		dialog_->pageLayoutModule->paperwidthUnitCO);

	params.paperheight = widgetsToLength(dialog_->pageLayoutModule->paperheightLE,
		dialog_->pageLayoutModule->paperheightUnitCO);

	if (dialog_->pageLayoutModule->facingPagesCB->isChecked())
		params.sides = LyXTextClass::TwoSides;
	else
		params.sides = LyXTextClass::OneSide;

	if (dialog_->pageLayoutModule->landscapeRB->isChecked())
		params.orientation = ORIENTATION_LANDSCAPE;
	else
		params.orientation = ORIENTATION_PORTRAIT;

	// margins
	params.use_geometry =
		(dialog_->marginsModule->marginCO->currentItem() == 1);

	int margin = dialog_->marginsModule->marginCO->currentItem();
	if (margin > 0) {
		margin = margin - 1;
	}
	params.paperpackage = PAPER_PACKAGES(margin);

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

	// branches
	string const all_branches = params.branchlist().allBranches();
	if (!all_branches.empty()) {
		std::vector<string> all = getVectorFromString(all_branches, "|");
		for (unsigned i = 0; i < all.size(); ++i) {
			string const current_branch = all[i].c_str();
			string x11hexname = params.branchlist().getColor(current_branch);
			// check that we have a valid color!
			if (x11hexname.empty() || x11hexname[0] != '#')
				x11hexname = lcolor.getX11Name(LColor::background);
			// display the new color
			controller().setBranchColor(current_branch, x11hexname);
		}
	}
	if (branchlist_.empty())
		branchlist_ = params.branchlist();
	params.branchlist() = branchlist_;
	branchlist_.clear();
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
		case PAPER_DEFAULT: break;

		case PAPER_USLETTER:
		case PAPER_LEGALPAPER:
		case PAPER_EXECUTIVEPAPER:
			defaultUnit = LyXLength::IN;
			break;

		case PAPER_A3PAPER:
		case PAPER_A4PAPER:
		case PAPER_A5PAPER:
		case PAPER_B5PAPER:
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

	dialog_->langModule->defaultencodingCB->setChecked(true);

	if (params.inputenc != "auto") {
		dialog_->langModule->defaultencodingCB->setChecked(false);
		if (params.inputenc == "default") {
			dialog_->langModule->encodingCO->setCurrentItem(0);
		} else {
			int i = 0;
			while (encodings[i]) {
				if (encodings[i] == params.inputenc)
					dialog_->langModule->encodingCO->setCurrentItem(i);
				++i;
			}
		}
	}

	// numbering
	dialog_->numberingModule->tocSL->setValue(params.tocdepth);
	dialog_->numberingModule->depthSL->setValue(params.secnumdepth);
	dialog_->updateNumbering();

	// bullets
	dialog_->bulletsModule->setBullet(0,params.user_defined_bullet(0));
	dialog_->bulletsModule->setBullet(1,params.user_defined_bullet(1));
	dialog_->bulletsModule->setBullet(2,params.user_defined_bullet(2));
	dialog_->bulletsModule->setBullet(3,params.user_defined_bullet(3));

	// packages
	QString text = toqstr(params.graphicsDriver);
	int nitem = dialog_->latexModule->psdriverCO->count();
	for (int n = 0; n < nitem ; ++n) {
		QString enc = tex_graphics[n];
		if (enc == text) {
			dialog_->latexModule->psdriverCO->setCurrentItem(n);
		}
	}


	dialog_->mathsModule->amsCB->setChecked(
		params.use_amsmath == BufferParams::AMS_ON);
	dialog_->mathsModule->amsautoCB->setChecked(
		params.use_amsmath == BufferParams::AMS_AUTO);

	switch (params.spacing().getSpace()) {
		case Spacing::Other: nitem = 3; break;
		case Spacing::Double: nitem = 2; break;
		case Spacing::Onehalf: nitem = 1; break;
		case Spacing::Default: case Spacing::Single: nitem = 0; break;
	}


	// layout
	dialog_->latexModule->classCO->setCurrentItem(params.textclass);

	dialog_->updateFontsize(controller().textClass().opt_fontsize(),
				params.fontsize);

	dialog_->updatePagestyle(controller().textClass().opt_pagestyle(),
				 params.pagestyle);

	for (int n = 0; tex_fonts[n][0]; ++n) {
		if (tex_fonts[n] == params.fonts) {
			dialog_->textLayoutModule->fontsCO->setCurrentItem(n);
			break;
		}
	}

	dialog_->textLayoutModule->lspacingCO->setCurrentItem(nitem);
	if (params.spacing().getSpace() == Spacing::Other) {
		dialog_->textLayoutModule->lspacingLE->setText(
			toqstr(tostr(params.spacing().getValue())));
		dialog_->setLSpacing(3);
	}

	if (params.paragraph_separation
	    == BufferParams::PARSEP_INDENT) {
		dialog_->textLayoutModule->indentRB->setChecked(true);
	} else {
		dialog_->textLayoutModule->skipRB->setChecked(true);
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
		dialog_->textLayoutModule->skipLengthCO->setCurrentItem(LyXLength(length).unit());
		dialog_->textLayoutModule->skipLE->setText(toqstr(tostr(LyXLength(length).value())));
		break;
	}
	default:
		skip = 0;
		break;
	}
	dialog_->textLayoutModule->skipCO->setCurrentItem(skip);
	dialog_->setSkip(skip);

	dialog_->textLayoutModule->twoColumnCB->setChecked(
		params.columns == 2);

	if (!params.options.empty()) {
		dialog_->latexModule->optionsLE->setText(
			toqstr(params.options));
	} else {
		dialog_->latexModule->optionsLE->setText("");
	}

	dialog_->floatModule->set(params.float_placement);

	// paper
	int const psize = params.papersize2;
	dialog_->pageLayoutModule->papersizeCO->setCurrentItem(psize);
	dialog_->setMargins(psize);
	dialog_->setCustomPapersize(psize);

	bool const landscape =
		params.orientation == ORIENTATION_LANDSCAPE;
	dialog_->pageLayoutModule->landscapeRB->setChecked(landscape);
	dialog_->pageLayoutModule->portraitRB->setChecked(!landscape);

	dialog_->pageLayoutModule->facingPagesCB->setChecked(
		params.sides == LyXTextClass::TwoSides);



	lengthToWidgets(dialog_->pageLayoutModule->paperwidthLE,
		dialog_->pageLayoutModule->paperwidthUnitCO, params.paperwidth, defaultUnit);

	lengthToWidgets(dialog_->pageLayoutModule->paperheightLE,
		dialog_->pageLayoutModule->paperheightUnitCO, params.paperheight, defaultUnit);

	// margins

	MarginsModuleBase * m = dialog_->marginsModule;

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

	// branches
	dialog_->updateBranchView();
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

	params.textclass = dialog_->latexModule->classCO->currentItem();
	params.useClassDefaults();
	update_contents();
}
