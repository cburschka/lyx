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

	/* biblio */
	dialog_->biblioModule->citeStyleCO->insertItem(_("Author-year"));
	dialog_->biblioModule->citeStyleCO->insertItem(_("Numerical"));
	dialog_->biblioModule->citeStyleCO->setCurrentItem(0);

	/* language & quotes */
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

	/* packages */
	char const * items[] = {"default","auto","latin1","latin2",
			     "latin3","latin4","latin5","latin9",
			     "koi8-r","koi8-u","cp866","cp1251",
			     "iso88595","pt154",0};
	dialog_->packagesModule->encodingCO->insertStrList(items);
	
	dialog_->packagesModule->lspacingCO->insertItem(
		_("Single"),Spacing::Single);
	dialog_->packagesModule->lspacingCO->insertItem(
		_("OneHalf"),Spacing::Onehalf);
	dialog_->packagesModule->lspacingCO->insertItem(
		_("Double"),Spacing::Double);
	dialog_->packagesModule->lspacingCO->insertItem(
		_("Custom"),Spacing::Other);

	int n;
	for (n=0; tex_graphics[n][0]; ++n) {
		QString enc = tex_graphics[n];
		dialog_->packagesModule->psdriverCO->insertItem(enc);
	}

	/* paper */
	QStringList papersize_;
	papersize_ << _("Default") << _("Custom") << _("US letter") << _("US legal")
		   << _("US executive") << _("A3") << _("A4") << _("A5")
		   << _("B3") << _("B4") << _("B5");
	dialog_->paperModule->papersizeCO->insertStringList(papersize_);

	/* layout */
	for (LyXTextClassList::const_iterator cit = textclasslist.begin();
	     cit != textclasslist.end(); ++cit) {
		dialog_->layoutModule->classCO->insertItem(cit->description().c_str());
	}

	for (n=0; tex_fonts[n][0]; ++n) {
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

	/* margins */
	dialog_->setMargins(0);

	string const bmtablefile = LibFileSearch("images", "standard", "xpm");
	QString s = bmtablefile.c_str();
   qWarning("%d",s);
   
	// Manage the restore, ok, apply, restore and cancel/close buttons
	bc().setOK(dialog_->okPB);
	bc().setApply(dialog_->applyPB);
	bc().setCancel(dialog_->cancelPB);
	bc().setRestore(dialog_->restorePB);
}


void QDocument::apply()
{
	/* preamble */
	controller().params().preamble =
		dialog_->preambleModule->preambleMLE->text().latin1();
	
	/* biblio */
	controller().params().use_natbib =
		dialog_->biblioModule->natbibCB->isChecked();
	controller().params().use_numerical_citations  =
		dialog_->biblioModule->citeStyleCO->currentItem();

	/* language & quotes */
	if (dialog_->langModule->singleQuoteRB->isChecked())
		controller().params().quotes_times = InsetQuotes::SingleQ;
	else
		controller().params().quotes_times = InsetQuotes::DoubleQ;

	
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
	controller().params().quotes_language = lga;

	int const pos = dialog_->langModule->languageCO->currentItem();
	controller().params().language = languages.getLanguage(lang_[pos]);

	/* numbering */
	controller().params().secnumdepth =
		dialog_->numberingModule->tocDepthSB->value();
	controller().params().tocdepth =
		dialog_->numberingModule->sectionnrDepthSB->value();

	/* packages */
	switch (dialog_->packagesModule->lspacingCO->currentItem()) {
	case 0:
		controller().params().spacing.set(Spacing::Single);
		break;
	case 1:
		controller().params().spacing.set(Spacing::Onehalf);
		break;
	case 2:
		controller().params().spacing.set(Spacing::Double);
		break;
	case 3:
		controller().params().spacing.set(Spacing::Other,
				   dialog_->packagesModule->
				   lspacingLE->text().toFloat()
				   );
		break;
	}
	
	controller().params().graphicsDriver =
		dialog_->packagesModule->psdriverCO->currentText().latin1();
	
	controller().params().use_amsmath =
		dialog_->packagesModule->amsCB->isChecked();
	
	controller().params().inputenc =
		dialog_->packagesModule->encodingCO->currentText().latin1();

	/* layout */
	controller().params().textclass =
		dialog_->layoutModule->classCO->currentItem();

	//bool succes = controller().classApply();
	
	controller().params().fonts =
		dialog_->layoutModule->fontsCO->currentText().latin1();
	
	controller().params().fontsize =
		dialog_->layoutModule->fontsizeCO->currentText().latin1();
	
	controller().params().pagestyle =
		dialog_->layoutModule->pagestyleCO->currentText().latin1();
	
	if (dialog_->layoutModule->indentRB->isChecked())
		controller().params().paragraph_separation = BufferParams::PARSEP_INDENT;
	else
		controller().params().paragraph_separation = BufferParams::PARSEP_SKIP;

	switch (dialog_->layoutModule->skipCO->currentItem()) {
	case 0:
		controller().params().setDefSkip(VSpace(VSpace::SMALLSKIP));
		break;
	case 1:
		controller().params().setDefSkip(VSpace(VSpace::MEDSKIP));
		break;
	case 2:
		controller().params().setDefSkip(VSpace(VSpace::BIGSKIP));
		break;
	case 3:
	{
		LyXLength::UNIT unit =
			dialog_->layoutModule->skipLengthCO->
			currentLengthItem();
		double length =
			dialog_->layoutModule->skipLE->text().toDouble();
		VSpace vs = VSpace(LyXGlueLength(LyXLength(length,unit)));
		controller().params().setDefSkip(vs);
		break;
	}
	default:
		// DocumentDefskipCB assures that this never happens
		controller().params().setDefSkip(VSpace(VSpace::MEDSKIP));
		break;
	}

	controller().params().options =
		dialog_->layoutModule->optionsLE->text().latin1();
	
	controller().params().float_placement =
		dialog_->layoutModule->floatPlacementLE->text().latin1();

	/* paper */
	controller().params().papersize2 =
		dialog_->paperModule->papersizeCO->currentItem();
	
	controller().params().paperwidth =
		LyXLength(dialog_->paperModule->paperwidthLE->text().toDouble(),
			  dialog_->paperModule->paperwidthUnitCO->currentLengthItem()
			).asString();

	controller().params().paperheight =
		LyXLength(dialog_->paperModule->paperheightLE->text().toDouble(),
			  dialog_->paperModule->paperheightUnitCO->currentLengthItem()
			).asString();

	if (dialog_->paperModule->twoColumnCB->isChecked())
		controller().params().columns = 2;
	else
		controller().params().columns = 1;
	
	if (dialog_->paperModule->facingPagesCB->isChecked())
		controller().params().sides = LyXTextClass::TwoSides;
	else
		controller().params().sides = LyXTextClass::OneSide;
	
	if (dialog_->paperModule->landscapeRB->isChecked())
		controller().params().orientation = BufferParams::ORIENTATION_LANDSCAPE;
	else
		controller().params().orientation = BufferParams::ORIENTATION_PORTRAIT;

	/* margins */
	controller().params().use_geometry =
		dialog_->marginsModule->marginCO->currentItem()==1;

	int margin = dialog_->marginsModule->marginCO->currentItem();
	if (margin>0) {
		margin = margin - 1;
	}
	controller().params().paperpackage = char(margin);
	
	controller().params().leftmargin =
		LyXLength(dialog_->marginsModule->innerLE->text().toDouble(),
			  dialog_->marginsModule->innerUnit->currentLengthItem()
			  ).asString();
	
	controller().params().topmargin =
		LyXLength(dialog_->marginsModule->topLE->text().toDouble(),
			  dialog_->marginsModule->topUnit->currentLengthItem()
			  ).asString();


	controller().params().rightmargin =
		LyXLength(dialog_->marginsModule->outerLE->text().toDouble(),
			  dialog_->marginsModule->outerUnit->currentLengthItem()
			  ).asString();


	controller().params().bottommargin =
		LyXLength(dialog_->marginsModule->bottomLE->text().toDouble(),
			  dialog_->marginsModule->bottomUnit->currentLengthItem()
			  ).asString();


	controller().params().headheight =
		LyXLength(dialog_->marginsModule->headheightLE->text().toDouble(),
			  dialog_->marginsModule->headheightUnit->currentLengthItem()
			  ).asString();


	controller().params().headsep =
		LyXLength(dialog_->marginsModule->headsepLE->text().toDouble(),
			  dialog_->marginsModule->headsepUnit->currentLengthItem()
			  ).asString();


	controller().params().footskip =
		LyXLength(dialog_->marginsModule->footskipLE->text().toDouble(),
			  dialog_->marginsModule->footskipUnit->currentLengthItem()
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

	/* preamble */
	QString preamble = controller().params().preamble.c_str();
	dialog_->preambleModule->preambleMLE->setText(preamble);

	/* biblio */
	dialog_->biblioModule->natbibCB->setChecked(
		controller().params().use_natbib);
	dialog_->biblioModule->citeStyleCO->setCurrentItem(
		controller().params().use_numerical_citations ? 1 : 0);
	
	/* language & quotes */
	dialog_->langModule->singleQuoteRB->setChecked(
		controller().params().quotes_times == InsetQuotes::SingleQ);
	dialog_->langModule->doubleQuoteRB->setChecked(
		controller().params().quotes_times == InsetQuotes::DoubleQ);
	
	int const pos = int(findPos(lang_,
				    controller().params().language->lang()));
	dialog_->langModule->languageCO->setCurrentItem(pos);

	dialog_->langModule->quoteStyleCO->setCurrentItem(
		controller().params().quotes_language);

	/* numbering */
	dialog_->numberingModule->tocDepthSB->setValue(
		controller().params().secnumdepth);
	dialog_->numberingModule->sectionnrDepthSB->setValue(
		controller().params().tocdepth);

	/* packages */
	QStringList enc;
	enc <<  "default" << "auto" << "latin1" << "latin2" << "latin3" <<
		"latin4" << "latin5" << "latin9" << "koi8-r" << "koi8-u" <<
		"cp866" << "cp1251" << "iso88595" << "pt154";
	int pos2 = 0;
	for (QStringList::Iterator it = enc.begin();
	     it!=enc.end(); ++it) {
		if (*it==controller().params().inputenc.c_str()) {
			dialog_->packagesModule->encodingCO->setCurrentItem(pos2);
		}
		++pos2;
	}
	
	
	QString text = controller().params().graphicsDriver.c_str();
	int nitem = dialog_->packagesModule->psdriverCO->count();
	for (int n=0; n<nitem ; ++n) {
		QString enc = tex_graphics[n];
		if (enc==text) {
			dialog_->packagesModule->psdriverCO->setCurrentItem(n);
		}
	}
	
	
	dialog_->packagesModule->amsCB->setChecked(
		controller().params().use_amsmath);

	dialog_->packagesModule->lspacingCO->
		setCurrentItem(controller().params().spacing.getSpace());
	if (controller().params().spacing.getSpace()==
	    Spacing::Other) {
		dialog_->packagesModule->lspacingLE->setText(
			tostr(controller().params().spacing.getValue()).c_str());
		dialog_->setLSpacing(3);
	}

	/* layout */
	for (int n = 0; n<dialog_->layoutModule->classCO->count(); ++n) {
		if (dialog_->layoutModule->classCO->text(n)==
		    controller().textClass().description().c_str()) {
			dialog_->layoutModule->classCO->setCurrentItem(n);
			break;
		}
	}

	dialog_->updateFontsize(controller().textClass().opt_fontsize(),
				controller().params().fontsize);
	
	dialog_->updatePagestyle(controller().textClass().opt_pagestyle(),
				 controller().params().pagestyle);
	
	
	for (int n=0; tex_fonts[n][0]; ++n) {
		if (tex_fonts[n]==controller().params().fonts) {
			dialog_->layoutModule->fontsCO->setCurrentItem(n);
			break;
		}
	}

	if (controller().params().paragraph_separation
	    == BufferParams::PARSEP_INDENT) {
		dialog_->layoutModule->indentRB->setChecked(true);
	} else {
		dialog_->layoutModule->skipRB->setChecked(true);
	}
	
	int skip = 0;
	switch (controller().params().getDefSkip().kind()) {
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
		string const length = controller().params().getDefSkip().asLyXCommand();
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

	if (!controller().params().options.empty()) {
		dialog_->layoutModule->optionsLE->setText(
			controller().params().options.c_str());
	} else {
		dialog_->layoutModule->optionsLE->setText("");
	}
	
	/* paper */
	int const psize = controller().params().papersize2;
	dialog_->paperModule->papersizeCO->setCurrentItem(psize);
	dialog_->setMargins(psize);
	dialog_->setCustomPapersize(psize);

	bool const landscape =
		controller().params().orientation == BufferParams::ORIENTATION_LANDSCAPE;
	dialog_->paperModule->landscapeRB->setChecked(landscape);
	dialog_->paperModule->portraitRB->setChecked(!landscape);

	dialog_->paperModule->facingPagesCB->setChecked(
		controller().params().sides == LyXTextClass::TwoSides);
	
	dialog_->paperModule->twoColumnCB->setChecked(
		controller().params().columns == 2);
	
	dialog_->paperModule->paperwidthUnitCO->setCurrentItem(
		LyXLength(controller().params().paperwidth).unit());
	
	dialog_->paperModule->paperwidthLE->setText(
		tostr(LyXLength(controller().params().paperwidth).value()).c_str());
	
	dialog_->paperModule->paperheightUnitCO->setCurrentItem(
		LyXLength(controller().params().paperheight).unit());

	dialog_->paperModule->paperheightLE->setText(
		tostr(LyXLength(controller().params().paperheight).value()).c_str());
		
	/* margins */
	int item = controller().params().paperpackage;
	if (controller().params().use_geometry) {
		item = 1;
	} else if (item>0) {
		item = item + 1;
	}
	dialog_->marginsModule->marginCO->setCurrentItem(item);
	dialog_->setCustomMargins(item);

	dialog_->marginsModule->topUnit->setCurrentItem(
		LyXLength(controller().params().topmargin).unit());
	dialog_->marginsModule->topLE->setText(
		tostr(LyXLength(controller().params().topmargin).value()).c_str());
	
	dialog_->marginsModule->bottomUnit->setCurrentItem(
		LyXLength(controller().params().bottommargin).unit());
	dialog_->marginsModule->bottomLE->setText(
		tostr(LyXLength(controller().params().bottommargin).value()).c_str());
	
	dialog_->marginsModule->innerUnit->setCurrentItem(
		LyXLength(controller().params().leftmargin).unit());
	dialog_->marginsModule->innerLE->setText(
		tostr(LyXLength(controller().params().leftmargin).value()).c_str());
	
	dialog_->marginsModule->outerUnit->setCurrentItem(
		LyXLength(controller().params().rightmargin).unit());
	dialog_->marginsModule->outerLE->setText(
		tostr(LyXLength(controller().params().rightmargin).value()).c_str());
	
	dialog_->marginsModule->headheightUnit->setCurrentItem(
		LyXLength(controller().params().headheight).unit());
	dialog_->marginsModule->headheightLE->setText(
		tostr(LyXLength(controller().params().headheight).value()).c_str());
	
	dialog_->marginsModule->headsepUnit->setCurrentItem(
		LyXLength(controller().params().headsep).unit());
	dialog_->marginsModule->headsepLE->setText(
		tostr(LyXLength(controller().params().headsep).value()).c_str());
	
	dialog_->marginsModule->footskipUnit->setCurrentItem(
		LyXLength(controller().params().footskip).unit());
	dialog_->marginsModule->footskipLE->setText(
		tostr(LyXLength(controller().params().footskip).value()).c_str());
	
	
}







