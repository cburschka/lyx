// -*- C++ -*-
/* This file is part of
 * ====================================================== 
 *
 *           LyX, The Document Processor
 *
 *           Copyright 2000 The LyX Team.
 *
 * ======================================================
 */

#ifndef FORMCITATION_H
#define FORMCITATION_H

#include "DialogBase.h"
#include "LString.h"
#include <vector>

class Dialogs;
// same arguement as in Dialogs.h s/LyX/UI/
class LyXView;
class InsetCitation;
struct FD_form_citation;

/** This class provides an XForms implementation of the FormCitation Dialog.
 */
class FormCitation : public DialogBase {
public:
	///
	enum State {
		DOWN,
		UP,
		DELETE,
		ADD,
		BIBBRSR,
		CITEBRSR,
		ON,
		OFF
	};
	/**@name Constructors and Destructors */
	//@{
	/// #FormCitation x(LyXFunc ..., Dialogs ...);#
	FormCitation(LyXView *, Dialogs *);
	///
	~FormCitation();
	//@}

	/**@name Real per-instance Callback Methods */
	//@{
	static  int WMHideCB(FL_FORM *, void *);
	static void OKCB(FL_OBJECT *, long);
	static void CancelCB(FL_OBJECT *, long);
	static void InputCB(FL_OBJECT *, long);
	//@}

private:
	FormCitation() {}
	FormCitation(FormCitation &) : DialogBase() {}
	
	/**@name Slot Methods */
	//@{
	/// Create the dialog if necessary, update it and display it.
	void createInset( string const & );
	/// 
	void showInset( InsetCitation * );
	/// 
	void hideInset( InsetCitation * );
	/// 
	void show();
	/// Hide the dialog.
	void hide();
	/// Not used but we've got to implement it.
	void update();
	//@}

	/**@name Dialog internal methods */
	//@{
	/// Apply from dialog
	void apply();
	/// Filter the inputs
	void input( State );
	/// Build the dialog
	void build();
	///
	void updateCitekeys( string const & );
	///
	void updateBrowser( FL_OBJECT *, std::vector<string> const & ) const;
	///
	void setBibButtons( State ) const;
	///
	void setCiteButtons( State ) const;
	///
	void setSize( int, bool ) const;
	///
	FD_form_citation * build_citation();
	/// Explicitly free the dialog.
	void free();
	//@}

	/**@name Private Data */
	//@{
	/// Real GUI implementation.
	FD_form_citation * dialog_;
	/** Which LyXFunc do we use?
	    We could modify Dialogs to have a visible LyXFunc* instead and
	    save a couple of bytes per dialog.
	*/
	LyXView * lv_;
	/** Which Dialogs do we belong to?
	    Used so we can get at the signals we have to connect to.
	*/
	Dialogs * d_;
	/// Update connection.
	Connection u_;
	/// Hide connection.
	Connection h_;
	///
	InsetCitation * inset_;
	///
	bool dialogIsOpen;
	///
  	string textAfter;
	///
	std::vector<string> citekeys;
	///
	std::vector<string> bibkeys;
	///
	std::vector<string> bibkeysInfo;
	//@}
};

#endif
