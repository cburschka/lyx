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

#ifdef __GNUG__
#pragma implementation
#endif

#include "gettext.h"

#include "ControlDocument.h"
#include "QDocument.h"
#include "QDocumentDialog.h"
#include "Qt2BC.h"

#include "language.h"
#include "helper_funcs.h" // getSecond()
#include "insets/insetquotes.h"
#include "frnt_lang.h"
#include "tex-strings.h" // tex_graphics
#include "support/lstrings.h" // tostr()
#include "support/filetools.h" // LibFileSearch()
#include "lyxtextclasslist.h"
#include "vspace.h"
#include "bufferparams.h"

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
	: base_class(_("Document Settings"))
{
	vector<frnt::LanguagePair> const langs = frnt::getLanguageData(false);
	lang_ = getSecond(langs);
}


void QDocument::build_dialog()
{
	dialog_.reset(new QDocumentDialog(this));

	// biblio
	dialog_->biblioModule->citeStyleCO->insertItem(_("Author-year"));
	dialog_->biblioModule->citeStyleCO->insertItem(_("Numerical"));
	dialog_->biblioModule->citeStyleCO->setCurrentItem(0);

	// language & quotes
	vector<frnt::LanguagePair> const langs = frnt::getLanguageData(false);
	vector<frnt::LanguagePair>::const_iterator lit  = langs.begin();
	vector<frnt::LanguagePair>::const_iterator lend = langs.end();
	for (; lit != lend; ++lit) {
		dialog_->langModule->languageCO->insertItem(
			lit->first.c_str());
	}

	dialog_->langModule->quoteStyleCO->insertItem(_("``text''"));
	dialog_->langModule->quoteStyleCO->insertItem(_("''text''"));
	dialog_->langModule->quoteStyleCO->insertItem(_(",,text``"));
	dialog_->langModule->quoteStyleCO->insertItem(_(",,text''"));
	dialog_->langModule->quoteStyleCO->insertItem(_("«text»"));
	dialog_->langModule->quoteStyleCO->insertItem(_("»text«"));

	// packages
	char const * items[] = {"default","auto","latin1","latin2",
			     "latin3","latin4","latin5","latin9",
			     "koi8-r","koi8-u","cp866","cp1251",
			     "iso88595","pt154",0};
	dialog_->packagesModule->encodingCO->insertStrList(items);

	dialog_->packagesModule->lspacingCO->insertItem(
		_("Single"), Spacing::Single);
	dialog_->packagesModule->lspacingCO->insertItem(
		_("OneHalf"), Spacing::Onehalf);
	dialog_->packagesModule->lspacingCO->insertItem(
		_("Double"), Spacing::Double);
	dialog_->packagesModule->lspacingCO->insertItem(
		_("Custom"), Spacing::Other);

	for (int n = 0; tex_graphics[n][0]; ++n) {
		QString enc = tex_graphics[n];
		dialog_->packagesModule->psdriverCO->insertItem(enc);
	}

	// paper
	char const * sizes[] = { 
		_("Default") , _("Custom") , _("US letter") , _("US legal")
		   , _("US executive") , _("A3") , _("A4") , _("A5")
		   , _("B3") , _("B4") , _("B5") };
	dialog_->paperModule->papersizeCO->insertStrList(sizes);

	// layout
	for (LyXTextClassList::const_iterator cit = textclasslist.begin();
	     cit != textclasslist.end(); ++cit) {
		dialog_->layoutModule->classCO->insertItem(cit->description().c_str());
	}

	for (int n = 0; tex_fonts[n][0]; ++n) {
		QString font = tex_fonts[n];
		dialog_->layoutModule->fontsCO->insertItem(font);
	}

	dialog_->layoutModule->fontsizeCO->insertItem(_("default"));
	dialog_->layoutModule->fontsizeCO->insertItem(_("10"));
	dialog_->layoutModule->fontsizeCO->insertItem(_("11"));
	dialog_->layoutModule->fontsizeCO->insertItem(_("12"));

	dialog_->layoutModule->skipCO->insertItem(_("Smallskip"));
	dialog_->layoutModule->skipCO->insertItem(_("Medskip"));
	dialog_->layoutModule->skipCO->insertItem(_("Bigskip"));
	dialog_->layoutModule->skipCO->insertItem(_("Length"));

	dialog_->layoutModule->pagestyleCO->insertItem(_("default"));
	dialog_->layoutModule->pagestyleCO->insertItem(_("empty"));
	dialog_->layoutModule->pagestyleCO->insertItem(_("plain"));
	dialog_->layoutModule->pagestyleCO->insertItem(_("headings"));
	dialog_->layoutModule->pagestyleCO->insertItem(_("fancy"));

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
		dialog_->preambleModule->preambleMLE->text().latin1();

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
	params.secnumdepth =
		dialog_->numberingModule->tocDepthSB->value();
	params.tocdepth =
		dialog_->numberingModule->sectionnrDepthSB->value();

	// bullets
	params.user_defined_bullets[0].setText(dialog_->bulletsModule->bullet1LE->text().latin1());
	params.user_defined_bullets[1].setText(dialog_->bulletsModule->bullet2LE->text().latin1());
	params.user_defined_bullets[2].setText(dialog_->bulletsModule->bullet3LE->text().latin1());
	params.user_defined_bullets[3].setText(dialog_->bulletsModule->bullet4LE->text().latin1());
	
	params.user_defined_bullets[0].setSize(dialog_->bulletsModule->bulletsize1CO->currentItem()-1);
	params.user_defined_bullets[1].setSize(dialog_->bulletsModule->bulletsize2CO->currentItem()-1);
	params.user_defined_bullets[2].setSize(dialog_->bulletsModule->bulletsize3CO->currentItem()-1);
	params.user_defined_bullets[3].setSize(dialog_->bulletsModule->bulletsize4CO->currentItem()-1);

	// packages
	switch (dialog_->packagesModule->lspacingCO->currentItem()) {
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
				   dialog_->packagesModule->
				   lspacingLE->text().toFloat()
				   );
		break;
	}

	params.graphicsDriver =
		dialog_->packagesModule->psdriverCO->currentText().latin1();

	params.use_amsmath =
		dialog_->packagesModule->amsCB->isChecked();

	params.inputenc =
		dialog_->packagesModule->encodingCO->currentText().latin1();

	// layout
	params.textclass =
		dialog_->layoutModule->classCO->currentItem();

	//bool succes = controller().classApply();

	params.fonts =
		dialog_->layoutModule->fontsCO->currentText().latin1();

	params.fontsize =
		dialog_->layoutModule->fontsizeCO->currentText().latin1();

	params.pagestyle =
		dialog_->layoutModule->pagestyleCO->currentText().latin1();

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
		dialog_->layoutModule->optionsLE->text().latin1();

	params.float_placement =
		dialog_->layoutModule->floatPlacementLE->text().latin1();

	// paper
	params.papersize2 =
		dialog_->paperModule->papersizeCO->currentItem();

	params.paperwidth =
		LyXLength(dialog_->paperModule->paperwidthLE->text().toDouble(),
			  dialog_->paperModule->paperwidthUnitCO->currentLengthItem()
			).asString();

	params.paperheight =
		LyXLength(dialog_->paperModule->paperheightLE->text().toDouble(),
			  dialog_->paperModule->paperheightUnitCO->currentLengthItem()
			).asString();

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
		(dialog_->marginsModule->marginCO->currentItem() == 1);

	int margin = dialog_->marginsModule->marginCO->currentItem();
	if (margin > 0) {
		margin = margin - 1;
	}
	params.paperpackage = char(margin);

	MarginsModuleBase const * m(dialog_->marginsModule);
 
	params.leftmargin =
		LyXLength(m->innerLE->text().toDouble(),
			  m->innerUnit->currentLengthItem()
			  ).asString();

	params.topmargin =
		LyXLength(m->topLE->text().toDouble(),
			  m->topUnit->currentLengthItem()
			  ).asString();

	params.rightmargin =
		LyXLength(m->outerLE->text().toDouble(),
			  m->outerUnit->currentLengthItem()
			  ).asString();

	params.bottommargin =
		LyXLength(m->bottomLE->text().toDouble(),
			  m->bottomUnit->currentLengthItem()
			  ).asString();

	params.headheight =
		LyXLength(m->headheightLE->text().toDouble(),
			  m->headheightUnit->currentLengthItem()
			  ).asString();

	params.headsep =
		LyXLength(m->headsepLE->text().toDouble(),
			  m->headsepUnit->currentLengthItem()
			  ).asString();

	params.footskip =
		LyXLength(m->footskipLE->text().toDouble(),
			  m->footskipUnit->currentLengthItem()
			  ).asString();
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

	// preamble
	QString preamble = params.preamble.c_str();
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

	// numbering
	dialog_->numberingModule->tocDepthSB->setValue(
		params.secnumdepth);
	dialog_->numberingModule->sectionnrDepthSB->setValue(
		params.tocdepth);

	// bullets
	QString s;
	s = params.user_defined_bullets[0].getText().c_str();
	dialog_->bulletsModule->bullet1LE->setText(s);
	s = params.user_defined_bullets[1].getText().c_str();
	dialog_->bulletsModule->bullet2LE->setText(s);
	s = params.user_defined_bullets[2].getText().c_str();
	dialog_->bulletsModule->bullet3LE->setText(s);
	s = params.user_defined_bullets[3].getText().c_str();
	dialog_->bulletsModule->bullet4LE->setText(s);
	
	dialog_->bulletsModule->bulletsize1CO->setCurrentItem(
		params.user_defined_bullets[0].getSize() + 1);
	dialog_->bulletsModule->bulletsize2CO->setCurrentItem(
		params.user_defined_bullets[1].getSize() + 1);
	dialog_->bulletsModule->bulletsize3CO->setCurrentItem(
		params.user_defined_bullets[2].getSize() + 1);
	dialog_->bulletsModule->bulletsize4CO->setCurrentItem(
		params.user_defined_bullets[3].getSize() + 1);
	
	// packages
	char const * enc[] = { 
		"default" , "auto" , "latin1" , "latin2" , "latin3" ,
		"latin4" , "latin5" , "latin9" , "koi8-r" , "koi8-u" ,
		"cp866" , "cp1251" , "iso88595" , "pt154" };
	for (size_t i = 0; i < sizeof(enc)/sizeof(char *); ++i) {
		if (params.inputenc == enc[i])
			dialog_->packagesModule->encodingCO->setCurrentItem(i);
	}

	QString text = params.graphicsDriver.c_str();
	int nitem = dialog_->packagesModule->psdriverCO->count();
	for (int n = 0; n < nitem ; ++n) {
		QString enc = tex_graphics[n];
		if (enc == text) {
			dialog_->packagesModule->psdriverCO->setCurrentItem(n);
		}
	}


	dialog_->packagesModule->amsCB->setChecked(
		params.use_amsmath);

	// FIXME: this is wrong
	// QComboBox::setCurrentItem: (lspacingCO) Index 4 out of range
 
	dialog_->packagesModule->lspacingCO->
		setCurrentItem(params.spacing.getSpace());
	if (params.spacing.getSpace() == Spacing::Other) {
		dialog_->packagesModule->lspacingLE->setText(
			tostr(params.spacing.getValue()).c_str());
		dialog_->setLSpacing(3);
	}

	// layout
	for (int n = 0; n<dialog_->layoutModule->classCO->count(); ++n) {
		if (dialog_->layoutModule->classCO->text(n) ==
		    controller().textClass().description().c_str()) {
			dialog_->layoutModule->classCO->setCurrentItem(n);
			break;
		}
	}

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
		dialog_->layoutModule->skipLE->setText(tostr(LyXLength(length).value()).c_str());
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
			params.options.c_str());
	} else {
		dialog_->layoutModule->optionsLE->setText("");
	}

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

	dialog_->paperModule->paperwidthUnitCO->setCurrentItem(
		LyXLength(params.paperwidth).unit());

	dialog_->paperModule->paperwidthLE->setText(
		tostr(LyXLength(params.paperwidth).value()).c_str());

	dialog_->paperModule->paperheightUnitCO->setCurrentItem(
		LyXLength(params.paperheight).unit());

	dialog_->paperModule->paperheightLE->setText(
		tostr(LyXLength(params.paperheight).value()).c_str());

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

	m->topUnit->setCurrentItem(LyXLength(params.topmargin).unit());
	m->topLE->setText(tostr(LyXLength(params.topmargin).value()).c_str());

	m->bottomUnit->setCurrentItem(LyXLength(params.bottommargin).unit());
	m->bottomLE->setText(tostr(LyXLength(params.bottommargin).value()).c_str());

	m->innerUnit->setCurrentItem(LyXLength(params.leftmargin).unit());
	m->innerLE->setText(tostr(LyXLength(params.leftmargin).value()).c_str());

	m->outerUnit->setCurrentItem(LyXLength(params.rightmargin).unit());
	m->outerLE->setText(tostr(LyXLength(params.rightmargin).value()).c_str());

	m->headheightUnit->setCurrentItem(LyXLength(params.headheight).unit());
	m->headheightLE->setText(tostr(LyXLength(params.headheight).value()).c_str());

	m->headsepUnit->setCurrentItem(LyXLength(params.headsep).unit());
	m->headsepLE->setText(tostr(LyXLength(params.headsep).value()).c_str());

	m->footskipUnit->setCurrentItem(LyXLength(params.footskip).unit());
	m->footskipLE->setText(tostr(LyXLength(params.footskip).value()).c_str());
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
