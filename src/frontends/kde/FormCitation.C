/*
 * FormCitation.C
 * (C) 2000 LyX Team
 * John Levon, moz@compsoc.man.ac.uk
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

#include "Dialogs.h"
#include "FormCitation.h"
#include "gettext.h"
#include "buffer.h"
#include "LyXView.h"
#include "lyxfunc.h"
#include "formcitationdialog.h"
 
using std::vector;
using std::pair;
using std::find;
 
FormCitation::FormCitation(LyXView *v, Dialogs *d)
	: dialog_(0), lv_(v), d_(d), inset_(0), h_(0), u_(0), ih_(0), 
	keys(0), chosenkeys(0)
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

void FormCitation::showCitation(InsetCommand * const inset)
{
	// FIXME: when could inset be 0 here ?
	if (inset==0)
		return;

	inset_ = inset;
	readonly = lv_->buffer()->isReadonly();
	ih_ = inset_->hide.connect(slot(this,&FormCitation::hide));
	params = inset->params();
	
	show();
}

void FormCitation::createCitation(string const & arg)
{
	// we could already be showing stuff, clear it out
	if (inset_)
		close();

	readonly = lv_->buffer()->isReadonly();
	params.setFromString(arg);
	show();
}

void FormCitation::updateButtons()
{ 
	bool iskey,ischosenkey;

	iskey = !selectedKey.empty();
	ischosenkey = !selectedChosenKey.empty();
 
	dialog_->remove->setEnabled(ischosenkey);
	dialog_->add->setEnabled(iskey);
	dialog_->up->setEnabled(ischosenkey);
	dialog_->down->setEnabled(ischosenkey);
}
 
void FormCitation::updateChosenList()
{
	dialog_->chosen->setAutoUpdate(false);
	dialog_->chosen->clear();

	for (vector< string >::const_iterator iter = chosenkeys.begin();
		iter != chosenkeys.end(); ++iter) {
		dialog_->chosen->insertItem(iter->c_str());
	}
	dialog_->chosen->setAutoUpdate(true);
	dialog_->chosen->update();
}

void FormCitation::updateAvailableList()
{
	dialog_->keys->setAutoUpdate(false);
	dialog_->keys->clear();

	for (vector< pair<string,string> >::const_iterator iter = keys.begin();
		iter != keys.end(); ++iter) {
		dialog_->keys->insertItem(iter->first.c_str());
	}
	dialog_->keys->setAutoUpdate(true);
	dialog_->keys->update();
}
 
void FormCitation::update()
{
	keys.clear();
 
	vector < pair<string,string> > ckeys = lv_->buffer()->getBibkeyList(); 

	for (vector< pair<string,string> >::const_iterator iter = ckeys.begin();
		iter != ckeys.end(); ++iter) {
		keys.push_back(*iter);
	}
 
	updateAvailableList();
	selectedKey.erase();
 
	chosenkeys.clear();
 
	string tmp, paramkeys(params.getContents());
	paramkeys = frontStrip(split(paramkeys, tmp, ','));

	while (!tmp.empty()) {
		chosenkeys.push_back(tmp);
		paramkeys = frontStrip(split(paramkeys, tmp, ','));
	}

	updateChosenList();
	selectedChosenKey.erase();
 
	dialog_->entry->setText("");

	dialog_->after->setText(params.getOptions().c_str());

	updateButtons();
 
	if (readonly) {
		dialog_->keys->setFocusPolicy(QWidget::NoFocus);
		dialog_->chosen->setFocusPolicy(QWidget::NoFocus);
		dialog_->after->setFocusPolicy(QWidget::NoFocus);
		dialog_->buttonOk->setEnabled(false);
		dialog_->buttonCancel->setText(_("Close"));
	} else {
		dialog_->keys->setFocusPolicy(QWidget::StrongFocus);
		dialog_->chosen->setFocusPolicy(QWidget::StrongFocus);
		dialog_->after->setFocusPolicy(QWidget::StrongFocus);
		dialog_->buttonOk->setEnabled(true);
		dialog_->buttonCancel->setText(_("Cancel"));
	}
}

void FormCitation::apply()
{
	if (readonly)
		return;

	string contents;

	for (vector< string >::const_iterator iter = chosenkeys.begin();
		iter != chosenkeys.end(); ++iter) {
		if (iter != chosenkeys.begin())
			contents += ", ";
		contents += *iter;
	}
		
	params.setContents(contents);
	params.setOptions(dialog_->after->text());
	 
	if (inset_ != 0) {
		if (params != inset_->params()) {
			inset_->setParams(params);
			lv_->view()->updateInset(inset_, true);
		}
	} else
		lv_->getLyXFunc()->Dispatch(LFUN_CITATION_INSERT, params.getAsString().c_str());
}

void FormCitation::show()
{
	if (!dialog_)
		dialog_ = new FormCitationDialog(this, 0, _("LyX: Citation Reference"), false);

	if (!dialog_->isVisible()) {
		h_ = d_->hideBufferDependent.connect(slot(this, &FormCitation::hide));
		u_ = d_->updateBufferDependent.connect(slot(this, &FormCitation::update));
	}

	dialog_->raise();
	dialog_->setActiveWindow();

	update();
	dialog_->show();
}

void FormCitation::close()
{
	h_.disconnect();
	u_.disconnect();
	ih_.disconnect();
	inset_ = 0;
}

void FormCitation::hide()
{
	dialog_->hide();
	close();
}

void FormCitation::selectChosen()
{
	for (unsigned int i=0; i < dialog_->chosen->count(); ++i) {
		if (dialog_->chosen->text(i)==selectedChosenKey) {
			dialog_->chosen->setSelected(i,true);
			dialog_->chosen->setTopItem(i);
			break;
		}
	}
}
 
void FormCitation::add()
{
	if (selectedKey.empty())
		return;

	for (vector< pair<string,string> >::const_iterator iter = keys.begin(); 
		iter != keys.end(); ++iter) {
		if (iter->first == selectedKey) {
			chosenkeys.push_back(iter->first);
			break; 
		}
	}
		 
	selectedChosenKey.erase();
	updateChosenList();
	updateButtons();
}

void FormCitation::remove()
{
	if (selectedChosenKey.empty())
		return; 

	for (vector< string >::iterator iter = chosenkeys.begin();
		iter != chosenkeys.end(); ++iter) {
		if (*iter==selectedChosenKey) {
			chosenkeys.erase(iter);
			break;
		}
	}
	selectedChosenKey.erase();
	updateChosenList();
	updateButtons();
}

void FormCitation::up()
{
	if (selectedChosenKey.empty())
		return;

	// Qt will select the first one on redo, so we need this 
	string tmp = selectedChosenKey;
 
	for (vector< string >::iterator iter = chosenkeys.begin();
		iter != chosenkeys.end(); ++iter) {
		if (*iter==selectedChosenKey && iter!=chosenkeys.begin()) {
			string tmp = *iter;
			chosenkeys.erase(iter);
			chosenkeys.insert(iter-1,tmp);
			break;
		}
	}
	if (iter==chosenkeys.end())
		return;
 
	updateChosenList();
	selectedChosenKey=tmp;
	selectChosen();
}

void FormCitation::down()
{
	if (selectedChosenKey.empty())
		return;

	// Qt will select the first one on redo, so we need this 
	string tmp = selectedChosenKey;
 
	for (vector< string >::iterator iter = chosenkeys.begin();
		iter != chosenkeys.end(); ++iter) {
		if (*iter==selectedChosenKey && (iter+1)!=chosenkeys.end()) {
			string tmp = *iter;
			chosenkeys.erase(iter);
			chosenkeys.insert(iter+1, tmp);
			break;
		}
	}
	if (iter==chosenkeys.end())
		return;
 
	updateChosenList();
	selectedChosenKey=tmp;
	selectChosen();
}
 
void FormCitation::select_key(const char *key)
{
	selectedKey.erase();
	selectedKey = key;

	add();
}
 
void FormCitation::highlight_key(const char *key)
{
	selectedKey.erase();
	selectedKey = key;

	for (unsigned int i=0; i < keys.size(); i++) {
		if (keys[i].first==key) {
			dialog_->entry->setText(keys[i].second.c_str());
			break;
		}
	}

	updateButtons();
}

void FormCitation::highlight_chosen(const char *key)
{
	selectedChosenKey.erase();
	selectedChosenKey = key;
 
	unsigned int i;
	for (i=0; i < keys.size(); i++) {
		if (keys[i].first==key && keys[i].second.compare(dialog_->entry->text())) {
			dialog_->entry->setText(keys[i].second.c_str());
			break;
		}
	}

	if (i==keys.size())
		dialog_->entry->setText(_("Key not found in references."));
 
	updateButtons();
}
