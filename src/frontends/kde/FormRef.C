/**
 * \file FormRef.C
 * Copyright 2001 the LyX Team
 * Read the file COPYING
 *
 * \author John Levon
 */

#include <config.h>

#include "support/lstrings.h"
 
#include "FormRef.h"
#include "ControlRef.h"
#include "gettext.h"
#include "refdlg.h"
#include "insets/insetref.h"

#include <qtooltip.h>

using std::endl;

FormRef::FormRef(ControlRef & c)
	: KFormBase<ControlRef, RefDialog>(c),
	sort_(0), gotowhere_(GOTOREF), refs_(0)
{
}


void FormRef::build()
{
	dialog_.reset(new RefDialog(this, 0, _("LyX: Cross Reference"), false));

	//bc().setUndoAll(dialog_->buttonRestore); 
	bc().setOK(dialog_->buttonOk); 
	//bc().setApply(dialog_->buttonApply); 
	bc().setCancel(dialog_->buttonCancel); 
	// FIXME 
}

 
void FormRef::update()
{
	dialog_->reference->setText(controller().params().getContents().c_str());
	dialog_->refname->setText(controller().params().getOptions().c_str());

	dialog_->type->setCurrentItem(InsetRef::getType(controller().params().getCmdName()));

	// Name is irrelevant to LaTeX/Literate documents
	dialog_->refname->setEnabled(controller().docType() != ControlRef::LATEX &&
	    controller().docType() != ControlRef::LITERATE);

	// type is irrelevant to LinuxDoc/DocBook.
	if (controller().docType() == ControlRef::LINUXDOC ||
	    controller().docType() == ControlRef::DOCBOOK) {
		dialog_->type->setCurrentItem(0);
		dialog_->type->setEnabled(false);
	} else
		dialog_->type->setEnabled(true);

	dialog_->buttonGoto->setText(_("&Goto reference"));
	QToolTip::remove(dialog_->buttonGoto);
	QToolTip::add(dialog_->buttonGoto,_("Jump to selected reference"));

	gotowhere_ = GOTOREF;

	dialog_->sort->setChecked(sort_);

	do_ref_update();

	//dialog_->buttonGoto->setEnabled(params.getContents()!="");
	//dialog_->buttonOk->setEnabled(params.getContents()!="");
}


void FormRef::apply()
{
	controller().params().setCmdName(InsetRef::getName(dialog_->type->currentItem()));
	controller().params().setContents(dialog_->reference->text());
	controller().params().setOptions(dialog_->refname->text());
}


void FormRef::select(char const * text)
{
	highlight(text);
	goto_ref();
}


void FormRef::highlight(char const * text)
{
	// FIXME 
	if (gotowhere_==GOTOBACK)
		goto_ref();

	dialog_->buttonGoto->setEnabled(true);
}


void FormRef::set_sort(bool on)
{
	if (on != sort_) {
		sort_=on;
		dialog_->refs->clear();
		updateRefs();
	}
}


void FormRef::goto_ref()
{
	/* FIXME 
	switch (gotowhere_) {
		case GOTOREF:
			lv_->getLyXFunc()->Dispatch(LFUN_REF_GOTO, dialog_->reference->text());
			gotowhere=GOTOBACK;
			dialog_->buttonGoto->setText(_("&Go back"));
			QToolTip::remove(dialog_->buttonGoto); 
			QToolTip::add(dialog_->buttonGoto,_("Jump back to original position"));
			break;
		case GOTOBACK:
			lv_->getLyXFunc()->Dispatch(LFUN_BOOKMARK_GOTO, "0");
			gotowhere=GOTOREF;
			dialog_->buttonGoto->setText(_("&Goto reference"));
			QToolTip::remove(dialog_->buttonGoto); 
			QToolTip::add(dialog_->buttonGoto,_("Jump to selected reference"));
			break;
		}
	*/ 
}


void FormRef::updateRefs()
{
	// list will be re-done, should go back if necessary
	/* FIXME if (gotowhere == GOTOBACK) {
		lv_->getLyXFunc()->Dispatch(LFUN_BOOKMARK_GOTO, "0");
		gotowhere = GOTOREF;
		dialog_->buttonGoto->setText(_("&Goto reference"));
		QToolTip::remove(dialog_->buttonGoto); 
		QToolTip::add(dialog_->buttonGoto,_("Jump to selected reference"));
	}*/

	dialog_->refs->setAutoUpdate(false);

	// need this because Qt will send a highlight() here for
	// the first item inserted
	string tmp(dialog_->reference->text());

	for (vector<string>::const_iterator iter = refs_.begin();
		iter != refs_.end(); ++iter) {
		if (sort_)
			dialog_->refs->inSort(iter->c_str());
		else
			dialog_->refs->insertItem(iter->c_str());
	}

	dialog_->reference->setText(tmp.c_str());

	for (unsigned int i=0; i < dialog_->refs->count(); ++i) {
		if (!compare(dialog_->reference->text(),
			     dialog_->refs->text(i)))
			dialog_->refs->setCurrentItem(i);
	}

	dialog_->refs->setAutoUpdate(true);
	dialog_->refs->update();
}


void FormRef::do_ref_update()
{
	refs_.clear();
	dialog_->refs->clear();
	refs_ = controller().getLabelList();
	if (!refs_.empty())
		dialog_->sort->setEnabled(true);
	updateRefs();
}
