/**
 * \file FormCitation.h
 * Copyright 2001 the LyX Team
 * Read the file COPYING
 *
 * \author John Levon
 * \author Angus Leeming
 */

#ifndef FORMCITATION_H
#define FORMCITATION_H

#include <vector>

#include "KFormBase.h"

class QListBox;
class ControlCitation;
class CitationDialog;

class FormCitation : public KFormBase<ControlCitation, CitationDialog> {
public:
	FormCitation(ControlCitation &);

	// Functions accessible to the Controller

	/// set the Params variable for the Controller
	virtual void apply();
	/// set the Params variable for the Controller
	virtual void build();
	/// update dialog before/whilst showing it
	virtual void update();
	/// hide the dialog
	virtual void hide(); 

	/// add a key
	ButtonPolicy::SMInput add();
	/// remove a key
	ButtonPolicy::SMInput remove();
	/// move a key up
	ButtonPolicy::SMInput up();
	/// move a key down
	ButtonPolicy::SMInput down();
	/// a key has been highlighted
	void highlight_key(char const * key);
	/// a chosen key has been highlighted
	void highlight_chosen(char const * key);
	/// a key has been double-clicked
	ButtonPolicy::SMInput select_key(char const * key);

	friend class CitationDialog;
 
private:
	/// update add,remove,up,down
	void updateButtons();
	/// update the available keys list
	void updateAvailableList();
	/// update the chosen keys list
	void updateChosenList();
	/// select the currently chosen key
	void selectChosen();
	/// does the dirty work for highlight_key(), highlight_chosen()
	void highlight(char const *, QListBox *, string &, string &);
	/// does the dirty work for updateAvailableList(), updateChosenList()
	void updateList(QListBox *, std::vector<string> const &);

	/// available citation keys
	std::vector<string> keys;
	/// chosen citation keys
	std::vector<string> chosenkeys;

	/// currently selected key
	string selectedKey;

	/// currently selected chosen key
	string selectedChosenKey;
};

#endif // FORMCITATION_H
