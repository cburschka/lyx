/* This file is part of
 * ======================================================
 *
 *		   LyX, The Document Processor
 *
 *		   Copyright 2000 The LyX Team.
 *
 * ======================================================
 *
 * \author Angus Leeming <a.leeming@ic.ac.uk>
 */

#include <config.h>

#ifdef __GNUG__
#pragma implementation
#endif

#include "FormCitationDialogImpl.h"
#include "FormCitation.h"

#include <qcheckbox.h>
#include <qcombobox.h>
#include <qlineedit.h>
#include <qlistbox.h>
#include <qmultilineedit.h>
#include <qpushbutton.h>

#undef emit
#include "qt2BC.h"
#include "ControlCitation.h"
#include "gettext.h"
#include "support/lstrings.h"
#include "helper_funcs.h"


using std::find;
using std::max;
using std::min;
using std::pair;
using std::sort;
using std::vector;

typedef Qt2CB<ControlCitation, Qt2DB<FormCitationDialogImpl> > base_class;

FormCitation::FormCitation(ControlCitation & c)
	: base_class(c, _("Citation"))
{}


void FormCitation::apply()
{
	controller().params().setCmdName("cite");
	controller().params().setContents(getStringFromVector(citekeys));
	
	string const after  = dialog_->textAfterED->text().latin1();
	controller().params().setOptions(after);
}


void FormCitation::hide()
{
	citekeys.clear();
	bibkeys.clear();
	
	Qt2Base::hide();
}


void FormCitation::build()
{
	// PENDING(kalle) Parent?
	dialog_.reset( new FormCitationDialogImpl( this ));

	dialog_->searchTypeCB->setChecked( false );
	dialog_->searchCaseCB->setChecked( false );
	
	// Manage the ok, apply, restore and cancel/close buttons
	bc().setOK(dialog_->okPB);
	bc().setApply(dialog_->applyPB);
	bc().setCancel(dialog_->cancelPB);
	bc().setRestore(dialog_->restorePB);

	bc().addReadOnly(dialog_->addPB);
	bc().addReadOnly(dialog_->delPB);
	bc().addReadOnly(dialog_->upPB);
	bc().addReadOnly(dialog_->downPB);
	bc().addReadOnly(dialog_->citationStyleCO);
	bc().addReadOnly(dialog_->textBeforeED);
	bc().addReadOnly(dialog_->textAfterED);

	bc().refresh();
}	


void FormCitation::update()
{
	// Make the list of all available bibliography keys
	bibkeys = biblio::getKeys(controller().bibkeysInfo());
	updateBrowser(dialog_->bibLB, bibkeys);
	
	// Ditto for the keys cited in this inset
	citekeys = getVectorFromString(controller().params().getContents());
	updateBrowser(dialog_->citeLB, citekeys);

	// No keys have been selected yet, so...
	dialog_->infoML->clear();
	setBibButtons(OFF);
	setCiteButtons(OFF);

	int noKeys = int(max(bibkeys.size(), citekeys.size()));

	// Place bounds, so that 4 <= noKeys <= 10
	noKeys = max(4, min(10, noKeys));

	dialog_->textAfterED->setText( controller().params().getOptions().c_str());
}


void FormCitation::updateBrowser( QListBox* browser,
				  vector<string> const & keys) const
{
	browser->clear();

	for (vector<string>::const_iterator it = keys.begin();
		it < keys.end(); ++it) {
		string key = frontStrip(strip(*it));
		if( !key.empty() )
			browser->insertItem( key.c_str() );
	}
}


void FormCitation::setBibButtons(State status) const
{
	dialog_->addPB->setEnabled( (status == ON) );
}


void FormCitation::setCiteButtons(State status) const
{
	int const sel = dialog_->citeLB->currentItem();
	int const maxline = dialog_->citeLB->count()-1;
	bool const activate = (status == ON);
	bool const activate_up = (activate && sel != 0);
	bool const activate_down = (activate && sel != maxline);

	dialog_->delPB->setEnabled( activate );
	dialog_->upPB->setEnabled( activate_up );
	dialog_->downPB->setEnabled( activate_down );
}
