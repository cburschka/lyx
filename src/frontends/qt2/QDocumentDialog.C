/**
 * \file QDocumentDialog.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Edwin Leuven
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "bufferparams.h"
#include "debug.h"
#include "lyxrc.h"

#include "controllers/ControlDocument.h"

#include "support/lstrings.h"

#include "QDocument.h"
#include "QDocumentDialog.h"

#include "floatplacement.h"
#include "lengthcombo.h"
#include "panelstack.h"
#include "qt_helpers.h"

#include <qlabel.h>
#include <qmultilineedit.h>
#include <qlineedit.h>
#include <qlistview.h>
#include <qpushbutton.h>
#include <qradiobutton.h>
#include <qcheckbox.h>
#include <qslider.h>
#include <qpixmap.h>
#include <qcolor.h>
#include <qcolordialog.h>

using lyx::support::token;

using std::string;


QDocumentDialog::QDocumentDialog(QDocument * form)
	: QDocumentDialogBase(0, 0, false, 0), form_(form)
{
	connect(okPB, SIGNAL(clicked()),
		form, SLOT(slotOK()));
	connect(applyPB, SIGNAL(clicked()),
		form, SLOT(slotApply()));
	connect(closePB, SIGNAL(clicked()),
		form, SLOT(slotClose()));
	connect(restorePB, SIGNAL(clicked()),
		form, SLOT(slotRestore()));

	textLayoutModule = new TextLayoutModuleBase(this);
	pageLayoutModule = new PageLayoutModuleBase(this);
	marginsModule = new MarginsModuleBase(this);
	langModule = new LanguageModuleBase(this);
	bulletsModule = new BulletsModule(this);
	numberingModule = new NumberingModuleBase(this);
	biblioModule = new BiblioModuleBase(this);
	mathsModule = new MathsModuleBase(this);
	floatModule = new FloatPlacement(this, "floatplacement");
	latexModule = new LaTeXModuleBase(this);
	branchesModule = new BranchesModuleBase(this);
	preambleModule = new PreambleModuleBase(this);

	docPS->addPanel(latexModule, _("Document Class"));
	docPS->addPanel(textLayoutModule, _("Text Layout"));
	docPS->addPanel(pageLayoutModule, _("Page Layout"));
	docPS->addPanel(marginsModule, _("Page Margins"));
	docPS->addPanel(langModule, _("Language"));
	docPS->addPanel(numberingModule, _("Numbering & TOC"));
	docPS->addPanel(biblioModule, _("Bibliography"));
	docPS->addPanel(mathsModule, _("Math options"));
	docPS->addPanel(floatModule, _("Float Placement"));
	docPS->addPanel(bulletsModule, _("Bullets"));
	docPS->addPanel(branchesModule, _("Branches"));
	docPS->addPanel(preambleModule, _("LaTeX Preamble"));
	docPS->setCurrentPanel(_("Document Class"));

	// preamble
	connect(preambleModule->preambleMLE, SIGNAL(textChanged()), this, SLOT(change_adaptor()));
	// biblio
	connect(biblioModule->citeDefaultRB, SIGNAL(toggled(bool)), this, SLOT(change_adaptor()));
	connect(biblioModule->citeNatbibRB, SIGNAL(toggled(bool)), this, SLOT(change_adaptor()));
	connect(biblioModule->citeStyleCO, SIGNAL(activated(int)), this, SLOT(change_adaptor()));
	connect(biblioModule->citeJurabibRB, SIGNAL(toggled(bool)), this, SLOT(change_adaptor()));
	connect(biblioModule->bibtopicCB, SIGNAL(toggled(bool)), this, SLOT(change_adaptor()));
	// language & quote
	connect(langModule->languageCO, SIGNAL(activated(int)), this, SLOT(change_adaptor()));
	connect(langModule->defaultencodingCB, SIGNAL(toggled(bool)), this, SLOT(change_adaptor()));
	connect(langModule->encodingCO, SIGNAL(activated(int)), this, SLOT(change_adaptor()));
	connect(langModule->quoteStyleCO, SIGNAL(activated(int)), this, SLOT(change_adaptor()));
	// numbering
	connect(numberingModule->depthSL, SIGNAL(valueChanged(int)), this, SLOT(change_adaptor()));
	connect(numberingModule->tocSL, SIGNAL(valueChanged(int)), this, SLOT(change_adaptor()));
	connect(numberingModule->depthSL, SIGNAL(valueChanged(int)), this, SLOT(updateNumbering()));
	connect(numberingModule->tocSL, SIGNAL(valueChanged(int)), this, SLOT(updateNumbering()));
	numberingModule->tocLV->setSorting(-1);
	// maths
	connect(mathsModule->amsCB, SIGNAL(toggled(bool)), this, SLOT(change_adaptor()));
	connect(mathsModule->amsautoCB, SIGNAL(toggled(bool)), this, SLOT(change_adaptor()));
	// float
	connect(floatModule, SIGNAL(changed()), this, SLOT(change_adaptor()));
	// latex class
	connect(latexModule->classCO, SIGNAL(activated(int)), this, SLOT(change_adaptor()));
	connect(latexModule->optionsLE, SIGNAL(textChanged(const QString&)), this, SLOT(change_adaptor()));
	connect(latexModule->psdriverCO, SIGNAL(activated(int)), this, SLOT(change_adaptor()));
	connect(latexModule->classCO, SIGNAL(activated(int)), this, SLOT(classChanged()));
	// text layout
	connect(textLayoutModule->fontsCO, SIGNAL(activated(int)), this, SLOT(change_adaptor()));
	connect(textLayoutModule->fontsizeCO, SIGNAL(activated(int)), this, SLOT(change_adaptor()));
	connect(textLayoutModule->lspacingCO, SIGNAL(activated(int)), this, SLOT(change_adaptor()));
	connect(textLayoutModule->lspacingCO, SIGNAL(activated(int)), this, SLOT(setLSpacing(int)));
	connect(textLayoutModule->lspacingLE, SIGNAL(textChanged(const QString&)), this, SLOT(change_adaptor()));
	connect(textLayoutModule->skipRB, SIGNAL(toggled(bool)), this, SLOT(change_adaptor()));
	connect(textLayoutModule->indentRB, SIGNAL(toggled(bool)), this, SLOT(change_adaptor()));
	connect(textLayoutModule->skipCO, SIGNAL(activated(int)), this, SLOT(change_adaptor()));
	connect(textLayoutModule->skipLE, SIGNAL(textChanged(const QString&)), this, SLOT(change_adaptor()));
	connect(textLayoutModule->skipLengthCO, SIGNAL(activated(int)), this, SLOT(change_adaptor()));
	connect(textLayoutModule->skipCO, SIGNAL(activated(int)), this, SLOT(setSkip(int)));
	connect(textLayoutModule->skipRB, SIGNAL(toggled(bool)), this, SLOT(enableSkip(bool)));
	connect(textLayoutModule->twoColumnCB, SIGNAL(toggled(bool)), this, SLOT(change_adaptor()));

	// margins
	connect(marginsModule->marginCO, SIGNAL(activated(int)), this, SLOT(setCustomMargins(int)));
	connect(marginsModule->marginCO, SIGNAL(activated(int)), this, SLOT(change_adaptor()));
	connect(marginsModule->topLE, SIGNAL(textChanged(const QString&)), this, SLOT(change_adaptor()));
	connect(marginsModule->topUnit, SIGNAL(activated(int)), this, SLOT(change_adaptor()));
	connect(marginsModule->bottomLE, SIGNAL(textChanged(const QString&)), this, SLOT(change_adaptor()));
	connect(marginsModule->bottomUnit, SIGNAL(activated(int)), this, SLOT(change_adaptor()));
	connect(marginsModule->innerLE, SIGNAL(textChanged(const QString&)), this, SLOT(change_adaptor()));
	connect(marginsModule->innerUnit, SIGNAL(activated(int)), this, SLOT(change_adaptor()));
	connect(marginsModule->outerLE, SIGNAL(textChanged(const QString&)), this, SLOT(change_adaptor()));
	connect(marginsModule->outerUnit, SIGNAL(activated(int)), this, SLOT(change_adaptor()));
	connect(marginsModule->headheightLE, SIGNAL(textChanged(const QString&)), this, SLOT(change_adaptor()));
	connect(marginsModule->headheightUnit, SIGNAL(activated(int)), this, SLOT(change_adaptor()));
	connect(marginsModule->headsepLE, SIGNAL(textChanged(const QString&)), this, SLOT(change_adaptor()));
	connect(marginsModule->headsepUnit, SIGNAL(activated(int)), this, SLOT(change_adaptor()));
	connect(marginsModule->footskipLE, SIGNAL(textChanged(const QString&)), this, SLOT(change_adaptor()));
	connect(marginsModule->footskipUnit, SIGNAL(activated(int)), this, SLOT(change_adaptor()));

	// page layout
	connect(pageLayoutModule->papersizeCO, SIGNAL(activated(int)), this, SLOT(setMargins(int)));
	connect(pageLayoutModule->papersizeCO, SIGNAL(activated(int)), this, SLOT(setCustomPapersize(int)));
	connect(pageLayoutModule->papersizeCO, SIGNAL(activated(int)), this, SLOT(setCustomPapersize(int)));
	connect(pageLayoutModule->portraitRB, SIGNAL(toggled(bool)), this, SLOT(portraitChanged()));
	connect(pageLayoutModule->papersizeCO, SIGNAL(activated(int)), this, SLOT(change_adaptor()));
	connect(pageLayoutModule->paperheightLE, SIGNAL(textChanged(const QString&)), this, SLOT(change_adaptor()));
	connect(pageLayoutModule->paperwidthLE, SIGNAL(textChanged(const QString&)), this, SLOT(change_adaptor()));
	connect(pageLayoutModule->paperwidthUnitCO, SIGNAL(activated(int)), this, SLOT(change_adaptor()));
	connect(pageLayoutModule->paperheightUnitCO, SIGNAL(activated(int)), this, SLOT(change_adaptor()));
	connect(pageLayoutModule->portraitRB, SIGNAL(toggled(bool)), this, SLOT(change_adaptor()));
	connect(pageLayoutModule->landscapeRB, SIGNAL(toggled(bool)), this, SLOT(change_adaptor()));
	connect(pageLayoutModule->facingPagesCB, SIGNAL(toggled(bool)), this, SLOT(change_adaptor()));
	connect(pageLayoutModule->pagestyleCO, SIGNAL(activated(int)), this, SLOT(change_adaptor()));

	// bullets
	connect(bulletsModule, SIGNAL(changed()), this, SLOT(change_adaptor()));

	// branches
	connect(branchesModule->addBranchPB, SIGNAL(pressed()), this, SLOT(addBranchPressed()));
	connect(branchesModule->removePB, SIGNAL(pressed()), this, SLOT(deleteBranchPressed()));
	connect(branchesModule->activatePB, SIGNAL(pressed()), this, SLOT(toggleBranchPressed()));
	connect(branchesModule->branchesLV, SIGNAL(doubleClicked(QListViewItem *)), this,
		SLOT(branchDoubleClicked(QListViewItem *)));
	connect(branchesModule->colorPB, SIGNAL(clicked()), this, SLOT(toggleBranchColor()));
	branchesModule->branchesLV->setSorting(0);
}


QDocumentDialog::~QDocumentDialog()
{
}


void QDocumentDialog::showPreamble()
{
	docPS->setCurrentPanel(_("LaTeX Preamble"));
}


void QDocumentDialog::saveDefaultClicked()
{
	form_->saveDocDefault();
}


void QDocumentDialog::useDefaultsClicked()
{
    form_->useClassDefaults();
}


void QDocumentDialog::change_adaptor()
{
	form_->changed();
}


void QDocumentDialog::closeEvent(QCloseEvent * e)
{
	form_->slotWMHide();
	e->accept();
}


void QDocumentDialog::setLSpacing(int item)
{
	textLayoutModule->lspacingLE->setEnabled(item == 3);
}


void QDocumentDialog::setSkip(int item)
{
	bool const enable = (item == 3);
	textLayoutModule->skipLE->setEnabled(enable);
	textLayoutModule->skipLengthCO->setEnabled(enable);
}


void QDocumentDialog::enableSkip(bool skip)
{
	textLayoutModule->skipCO->setEnabled(skip);
	textLayoutModule->skipLE->setEnabled(skip);
	textLayoutModule->skipLengthCO->setEnabled(skip);
	if (skip)
		setSkip(textLayoutModule->skipCO->currentItem());
}

void QDocumentDialog::portraitChanged()
{
	setMargins(pageLayoutModule->papersizeCO->currentItem());
}

void QDocumentDialog::setMargins(int papersize)
{
	int olditem = marginsModule->marginCO->currentItem();
	marginsModule->marginCO->clear();
	marginsModule->marginCO->insertItem(qt_("Default"));
	marginsModule->marginCO->insertItem(qt_("Custom"));
	bool a4size = (papersize == 6 || papersize == 0
			&& lyxrc.default_papersize == PAPER_A4PAPER);
	if (a4size && pageLayoutModule->portraitRB->isChecked()) {
		marginsModule->marginCO->insertItem(qt_("Small margins"));
		marginsModule->marginCO->insertItem(qt_("Very small margins"));
		marginsModule->marginCO->insertItem(qt_("Very wide margins"));
	} else if (olditem > 1) {
		olditem = 0;
	}
	marginsModule->marginCO->setCurrentItem(olditem);
	setCustomMargins(olditem);
}


void QDocumentDialog::setCustomPapersize(int papersize)
{
	bool const custom = (papersize == 1);

	pageLayoutModule->paperwidthL->setEnabled(custom);
	pageLayoutModule->paperwidthLE->setEnabled(custom);
	pageLayoutModule->paperwidthUnitCO->setEnabled(custom);
	pageLayoutModule->paperheightL->setEnabled(custom);
	pageLayoutModule->paperheightLE->setEnabled(custom);
	pageLayoutModule->paperheightLE->setFocus();
	pageLayoutModule->paperheightUnitCO->setEnabled(custom);
}


void QDocumentDialog::setCustomMargins(int margin)
{
	bool const custom = (margin == 1);

	marginsModule->topL->setEnabled(custom);
	marginsModule->topLE->setEnabled(custom);
	marginsModule->topUnit->setEnabled(custom);

	marginsModule->bottomL->setEnabled(custom);
	marginsModule->bottomLE->setEnabled(custom);
	marginsModule->bottomUnit->setEnabled(custom);

	marginsModule->innerL->setEnabled(custom);
	marginsModule->innerLE->setEnabled(custom);
	marginsModule->innerUnit->setEnabled(custom);

	marginsModule->outerL->setEnabled(custom);
	marginsModule->outerLE->setEnabled(custom);
	marginsModule->outerUnit->setEnabled(custom);

	marginsModule->headheightL->setEnabled(custom);
	marginsModule->headheightLE->setEnabled(custom);
	marginsModule->headheightUnit->setEnabled(custom);

	marginsModule->headsepL->setEnabled(custom);
	marginsModule->headsepLE->setEnabled(custom);
	marginsModule->headsepUnit->setEnabled(custom);

	marginsModule->footskipL->setEnabled(custom);
	marginsModule->footskipLE->setEnabled(custom);
	marginsModule->footskipUnit->setEnabled(custom);
}


void QDocumentDialog::updateFontsize(string const & items, string const & sel)
{
	textLayoutModule->fontsizeCO->clear();
	textLayoutModule->fontsizeCO->insertItem("default");

	for (int n = 0; !token(items,'|',n).empty(); ++n)
		textLayoutModule->fontsizeCO->
			insertItem(toqstr(token(items,'|',n)));

	for (int n = 0; n<textLayoutModule->fontsizeCO->count(); ++n) {
		if (fromqstr(textLayoutModule->fontsizeCO->text(n)) == sel) {
			textLayoutModule->fontsizeCO->setCurrentItem(n);
			break;
		}
	}
}


void QDocumentDialog::updatePagestyle(string const & items, string const & sel)
{
	pageLayoutModule->pagestyleCO->clear();
	pageLayoutModule->pagestyleCO->insertItem("default");

	for (int n=0; !token(items,'|',n).empty(); ++n)
		pageLayoutModule->pagestyleCO->
			insertItem(toqstr(token(items,'|',n)));

	for (int n = 0; n<pageLayoutModule->pagestyleCO->count(); ++n) {
		if (fromqstr(pageLayoutModule->pagestyleCO->text(n))==sel) {
			pageLayoutModule->pagestyleCO->setCurrentItem(n);
			break;
		}
	}
}


void QDocumentDialog::classChanged()
{
	ControlDocument & cntrl = form_->controller();
	BufferParams & params = cntrl.params();

	lyx::textclass_type const tc = latexModule->classCO->currentItem();

	if (form_->controller().loadTextclass(tc)) {
		params.textclass = tc;

		if (lyxrc.auto_reset_options) {
			params.useClassDefaults();
			form_->update_contents();
		} else {
			updateFontsize(cntrl.textClass().opt_fontsize(),
				       params.fontsize);

			updatePagestyle(cntrl.textClass().opt_pagestyle(),
					params.pagestyle);
		}
	} else {
		latexModule->classCO->setCurrentItem(params.textclass);
	}
}


void QDocumentDialog::updateNumbering()
{
	int const depth = numberingModule->depthSL->value();
	int const toc = numberingModule->tocSL->value();
	QListViewItem * partitem = numberingModule->tocLV->firstChild();
	QListViewItem * chapteritem = partitem->nextSibling();
	QListViewItem * sectionitem = chapteritem->nextSibling();
	QListViewItem * subsectionitem = sectionitem->nextSibling();
	QListViewItem * subsubsectionitem = subsectionitem->nextSibling();
	QListViewItem * paragraphitem = subsubsectionitem->nextSibling();
	QListViewItem * subparagraphitem = paragraphitem->nextSibling();

	QString const no = qt_("No");
	QString const yes = qt_("Yes");

	//numberingModule->tocLV->setUpdatesEnabled(false);

	partitem->setText(1, yes);
	chapteritem->setText(1, yes);
	sectionitem->setText(1, yes);
	subsectionitem->setText(1, yes);
	subsubsectionitem->setText(1, yes);
	paragraphitem->setText(1, yes);
	subparagraphitem->setText(1, yes);
	partitem->setText(2, yes);
	chapteritem->setText(2, yes);
	sectionitem->setText(2, yes);
	subsectionitem->setText(2, yes);
	subsubsectionitem->setText(2, yes);
	paragraphitem->setText(2, yes);
	subparagraphitem->setText(2, yes);

	// numbering
	if (depth < -1) partitem->setText(1, no);
	if (depth < 0) chapteritem->setText(1, no);
	if (depth < 1) sectionitem->setText(1, no);
	if (depth < 2) subsectionitem->setText(1, no);
	if (depth < 3) subsubsectionitem->setText(1, no);
	if (depth < 4) paragraphitem->setText(1, no);
	if (depth < 5) subparagraphitem->setText(1, no);

	// in toc
	if (toc < 0) chapteritem->setText(2, no);
	if (toc < 1) sectionitem->setText(2, no);
	if (toc < 2) subsectionitem->setText(2, no);
	if (toc < 3) subsubsectionitem->setText(2, no);
	if (toc < 4) paragraphitem->setText(2, no);
	if (toc < 5) subparagraphitem->setText(2, no);

	//numberingModule->tocLV->setUpdatesEnabled(true);
	//numberingModule->tocLV->update();
}


void QDocumentDialog::updateBranchView()
{
	// store the selected branch
	QListViewItem * selItem =
		branchesModule->branchesLV->selectedItem();
	QString sel_branch;
	if (selItem != 0)
		sel_branch = selItem->text(0);

	branchesModule->branchesLV->clear();

	BranchList::const_iterator it = form_->branchlist_.begin();
	BranchList::const_iterator const end = form_->branchlist_.end();
	for (; it != end; ++it) {
		QString const bname = toqstr(it->getBranch());
		QString const sel = it->getSelected() ? qt_("Yes") : qt_("No");
		QListViewItem * newItem =
			new QListViewItem(branchesModule->branchesLV, bname, sel);
		string const x11hexname = it->getColor();
		QColor itemcolor;
		if (x11hexname[0] == '#')
			itemcolor.setNamedColor(toqstr(x11hexname));
		if (itemcolor.isValid()) {
			QPixmap coloritem(30, 10);
			coloritem.fill(itemcolor);
			newItem->setPixmap(2, coloritem);
		}
		// restore selected branch
		if (bname == sel_branch)
			branchesModule->branchesLV->setSelected(newItem, true);
	}
	form_->changed();
}


void QDocumentDialog::addBranchPressed()
{
	QString const new_branch = branchesModule->newBranchLE->text();
	if (!new_branch.isEmpty()) {
		form_->branchlist_.add(fromqstr(new_branch));
		branchesModule->newBranchLE->clear();
		updateBranchView();
	}
}


void QDocumentDialog::deleteBranchPressed()
{
	QListViewItem * selItem =
		branchesModule->branchesLV->selectedItem();
	QString sel_branch;
	if (selItem != 0)
		sel_branch = selItem->text(0);
	if (sel_branch) {
		form_->branchlist_.remove(fromqstr(sel_branch));
		branchesModule->newBranchLE->clear();
		updateBranchView();
	}
}


void QDocumentDialog::toggleBranchPressed()
{
	QListViewItem * selItem =
		branchesModule->branchesLV->selectedItem();
	toggleBranch(selItem);
}


void QDocumentDialog::branchDoubleClicked(QListViewItem * selItem)
{
	toggleBranch(selItem);
}


void QDocumentDialog::toggleBranch(QListViewItem * selItem)
{
	if (selItem == 0)
		return;

	QString sel_branch = selItem->text(0);
	if (sel_branch) {
		bool const selected = selItem->text(1) == qt_("Yes");
		Branch * branch = form_->branchlist_.find(fromqstr(sel_branch));
		if (branch && branch->setSelected(!selected)) {
			branchesModule->newBranchLE->clear();
			updateBranchView();
		}
	}
}


void QDocumentDialog::toggleBranchColor()
{
	QListViewItem * selItem =
		branchesModule->branchesLV->selectedItem();
	QString sel_branch;
	if (selItem != 0)
		sel_branch = selItem->text(0);
	if (sel_branch) {
		QColor initial;
		string current_branch = fromqstr(sel_branch);
		Branch * branch =
			form_->branchlist_.find(current_branch);
		if (!branch)
			return;

		string x11hexname = branch->getColor();
		if (x11hexname[0] == '#')
			initial.setNamedColor(toqstr(x11hexname));
		QColor ncol(QColorDialog::getColor(initial));
		if (ncol.isValid()){
			// add the color to the branchlist
			branch->setColor(fromqstr(ncol.name()));
			branchesModule->newBranchLE->clear();
			updateBranchView();
		}
	}
}
