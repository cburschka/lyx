/*
 * FormCitation.C
 * (C) 2000 LyX Team
 * John Levon, moz@compsoc.man.ac.uk
 * Changed for Qt2 implementation by Kalle Dalheimer, kalle@klaralvdalens-datakonsult.se
 */

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <config.h>

#include <algorithm>
#include <support/lstrings.h>

#include "FormCitationDialogImpl.h"
#undef emit

#include "Dialogs.h"
#include "FormCitation.h"
#include "gettext.h"
#include "buffer.h"
#include "LyXView.h"
#include "lyxfunc.h"
#include "lyxfont.h"

#include <qlineedit.h>
#include <qlistbox.h>
#include <qmultilineedit.h>
#include <qpushbutton.h>

using SigC::slot;
using std::vector;
using std::pair;
using std::find;
using std::sort;

FormCitation::FormCitation(LyXView *v, Dialogs *d)
    : dialog_(0), lv_(v), d_(d), inset_(0), ih_(0)
{
    // let the dialog be shown
    // This is a permanent connection so we won't bother
    // storing a copy because we won't be disconnecting.
    d->showCitation.connect(slot(this, &FormCitation::showCitation));
    d->createCitation.connect(slot(this, &FormCitation::createCitation));
}

FormCitation::~FormCitation()
{
    delete dialog_;
}


void FormCitation::showCitation(InsetCommand * inset)
{
    if (inset == 0) return;  // maybe we should Assert this?

    // If connected to another inset, disconnect from it.
    if (inset_)
	ih_.disconnect();

    inset_    = inset;
    params    = inset->params();
    ih_ = inset->hideDialog.connect(slot(this, &FormCitation::hide));
    show();
}


void FormCitation::createCitation(string const & arg)
{
    if (inset_) {
	ih_.disconnect();
	inset_ = 0;
    }

    params.setFromString(arg);
    show();
}


void FormCitation::hide()
{
    if( dialog_ )
	dialog_->hide();
}



void FormCitation::show()
{
    if (!dialog_)
	dialog_ = new FormCitationDialogImpl(this, 0, _("LyX: Citation Reference"), false);

    dialog_->show();

    update();
}


void FormCitation::update()
{
    bibkeys.clear();
    bibkeysInfo.clear();

    vector<pair<string,string> > blist =
	lv_->buffer()->getBibkeyList();
    sort(blist.begin(), blist.end());

    for (unsigned int i = 0; i < blist.size(); ++i) {
	bibkeys.push_back(blist[i].first);
	bibkeysInfo.push_back(blist[i].second);
    }
    blist.clear();

    updateBrowser(dialog_->bibliographyKeysLB, bibkeys);

    // Ditto for the keys cited in this inset
    citekeys.clear();
    string tmp, keys(params.getContents());
    keys = frontStrip(split(keys, tmp, ','));
    while (!tmp.empty()) {
	citekeys.push_back(tmp);
	keys = frontStrip(split(keys, tmp, ','));
    }
    updateBrowser(dialog_->insetKeysLB, citekeys);

    // No keys have been selected yet, so...
    dialog_->infoML->clear();
    setBibButtons(OFF);
    setCiteButtons(OFF);

    dialog_->textAfterED->setText( params.getOptions().c_str() );
}


void FormCitation::updateBrowser( QListBox* listbox,
				  vector<string> const & keys) const
{
    listbox->clear();

    for (unsigned int i = 0; i < keys.size(); ++i)
	listbox->insertItem( keys[i].c_str() );
}


void FormCitation::setBibButtons(State status) const
{
    dialog_->leftPB->setEnabled( status == ON );
}


void FormCitation::setCiteButtons(State status) const
{
    int const sel     = dialog_->insetKeysLB->currentItem();
    int const maxline = dialog_->insetKeysLB->count();

    bool const activate      = (status == ON);
    bool const activate_up   = (activate && sel != 1);
    bool const activate_down = (activate && sel != maxline);

    dialog_->stopPB->setEnabled(activate);
    dialog_->upPB->setEnabled(activate_up);
    dialog_->downPB->setEnabled(activate_down);
}


void FormCitation::apply()
{
    if (lv_->buffer()->isReadonly()) return;

    string contents;
    for (unsigned int i = 0; i < citekeys.size(); ++i) {
	if (i > 0) contents += ",";
	contents += citekeys[i];
    }

    params.setContents(contents);
    params.setOptions( dialog_->textAfterED->text().latin1() );

    if (inset_ != 0) {
	// Only update if contents have changed
	if (params != inset_->params()) {
	    inset_->setParams(params);
	    lv_->view()->updateInset(inset_, true);
	}
    } else {
	lv_->getLyXFunc()->Dispatch(LFUN_CITATION_INSERT,
				    params.getAsString());
    }
}


