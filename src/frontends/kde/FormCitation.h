/**
 * \file FormCitation.h
 * Copyright 2001 the LyX Team
 * Read the file COPYING
 *
 * \author John Levon
 */

#ifndef FORMCITATION_H
#define FORMCITATION_H

#include "DialogBase.h"
#include "support/lstrings.h"
#include "boost/utility.hpp"
#include "insets/insetcommand.h"

#include <vector>

class Dialogs;
class LyXView;
class CitationDialog;

class FormCitation : public DialogBase {
public:
	FormCitation(LyXView *, Dialogs *);

	~FormCitation();

	/// Apply changes
	void apply();
	/// close the connections
	void close();
	/// add a key
	void add();
	/// remove a key
	void remove();
	/// move a key up
	void up();
	/// move a key down
	void down();
	/// a key has been highlighted
	void highlight_key(char const * key);
	/// a chosen key has been highlighted
	void highlight_chosen(char const * key);
	/// a key has been double-clicked
	void select_key(char const * key);

private:
	/// Create the dialog if necessary, update it and display it.
	void show();
	/// Hide the dialog.
	void hide();
	/// Update the dialog.
	void update(bool switched = false);

	/// create a Citation inset
	void createCitation(string const &);
	/// edit a Citation inset
	void showCitation(InsetCommand * const);

	/// update add,remove,up,down
	void updateButtons();
	/// update the available keys list
	void updateAvailableList();
	/// update the chosen keys list
	void updateChosenList();
	/// select the currently chosen key
	void selectChosen();

	/// Real GUI implementation.
	CitationDialog * dialog_;

	/// the LyXView we belong to
	LyXView * lv_;

	/** Which Dialogs do we belong to?
	    Used so we can get at the signals we have to connect to.
	*/
	Dialogs * d_;
	/// pointer to the inset if any
	InsetCommand * inset_;
	/// insets params
	InsetCommandParams params;
	/// is the inset we are reading from a readonly buffer ?
	bool readonly;
	
	/// Hide connection.
	Connection h_;
	/// Update connection.
	Connection u_;
	/// Inset hide connection.
	Connection ih_;

	/// available citation keys
	std::vector<std::pair<string, string> > keys;
	/// chosen citation keys
	std::vector<string> chosenkeys;

	/// currently selected key
	string selectedKey;

	/// currently selected chosen key
	string selectedChosenKey;
};

#endif // FORMCITATION_H
