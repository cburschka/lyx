/**
 * \file QDocumentDialog.C
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

#include "ui/ClassModuleBase.h"
#include "ui/PackagesModuleBase.h"
#include "ui/PaperModuleBase.h"
#include "ui/LanguageModuleBase.h"
#include "ui/BulletsModuleBase.h"
#include "BulletsModule.h"
#include "ui/BiblioModuleBase.h"
#include "ui/NumberingModuleBase.h"
#include "ui/MarginsModuleBase.h"
#include "ui/PreambleModuleBase.h"

#include "Spacing.h"
#include "support/lstrings.h"
#include "lyxrc.h"
#include "buffer.h"

#include <qwidgetstack.h>
#include <qlistbox.h>
#include <qlabel.h>
#include <qmultilineedit.h>
#include <qlineedit.h>
#include <qlistview.h>
#include <qpushbutton.h>
#include <qcombobox.h>
#include <qradiobutton.h>
#include <qcheckbox.h>
#include <qspinbox.h>
#include "lengthcombo.h"


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

	moduleLB->clear();
	moduleLB->insertItem(qt_("Layout"), LAYOUT);
	moduleLB->insertItem(qt_("Paper"), PAPER);
	moduleLB->insertItem(qt_("Margins"), MARGINS);
	moduleLB->insertItem(qt_("Language"), LANGUAGE);
	moduleLB->insertItem(qt_("Bullets"), BULLETS);
	moduleLB->insertItem(qt_("Numbering"), NUMBERING);
	moduleLB->insertItem(qt_("Bibliography"), BIBLIOGRAPHY);
	moduleLB->insertItem(qt_("Packages"), PACKAGES);
	moduleLB->insertItem(qt_("Preamble"), PREAMBLE);
	moduleLB->setCurrentItem(LAYOUT);
	moduleLB->setMinimumSize(moduleLB->sizeHint());

	layoutModule = new ClassModuleBase(this);
	paperModule = new PaperModuleBase(this);
	marginsModule = new MarginsModuleBase(this);
	langModule = new LanguageModuleBase(this);
	bulletsModule = new BulletsModule(this);
	numberingModule = new NumberingModuleBase(this);
	biblioModule = new BiblioModuleBase(this);
	packagesModule = new PackagesModuleBase(this);
	preambleModule = new PreambleModuleBase(this);

	moduleStack->addWidget(layoutModule, LAYOUT);
	moduleStack->addWidget(paperModule, PAPER);
	moduleStack->addWidget(marginsModule, MARGINS);
	moduleStack->addWidget(langModule, LANGUAGE);
	moduleStack->addWidget(bulletsModule, BULLETS);
	moduleStack->addWidget(numberingModule, NUMBERING);
	moduleStack->addWidget(biblioModule, BIBLIOGRAPHY);
	moduleStack->addWidget(packagesModule, PACKAGES);
	moduleStack->addWidget(preambleModule, PREAMBLE);

	moduleStack->raiseWidget(LAYOUT);


	// take care of title
	QFont f = titleL->font();
	f.setWeight(QFont::Bold);
	titleL->setFont(f);
	setTitle(LAYOUT);

	// preamble
	connect(preambleModule->preambleMLE, SIGNAL(textChanged()),
		 this, SLOT(change_adaptor()));
	// biblio
	connect(biblioModule->natbibCB, SIGNAL(toggled(bool)),
		 this, SLOT(change_adaptor()));
	connect(biblioModule->citeStyleCO, SIGNAL(activated(int)),
		 this, SLOT(change_adaptor()));
	// language & quote
	connect(langModule->singleQuoteRB, SIGNAL(toggled(bool)),
		 this, SLOT(change_adaptor()));
	connect(langModule->doubleQuoteRB, SIGNAL(toggled(bool)),
		 this, SLOT(change_adaptor()));
	connect(langModule->languageCO, SIGNAL(activated(int)),
		 this, SLOT(change_adaptor()));
	connect(langModule->defaultencodingCB, SIGNAL(toggled(bool)),
		 this, SLOT(change_adaptor()));
	connect(langModule->encodingCO, SIGNAL(activated(int)),
		 this, SLOT(change_adaptor()));
	connect(langModule->quoteStyleCO, SIGNAL(activated(int)),
		 this, SLOT(change_adaptor()));
	// numbering
	connect(numberingModule->sectionnrDepthSB,
		 SIGNAL(valueChanged(int)),
		 this, SLOT(change_adaptor()));
	connect(numberingModule->tocDepthSB,
		 SIGNAL(valueChanged(int)),
		 this, SLOT(change_adaptor()));
	// packages
	connect(packagesModule->amsCB, SIGNAL(toggled(bool)),
		 this, SLOT(change_adaptor()));
	connect(packagesModule->psdriverCO, SIGNAL(activated(int)),
		 this, SLOT(change_adaptor()));
	// layout
	connect(layoutModule->classCO, SIGNAL(activated(int)),
		 this, SLOT(change_adaptor()));
	connect(layoutModule->optionsLE,
		 SIGNAL(textChanged(const QString&)),
		 this, SLOT(change_adaptor()));
	connect(layoutModule->pagestyleCO, SIGNAL(activated(int)),
		 this, SLOT(change_adaptor()));
	connect(layoutModule->fontsCO, SIGNAL(activated(int)),
		 this, SLOT(change_adaptor()));
	connect(layoutModule->fontsizeCO, SIGNAL(activated(int)),
		 this, SLOT(change_adaptor()));
	connect(layoutModule->lspacingCO, SIGNAL(activated(int)),
		 this, SLOT(change_adaptor()));
	connect(layoutModule->lspacingCO, SIGNAL(activated(int)),
		 this, SLOT(setLSpacing(int)));
	connect(layoutModule->lspacingLE,
		 SIGNAL(textChanged(const QString&)),
		 this, SLOT(change_adaptor()));
	connect(layoutModule->floatPlacementLE,
		 SIGNAL(textChanged(const QString&)),
		 this, SLOT(change_adaptor()));
	connect(layoutModule->skipRB, SIGNAL(toggled(bool)),
		 this, SLOT(change_adaptor()));
	connect(layoutModule->indentRB, SIGNAL(toggled(bool)),
		 this, SLOT(change_adaptor()));
	connect(layoutModule->skipCO, SIGNAL(activated(int)),
		 this, SLOT(change_adaptor()));
	connect(layoutModule->skipLE,
		 SIGNAL(textChanged(const QString&)),
		 this, SLOT(change_adaptor()));
	connect(layoutModule->skipLengthCO, SIGNAL(activated(int)),
		 this, SLOT(change_adaptor()));

	connect(layoutModule->classCO, SIGNAL(activated(int)),
		 this, SLOT(classChanged()));
	connect(layoutModule->skipCO, SIGNAL(activated(int)),
		 this, SLOT(setSkip(int)));
	connect(layoutModule->skipRB, SIGNAL(toggled(bool)),
		 this, SLOT(enableSkip(bool)));

	// margins
	connect(marginsModule->marginCO, SIGNAL(activated(int)),
		 this, SLOT(setCustomMargins(int)));

	connect(marginsModule->marginCO, SIGNAL(activated(int)),
		 this, SLOT(change_adaptor()));
	connect(marginsModule->topLE, SIGNAL(textChanged(const QString&)),
		 this, SLOT(change_adaptor()));
	connect(marginsModule->topUnit, SIGNAL(activated(int)),
		 this, SLOT(change_adaptor()));
	connect(marginsModule->bottomLE, SIGNAL(textChanged(const QString&)),
		 this, SLOT(change_adaptor()));
	connect(marginsModule->bottomUnit, SIGNAL(activated(int)),
		 this, SLOT(change_adaptor()));
	connect(marginsModule->innerLE, SIGNAL(textChanged(const QString&)),
		 this, SLOT(change_adaptor()));
	connect(marginsModule->innerUnit, SIGNAL(activated(int)),
		 this, SLOT(change_adaptor()));
	connect(marginsModule->outerLE, SIGNAL(textChanged(const QString&)),
		 this, SLOT(change_adaptor()));
	connect(marginsModule->outerUnit, SIGNAL(activated(int)),
		 this, SLOT(change_adaptor()));
	connect(marginsModule->headheightLE, SIGNAL(textChanged(const QString&)),
		 this, SLOT(change_adaptor()));
	connect(marginsModule->headheightUnit, SIGNAL(activated(int)),
		 this, SLOT(change_adaptor()));
	connect(marginsModule->headsepLE, SIGNAL(textChanged(const QString&)),
		 this, SLOT(change_adaptor()));
	connect(marginsModule->headsepUnit, SIGNAL(activated(int)),
		 this, SLOT(change_adaptor()));
	connect(marginsModule->footskipLE, SIGNAL(textChanged(const QString&)),
		 this, SLOT(change_adaptor()));
	connect(marginsModule->footskipUnit, SIGNAL(activated(int)),
		 this, SLOT(change_adaptor()));

	// paper
	connect(paperModule->papersizeCO, SIGNAL(activated(int)),
		 this, SLOT(setMargins(int)));
	connect(paperModule->papersizeCO, SIGNAL(activated(int)),
		 this, SLOT(setCustomPapersize(int)));
	connect(paperModule->papersizeCO, SIGNAL(activated(int)),
		 this, SLOT(setCustomPapersize(int)));
	connect(paperModule->portraitRB, SIGNAL(toggled(bool)),
		 this, SLOT(portraitChanged()));

	connect(paperModule->papersizeCO, SIGNAL(activated(int)),
		 this, SLOT(change_adaptor()));
	connect(paperModule->paperheightLE, SIGNAL(textChanged(const QString&)),
		 this, SLOT(change_adaptor()));
	connect(paperModule->paperwidthLE, SIGNAL(textChanged(const QString&)),
		 this, SLOT(change_adaptor()));
	connect(paperModule->paperwidthUnitCO, SIGNAL(activated(int)),
		 this, SLOT(change_adaptor()));
	connect(paperModule->paperheightUnitCO, SIGNAL(activated(int)),
		 this, SLOT(change_adaptor()));
	connect(paperModule->portraitRB, SIGNAL(toggled(bool)),
		 this, SLOT(change_adaptor()));
	connect(paperModule->landscapeRB, SIGNAL(toggled(bool)),
		 this, SLOT(change_adaptor()));
	connect(paperModule->twoColumnCB, SIGNAL(toggled(bool)),
		 this, SLOT(change_adaptor()));
	connect(paperModule->facingPagesCB, SIGNAL(toggled(bool)),
		 this, SLOT(change_adaptor()));

	// bullets
	connect(bulletsModule->bulletsizeCO, SIGNAL(activated(int)),
		 this, SLOT(change_adaptor()));
	connect(bulletsModule->bulletsLV, SIGNAL(selectionChanged()),
		 this, SLOT(change_adaptor()));
}


QDocumentDialog::~QDocumentDialog()
{
}


void QDocumentDialog::setTitle(int item)
{
	switch(item) {
	case LAYOUT:
		titleL->setText(qt_("Document Style"));
		break;
	case PAPER:
		titleL->setText(qt_("Papersize and Orientation"));
		break;
	case MARGINS:
		titleL->setText(qt_("Margins"));
		break;
	case LANGUAGE:
		titleL->setText(qt_("Language Settings and Quote Style"));
		break;
	case BULLETS:
		titleL->setText(qt_("Bullet Types"));
		break;
	case NUMBERING:
		titleL->setText(qt_("Numbering"));
		break;
	case BIBLIOGRAPHY:
		titleL->setText(qt_("Bibliography Settings"));
		break;
	case PACKAGES:
		titleL->setText(qt_("LaTeX Packages and Options"));
		break;
	case PREAMBLE:
		titleL->setText(qt_("LaTeX Preamble"));
		break;
	}
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
	layoutModule->lspacingLE->setEnabled(item == 3);
}


void QDocumentDialog::setSkip(int item)
{
	bool const enable = (item == 3);
	layoutModule->skipLE->setEnabled(enable);
	layoutModule->skipLengthCO->setEnabled(enable);
}


void QDocumentDialog::enableSkip(bool skip)
{
	layoutModule->skipCO->setEnabled(skip);
	layoutModule->skipLE->setEnabled(skip);
	layoutModule->skipLengthCO->setEnabled(skip);
	if (skip)
		setSkip(layoutModule->skipCO->currentItem());
}

void QDocumentDialog::portraitChanged()
{
	setMargins(paperModule->papersizeCO->currentItem());
}

void QDocumentDialog::setMargins(int papersize)
{
	int olditem = marginsModule->marginCO->currentItem();
	marginsModule->marginCO->clear();
	marginsModule->marginCO->insertItem(qt_("Default"));
	marginsModule->marginCO->insertItem(qt_("Custom"));
	bool a4size = (papersize == 6 || papersize == 0
			&& lyxrc.default_papersize == BufferParams::PAPER_A4PAPER);
	if (a4size && paperModule->portraitRB->isChecked()) {
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

	paperModule->paperwidthL->setEnabled(custom);
	paperModule->paperwidthLE->setEnabled(custom);
	paperModule->paperwidthUnitCO->setEnabled(custom);
	paperModule->paperheightL->setEnabled(custom);
	paperModule->paperheightLE->setEnabled(custom);
	paperModule->paperheightLE->setFocus();
	paperModule->paperheightUnitCO->setEnabled(custom);
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
	layoutModule->fontsizeCO->clear();
	layoutModule->fontsizeCO->insertItem("default");

	for (int n=0; !token(items,'|',n).empty(); ++n)
		layoutModule->fontsizeCO->
			insertItem(toqstr(token(items,'|',n)));

	for (int n = 0; n<layoutModule->fontsizeCO->count(); ++n) {
		if (fromqstr(layoutModule->fontsizeCO->text(n)) == sel) {
			layoutModule->fontsizeCO->setCurrentItem(n);
			break;
		}
	}
}


void QDocumentDialog::updatePagestyle(string const & items, string const & sel)
{
	layoutModule->pagestyleCO->clear();
	layoutModule->pagestyleCO->insertItem("default");

	for (int n=0; !token(items,'|',n).empty(); ++n)
		layoutModule->pagestyleCO->
			insertItem(toqstr(token(items,'|',n)));

	for (int n = 0; n<layoutModule->pagestyleCO->count(); ++n) {
		if (fromqstr(layoutModule->pagestyleCO->text(n))==sel) {
			layoutModule->pagestyleCO->setCurrentItem(n);
			break;
		}
	}
}


void QDocumentDialog::classChanged()
{
	ControlDocument & cntrl = form_->controller();
	BufferParams & params = cntrl.params();

	lyx::textclass_type const tc = layoutModule->classCO->currentItem();

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
		for (int n = 0; n<layoutModule->classCO->count(); ++n) {
			if (layoutModule->classCO->text(n) ==
			    toqstr(cntrl.textClass().description())) {
				layoutModule->classCO->setCurrentItem(n);
				break;
			}
		}
	}
}
