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
#include "gettext.h"

#include "ControlDocument.h"
#include "QDocument.h"

#ifdef __GNUG__
#pragma implementation
#endif

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

#include "QBrowseBox.h"
#include "Spacing.h"
#include "support/filetools.h"
#include "support/lstrings.h"
#include "Alert.h"
#include "lyxrc.h"
#include "buffer.h"
#include "lyx_main.h" // for user_lyxdir

#include <qwidgetstack.h>
#include <qlistbox.h>
#include <qlabel.h>
#include <qmultilineedit.h>
#include <qlineedit.h>
#include <qpushbutton.h>
#include <qcombobox.h>
#include <qradiobutton.h>
#include <qcheckbox.h>
#include <qspinbox.h>
#include "lengthcombo.h"
/* 
 *  Constructs a DocumentDialog which is a child of 'parent', with the 
 *  name 'name' and widget flags set to 'f' 
 *
 *  The dialog will by default be modeless, unless you set 'modal' to
 *  TRUE to construct a modal dialog.
 */
QDocumentDialog::QDocumentDialog( QDocument * form)
	: QDocumentDialogBase( 0, 0, false, 0 ), form_(form)
{
	connect(okPB, SIGNAL(clicked()),
		form, SLOT(slotOK()));
	connect(applyPB, SIGNAL(clicked()),
		form, SLOT(slotApply()));
	connect(cancelPB, SIGNAL(clicked()),
		form, SLOT(slotClose()));
	connect(restorePB, SIGNAL(clicked()),
		form, SLOT(slotRestore()));
	
	moduleLB->clear();
	moduleLB->insertItem( _("Layout"), LAYOUT );
	moduleLB->insertItem( _("Packages"), PACKAGES );
	moduleLB->insertItem( _("Paper"), PAPER );
	moduleLB->insertItem( _("Margins"), MARGINS );
	moduleLB->insertItem( _("Language"), LANGUAGE );
	moduleLB->insertItem( _("Bullets"), BULLETS );
	moduleLB->insertItem( _("Numbering"), NUMBERING );
	moduleLB->insertItem( _("Bibliography"), BIBLIOGRAPHY );
	moduleLB->insertItem( _("Preamble"), PREAMBLE );
	moduleLB->setCurrentItem(LAYOUT);

	layoutModule = new ClassModuleBase(this);
	paperModule = new PaperModuleBase(this);
	marginsModule = new MarginsModuleBase(this);
	packagesModule = new PackagesModuleBase(this);
	langModule = new LanguageModuleBase(this);
	bulletsModule = new BulletsModule(this);
	numberingModule = new NumberingModuleBase(this);
	biblioModule = new BiblioModuleBase(this);
	preambleModule = new PreambleModuleBase(this);

	moduleStack->addWidget(layoutModule,LAYOUT);
	moduleStack->addWidget(paperModule,PAPER);
	moduleStack->addWidget(marginsModule,MARGINS);
	moduleStack->addWidget(packagesModule,PACKAGES);
	moduleStack->addWidget(langModule,LANGUAGE);
	moduleStack->addWidget(bulletsModule,BULLETS);
	moduleStack->addWidget(numberingModule,NUMBERING);
	moduleStack->addWidget(biblioModule,BIBLIOGRAPHY);
	moduleStack->addWidget(preambleModule,PREAMBLE);
	
	moduleStack->raiseWidget(LAYOUT);

	//QPixmap image("standard.xpm");
	//bulletsModule->bulletsBB->insertItem(image);
	//bulletsModule->bulletsBB->resize(image.width(),image.height());
	// take care of title
	QFont f = titleL->font();
	f.setWeight( QFont::Bold );
	titleL->setFont( f );
	setTitle(LAYOUT);
	
	/* preamble */
	connect( preambleModule->preambleMLE, SIGNAL( textChanged() ),
		 this , SLOT( change_adaptor() ) );
	/* biblio */
	connect( biblioModule->natbibCB, SIGNAL( toggled(bool) ),
		 this , SLOT( change_adaptor() ) );
	connect( biblioModule->citeStyleCO, SIGNAL( activated(int) ),
		 this , SLOT( change_adaptor() ) );
	/* language & quote */
	connect( langModule->singleQuoteRB, SIGNAL( toggled(bool) ),
		 this , SLOT( change_adaptor() ) );
	connect( langModule->doubleQuoteRB, SIGNAL( toggled(bool) ),
		 this , SLOT( change_adaptor() ) );
	connect( langModule->languageCO, SIGNAL( activated(int) ),
		 this , SLOT( change_adaptor() ) );
	connect( langModule->quoteStyleCO, SIGNAL( activated(int) ),
		 this , SLOT( change_adaptor() ) );
	/* numbering */
	connect( numberingModule->sectionnrDepthSB,
		 SIGNAL( valueChanged(int) ),
		 this , SLOT( change_adaptor() ) );
	connect( numberingModule->tocDepthSB,
		 SIGNAL( valueChanged(int) ),
		 this , SLOT( change_adaptor() ) );
	/* packages */
	connect( packagesModule->lspacingCO, SIGNAL( activated(int) ),
		 this , SLOT( change_adaptor() ) );
	connect( packagesModule->lspacingCO, SIGNAL( activated(int) ),
		 this , SLOT( setLSpacing(int) ) );
	connect( packagesModule->lspacingLE,
		 SIGNAL( textChanged(const QString&) ),
		 this , SLOT( change_adaptor() ) );
	connect( packagesModule->encodingCO, SIGNAL( activated(int) ),
		 this , SLOT( change_adaptor() ) );
	connect( packagesModule->amsCB, SIGNAL( toggled(bool) ),
		 this , SLOT( change_adaptor() ) );
	connect( packagesModule->psdriverCO, SIGNAL( activated(int) ),
		 this , SLOT( change_adaptor() ) );
	/* layout */
	connect( layoutModule->classCO, SIGNAL( activated(int) ),
		 this , SLOT( change_adaptor() ) );
	connect( layoutModule->optionsLE,
		 SIGNAL( textChanged(const QString&) ),
		 this , SLOT( change_adaptor() ) );
	connect( layoutModule->pagestyleCO, SIGNAL( activated(int) ),
		 this , SLOT( change_adaptor() ) );
	connect( layoutModule->fontsCO, SIGNAL( activated(int) ),
		 this , SLOT( change_adaptor() ) );
	connect( layoutModule->fontsizeCO, SIGNAL( activated(int) ),
		 this , SLOT( change_adaptor() ) );
	connect( layoutModule->floatPlacementLE,
		 SIGNAL( textChanged(const QString&) ),
		 this , SLOT( change_adaptor() ) );
	connect( layoutModule->skipRB, SIGNAL( toggled(bool) ),
		 this , SLOT( change_adaptor() ) );
	connect( layoutModule->indentRB, SIGNAL( toggled(bool) ),
		 this , SLOT( change_adaptor() ) );
	connect( layoutModule->skipCO, SIGNAL( activated(int) ),
		 this , SLOT( change_adaptor() ) );
	connect( layoutModule->skipLE,
		 SIGNAL( textChanged(const QString&) ),
		 this , SLOT( change_adaptor() ) );
	connect( layoutModule->skipLengthCO, SIGNAL( activated(int) ),
		 this , SLOT( change_adaptor() ) );
	
	connect( layoutModule->classCO, SIGNAL( activated(int) ),
		 this , SLOT( classChanged() ) );
	connect( layoutModule->skipCO, SIGNAL( activated(int) ),
		 this , SLOT( setSkip(int) ) );
	connect( layoutModule->skipRB, SIGNAL( toggled(bool) ),
		 this , SLOT( enableSkip(bool) ) );
	
	/* margins */
	connect( marginsModule->marginCO, SIGNAL( activated(int) ),
		 this , SLOT( setCustomMargins(int) ) );
	
	connect( marginsModule->marginCO, SIGNAL( activated(int) ),
		 this , SLOT( change_adaptor() ) );
	connect( marginsModule->topLE, SIGNAL( textChanged(const QString&) ),
		 this , SLOT( change_adaptor() ) );
	connect( marginsModule->topUnit, SIGNAL( activated(int) ),
		 this , SLOT( change_adaptor() ) );
	connect( marginsModule->bottomLE, SIGNAL( textChanged(const QString&) ),
		 this , SLOT( change_adaptor() ) );
	connect( marginsModule->bottomUnit, SIGNAL( activated(int) ),
		 this , SLOT( change_adaptor() ) );
	connect( marginsModule->innerLE, SIGNAL( textChanged(const QString&) ),
		 this , SLOT( change_adaptor() ) );
	connect( marginsModule->innerUnit, SIGNAL( activated(int) ),
		 this , SLOT( change_adaptor() ) );
	connect( marginsModule->outerLE, SIGNAL( textChanged(const QString&) ),
		 this , SLOT( change_adaptor() ) );
	connect( marginsModule->outerUnit, SIGNAL( activated(int) ),
		 this , SLOT( change_adaptor() ) );
	connect( marginsModule->headheightLE, SIGNAL( textChanged(const QString&) ),
		 this , SLOT( change_adaptor() ) );
	connect( marginsModule->headheightUnit, SIGNAL( activated(int) ),
		 this , SLOT( change_adaptor() ) );
	connect( marginsModule->headsepLE, SIGNAL( textChanged(const QString&) ),
		 this , SLOT( change_adaptor() ) );
	connect( marginsModule->headsepUnit, SIGNAL( activated(int) ),
		 this , SLOT( change_adaptor() ) );
	connect( marginsModule->footskipLE, SIGNAL( textChanged(const QString&) ),
		 this , SLOT( change_adaptor() ) );
	connect( marginsModule->footskipUnit, SIGNAL( activated(int) ),
		 this , SLOT( change_adaptor() ) );
	
	/* paper */
	connect( paperModule->papersizeCO, SIGNAL( activated(int) ),
		 this , SLOT( setMargins(int) ) );
	connect( paperModule->papersizeCO, SIGNAL( activated(int) ),
		 this , SLOT( setCustomPapersize(int) ) );
	connect( paperModule->papersizeCO, SIGNAL( activated(int) ),
		 this , SLOT( setCustomPapersize(int) ) );
	
	connect( paperModule->papersizeCO, SIGNAL( activated(int) ),
		 this , SLOT( change_adaptor() ) );
	connect( paperModule->paperheightLE, SIGNAL( textChanged(const QString&) ),
		 this , SLOT( change_adaptor() ) );
	connect( paperModule->paperwidthLE, SIGNAL( textChanged(const QString&) ),
		 this , SLOT( change_adaptor() ) );
	connect( paperModule->paperwidthUnitCO, SIGNAL( activated(int) ),
		 this , SLOT( change_adaptor() ) );
	connect( paperModule->paperheightUnitCO, SIGNAL( activated(int) ),
		 this , SLOT( change_adaptor() ) );
	connect( paperModule->portraitRB, SIGNAL( toggled(bool) ),
		 this , SLOT( change_adaptor() ) );
	connect( paperModule->landscapeRB, SIGNAL( toggled(bool) ),
		 this , SLOT( change_adaptor() ) );
	connect( paperModule->twoColumnCB, SIGNAL( toggled(bool) ),
		 this , SLOT( change_adaptor() ) );
	connect( paperModule->facingPagesCB, SIGNAL( toggled(bool) ),
		 this , SLOT( change_adaptor() ) );
	
}

void QDocumentDialog::showPreamble()
{
	moduleStack->raiseWidget(PREAMBLE);
	setTitle(PREAMBLE);
	QDocumentDialogBase::show();
}

/*  
 *  Destroys the object and frees any allocated resources
 */
QDocumentDialog::~QDocumentDialog()
{
    // no need to delete child widgets, Qt does it all for us
}

/* 
 * public slot
 */

void QDocumentDialog::setTitle(int item)
{
	switch(item) {
	case LAYOUT:
		titleL->setText(_("Document Style"));
		break;
	case PACKAGES:
		titleL->setText(_("LaTeX Packages"));
		break;
	case PAPER:
		titleL->setText(_("Papersize and Orientation"));
		break;
	case MARGINS:
		titleL->setText(_("Margins"));
		break;
	case LANGUAGE:
		titleL->setText(_("Language Settings and Quote Style"));
		break;
	case BULLETS:
		titleL->setText(_("Bullet Types"));
		break;
	case NUMBERING:
		titleL->setText(_("Numbering"));
		break;
	case BIBLIOGRAPHY:
		titleL->setText(_("Bibliography Settings"));
		break;
	case PREAMBLE:
		titleL->setText(_("LaTeX Preamble"));
		break;
	}
}

/* 
 * public slot
 */
void QDocumentDialog::saveDocDefault()
{}

/* 
 * public slot
 */
void QDocumentDialog::restore()
{
    qWarning( "DocumentDialog::restore() not yet implemented!" ); 
}

/* 
 * public slot
 */
void QDocumentDialog::useClassDefaults()
{
    qWarning( "DocumentDialog::useClassDefaults() not yet implemented!" ); 
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
	packagesModule->lspacingLE->setEnabled(item==3);
}

void QDocumentDialog::setSkip(int item)
{
	bool enable = (item==3);
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

void QDocumentDialog::setMargins(int papersize)
{
	QStringList a4only;
	QStringList normal;
	a4only << _("Small Margins") << _( "Very small Margins")
	       << _("Very wide Margins ");
	normal << _("Default") << _("Custom");
	
	int olditem = marginsModule->marginCO->currentItem();
	marginsModule->marginCO->clear();
	marginsModule->marginCO->insertStringList(normal);
	if (papersize==6) {
		marginsModule->marginCO->insertStringList(a4only);
	}
	marginsModule->marginCO->setCurrentItem(olditem);
	setCustomMargins(olditem);
}


void QDocumentDialog::setCustomPapersize(int papersize)
{
	bool const custom = (papersize==1);
	
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
	bool const custom = (margin==1);
	
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
			insertItem(token(items,'|',n).c_str());

	for (int n = 0; n<layoutModule->fontsizeCO->count(); ++n) {
		if (layoutModule->fontsizeCO->text(n).latin1()==sel) {
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
			insertItem(token(items,'|',n).c_str());

	for (int n = 0; n<layoutModule->pagestyleCO->count(); ++n) {
		if (layoutModule->pagestyleCO->text(n).latin1()==sel) {
			layoutModule->pagestyleCO->setCurrentItem(n);
			break;
		}
	}
}

void QDocumentDialog::classChanged()
{
	updateFontsize(form_->controller().textClass().opt_fontsize(),
		       form_->controller().params().fontsize);
	
	updatePagestyle(form_->controller().textClass().opt_pagestyle(),
			form_->controller().params().pagestyle);
	
}


