/* FormCitation.h
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

#ifndef FORMCITATION_H
#define FORMCITATION_H

#include "DialogBase.h"
#include "support/lstrings.h"
#include "boost/utility.hpp"
#include "insets/insetcommand.h" 

#include <vector> 
 
class Dialogs;
class LyXView;
class FormCitationDialog;

class FormCitation : public DialogBase, public noncopyable {
public: 
	/**@name Constructors and Destructors */
	//@{
	///
	FormCitation(LyXView *, Dialogs *);
	/// 
	~FormCitation();
	//@}

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
	void highlight_key(const char *key);
	/// a chosen key has been highlighted
	void highlight_chosen(const char *key); 
	/// a key has been double-clicked
	void select_key(const char *key);
 
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
	FormCitationDialog * dialog_;

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

#endif
