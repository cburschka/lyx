/**
 * \file FormCitation.C
 * Copyright 2001 the LyX Team
 * Read the file COPYING
 *
 * \author John Levon
 */

#include <config.h>

#include <algorithm>

#include "ControlCitation.h"
#include "FormCitation.h"
#include "citationdlg.h"
#include "gettext.h"
#include "kdeBC.h"
#include "support/lstrings.h"

using std::vector;
using std::pair;
using std::find;

FormCitation::FormCitation(ControlCitation & c)
	: ViewBC<kdeBC>(c), 
	  keys(0), chosenkeys(0)
{}

 
ControlCitation & FormCitation::controller() const
{
	return static_cast<ControlCitation &>(controller_);
	//return dynamic_cast<ControlCitation &>(controller_);
}


void FormCitation::show()
{
	if (!dialog_.get())
		build();

	update();

	dialog_->raise();
	dialog_->setActiveWindow();
	dialog_->show();
}


void FormCitation::apply()
{
	controller().params().setCmdName("cite");
	controller().params().setContents(getStringFromVector(chosenkeys));
	controller().params().setOptions(dialog_->after->text());
}


void FormCitation::hide()
{
	chosenkeys.clear();
	selectedKey.erase();
	selectedChosenKey.erase();

	if (dialog_.get() && dialog_->isVisible())
		dialog_->hide();
}


void FormCitation::build()
{
	dialog_.reset(new CitationDialog(this, 0, "Citation", false));

        // Manage the ok, apply, restore and cancel/close buttons
	bc().setOK(dialog_->buttonOk);
	//bc().setApply(dialog_->buttonApply);
	bc().setCancel(dialog_->buttonCancel);
	//bc().setUndoAll(dialog_->buttonRestore);
	bc().refresh();

	bc().addReadOnly(dialog_->add);
	bc().addReadOnly(dialog_->remove);
	bc().addReadOnly(dialog_->up);
	bc().addReadOnly(dialog_->down);
	//bc().addReadOnly(dialog_->style);
	//bc().addReadOnly(dialog_->labelbefore);
	bc().addReadOnly(dialog_->labelafter);
}


void FormCitation::update()
{
	keys = controller().getBibkeys();
	updateAvailableList();
	selectedKey.erase();

	chosenkeys = getVectorFromString(controller().params().getContents());
	updateChosenList();
	selectedChosenKey.erase();

	dialog_->entry->setText("");
	dialog_->after->setText(controller().params().getOptions().c_str());

	updateButtons();

	if (controller().isReadonly()) {
		dialog_->keys->setFocusPolicy(QWidget::NoFocus);
		dialog_->chosen->setFocusPolicy(QWidget::NoFocus);
		dialog_->after->setFocusPolicy(QWidget::NoFocus);
	} else {
		dialog_->keys->setFocusPolicy(QWidget::StrongFocus);
		dialog_->chosen->setFocusPolicy(QWidget::StrongFocus);
		dialog_->after->setFocusPolicy(QWidget::StrongFocus);
	}
}


void FormCitation::updateButtons()
{
	// Can go once ButtonController is working?
	if (controller().isReadonly()) {
		dialog_->add->setEnabled(false);
		dialog_->remove->setEnabled(false);
		dialog_->up->setEnabled(false);
		dialog_->down->setEnabled(false);
		return;
	}

	bool ischosenkey = !selectedChosenKey.empty();

	vector<string>::const_iterator iter =
		find(chosenkeys.begin(), chosenkeys.end(), selectedKey);

	dialog_->add->setEnabled(!selectedKey.empty() && iter == chosenkeys.end());
	dialog_->remove->setEnabled(ischosenkey);
	dialog_->up->setEnabled(ischosenkey);
	dialog_->down->setEnabled(ischosenkey);
}


void FormCitation::updateChosenList()
{
	updateList(dialog_->chosen, chosenkeys);
}


void FormCitation::updateAvailableList()
{
	updateList(dialog_->keys, keys);
}


void FormCitation::updateList(QListBox * lb, vector<string> const & keys)
{
	lb->setAutoUpdate(false);
	lb->clear();

	for (vector<string>::const_iterator iter = keys.begin();
		iter != keys.end(); ++iter) {
		lb->insertItem(iter->c_str());
	}
	lb->setAutoUpdate(true);
	lb->update();
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

ButtonPolicy::SMInput FormCitation::add()
{
	if (selectedKey.empty())
		return ButtonPolicy::SMI_NOOP;

	for (vector<string>::const_iterator iter = keys.begin();
		iter != keys.end(); ++iter) {
		if (*iter == selectedKey) {
			chosenkeys.push_back(*iter);
			break;
		}
	}
		
	selectedChosenKey.erase();
	updateChosenList();
	updateButtons();
	return ButtonPolicy::SMI_VALID;
}

ButtonPolicy::SMInput FormCitation::remove()
{
	if (selectedChosenKey.empty())
		return ButtonPolicy::SMI_NOOP;

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
	return ButtonPolicy::SMI_VALID;
}

ButtonPolicy::SMInput FormCitation::up()
{
	if (selectedChosenKey.empty())
		return ButtonPolicy::SMI_NOOP;

	// Qt will select the first one on redo, so we need this
	string tmp = selectedChosenKey;

	vector< string >::iterator iter = chosenkeys.begin();

	for (; iter != chosenkeys.end(); ++iter) {
		if (*iter==selectedChosenKey && iter!=chosenkeys.begin()) {
			string tmp = *iter;
			chosenkeys.erase(iter);
			chosenkeys.insert(iter-1,tmp);
			break;
		}
	}
	if (iter==chosenkeys.end())
		return ButtonPolicy::SMI_NOOP;

	updateChosenList();
	selectedChosenKey=tmp;
	selectChosen();
	return ButtonPolicy::SMI_VALID;
}

 
ButtonPolicy::SMInput FormCitation::down()
{
	if (selectedChosenKey.empty())
		return ButtonPolicy::SMI_NOOP;

	// Qt will select the first one on redo, so we need this
	string tmp = selectedChosenKey;

	vector< string >::iterator iter = chosenkeys.begin();

	for (; iter != chosenkeys.end(); ++iter) {
		if (*iter == selectedChosenKey && (iter+1)!=chosenkeys.end()) {
			string tmp = *iter;
			chosenkeys.erase(iter);
			chosenkeys.insert(iter+1, tmp);
			break;
		}
	}
	if (iter == chosenkeys.end())
		return ButtonPolicy::SMI_NOOP;

	updateChosenList();
	selectedChosenKey=tmp;
	selectChosen();
	return ButtonPolicy::SMI_VALID;	
}

 
ButtonPolicy::SMInput FormCitation::select_key(char const * key)
{
	if (controller().isReadonly())
		return ButtonPolicy::SMI_INVALID;

	vector<string>::const_iterator iter =
		find(chosenkeys.begin(), chosenkeys.end(), key);

	if (iter != chosenkeys.end())
		return ButtonPolicy::SMI_NOOP;

	selectedKey.erase();
	selectedKey = key;

	add();
	return ButtonPolicy::SMI_VALID;	
}

 
void FormCitation::highlight_key(char const * key)
{
	highlight(key, dialog_->chosen, selectedKey, selectedChosenKey);
}

 
void FormCitation::highlight_chosen(char const * key)
{
	highlight(key, dialog_->keys, selectedChosenKey, selectedKey);
}


void FormCitation::highlight(char const * key, QListBox * lb,
			     string & selected1, string & selected2)
{
	selected1.erase();
	selected1 = key;

	unsigned int i;

	for (i=0; i < keys.size(); ++i) {
		if (keys[i] == key) {
			string const tmp = controller().getBibkeyInfo(key);
			dialog_->entry->setText(tmp.c_str());
			lb->clearFocus();
			lb->clearSelection();
			selected2.erase();
			break;
		}
	}

	if (i == keys.size())
		dialog_->entry->setText(_("Key not found."));

	updateButtons();
}
