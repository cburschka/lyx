/* FormCitation.h
 * (C) 2000 LyX Team
 * John Levon, moz@compsoc.man.ac.uk
 * Changed for Qt2 port by Kalle Dalheimer, kalle@klaralvdalens-datakonsult.se
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
#include "insets/insetcommand.h"

#include <vector> 
 
class Dialogs;
class LyXView;
class FormCitationDialog;

class QListBox;

class FormCitation : public DialogBase {
    friend class FormCitationDialogImpl;
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
private: 
	///
	enum State {
		ON,
		///
		OFF
	};
	/// Create the dialog if necessary, update it and display it.
	void show();
	/// Hide the dialog.
	void hide();
	/// Update the dialog.
	void update();

	void setBibButtons(State status) const;
	void setCiteButtons(State status) const;

	/// create a Citation inset
	void createCitation(string const &);
	/// edit a Citation inset
	void showCitation(InsetCommand * const);
 
	/// update a listbox
	void updateBrowser( QListBox* listbox,
			    vector<string> const & keys) const;
 
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
	
	/// Inset hide connection.
	SigC::Connection ih_;

	std::vector<string> citekeys;
	///
	std::vector<string> bibkeys;
	///
	std::vector<string> bibkeysInfo;
};

#endif
