/**
 * \file FormCitation.C
 * Copyright 2001 the LyX Team
 * Read the file COPYING
 *
 * \author John Levon
 * \author Angus Leeming 
 */

#include <config.h>
 
#include <algorithm>

#include "FormCitation.h"
#include "ControlCitation.h"
#include "citationdlg.h"
#include "gettext.h"
#include "biblio.h" 
#include "support/lstrings.h"
#include "helper_funcs.h" 

using std::vector;
using std::pair;
using std::find;

FormCitation::FormCitation(ControlCitation & c)
	: KFormBase<ControlCitation, CitationDialog>(c),
	  keys(0), chosenkeys(0)
{
}

 
void FormCitation::apply()
{
	controller().params().setCmdName("cite");
	controller().params().setContents(getStringFromVector(chosenkeys));
	controller().params().setOptions(dialog_->line_after->text());
}


void FormCitation::hide()
{
	chosenkeys.clear();
	selectedKey.erase();
	selectedChosenKey.erase();

	KFormBase<ControlCitation, CitationDialog>::hide();
}


void FormCitation::build()
{
	dialog_.reset(new CitationDialog(this, 0, _("Citation")));

        // Manage the ok, apply, restore and cancel/close buttons
	bc().setOK(dialog_->button_ok);
	bc().setApply(dialog_->button_apply);
	bc().setRestore(dialog_->button_restore);
	bc().setCancel(dialog_->button_cancel);

	bc().addReadOnly(dialog_->list_available);
	bc().addReadOnly(dialog_->list_chosen);
	bc().addReadOnly(dialog_->line_after); 
	bc().addReadOnly(dialog_->button_add);
	bc().addReadOnly(dialog_->button_remove);
	bc().addReadOnly(dialog_->button_up);
	bc().addReadOnly(dialog_->button_down);
	//bc().addReadOnly(dialog_->style);
}


void FormCitation::update()
{
	keys = biblio::getKeys(controller().bibkeysInfo());

	updateAvailableList();
	selectedKey.erase();

	chosenkeys = getVectorFromString(controller().params().getContents());
	updateChosenList();
	selectedChosenKey.erase();

	dialog_->line_details->setText("");
	dialog_->line_after->setText(controller().params().getOptions().c_str());

	if (!controller().isReadonly()) 
		updateButtons();
}


void FormCitation::updateButtons()
{
	bool ischosenkey = !selectedChosenKey.empty();

	vector<string>::const_iterator iter =
		find(chosenkeys.begin(), chosenkeys.end(), selectedKey);

	dialog_->button_add->setEnabled(!selectedKey.empty() && iter == chosenkeys.end());
	dialog_->button_remove->setEnabled(ischosenkey);
	dialog_->button_up->setEnabled(ischosenkey);
	dialog_->button_down->setEnabled(ischosenkey);
}


void FormCitation::updateChosenList()
{
	updateList(dialog_->list_chosen, chosenkeys);
}


void FormCitation::updateAvailableList()
{
	updateList(dialog_->list_available, keys);
}


void FormCitation::updateList(QListBox * lb, vector<string> const & keys)
{
	lb->setAutoUpdate(false);
	lb->clear();

	for (vector<string>::const_iterator iter = keys.begin();
		iter != keys.end(); ++iter) {
		if (!iter->empty()) 
			lb->insertItem(iter->c_str());
	}
	lb->setAutoUpdate(true);
	lb->update();
}


void FormCitation::selectChosen()
{
	for (unsigned int i=0; i < dialog_->list_chosen->count(); ++i) {
		if (dialog_->list_chosen->text(i)==selectedChosenKey) {
			dialog_->list_chosen->setSelected(i,true);
			dialog_->list_chosen->setTopItem(i);
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
	highlight(key, dialog_->list_chosen, selectedKey, selectedChosenKey);
}

 
void FormCitation::highlight_chosen(char const * key)
{
	highlight(key, dialog_->list_available, selectedChosenKey, selectedKey);
}


void FormCitation::highlight(char const * key, QListBox * lb,
			     string & selected1, string & selected2)
{
	selected1.erase();
	selected1 = key;

	unsigned int i;

	for (i=0; i < keys.size(); ++i) {
		if (keys[i] == key) {
			string const tmp = biblio::getInfo(controller().bibkeysInfo(), key);
			dialog_->line_details->setText(tmp.c_str());
			lb->clearFocus();
			lb->clearSelection();
			selected2.erase();
			break;
		}
	}

	if (i == keys.size())
		dialog_->line_details->setText(_("Key not found."));

	updateButtons();
}
