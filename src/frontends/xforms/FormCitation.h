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

#ifdef __GNUG__
#pragma interface
#endif

#include "FormInset.h"
struct FD_form_citation;

/** This class provides an XForms implementation of the FormCitation Dialog.
 */
class FormCitation : public FormCommand {
public:
	///
	FormCitation(LyXView *, Dialogs *);
	///
	~FormCitation();
private:
	///
	enum State {
		///
		DOWN,
		///
		UP,
		///
		DELETE,
		///
		ADD,
		///
		BIBBRSR,
		///
		CITEBRSR,
		///
		ON,
		///
		OFF
	};
	/// Connect signals etc. Set form's max size.
	virtual void connect();
	/// Disconnect signals. Also perform any necessary housekeeping.
	virtual void disconnect();

	/// Build the dialog
	virtual void build();
	/// Filter the inputs
	virtual bool input( FL_OBJECT *, long );
	/// Update dialog before showing it
	virtual void update();
	/// Apply from dialog (modify or create inset)
	virtual void apply();
	/// Pointer to the actual instantiation of the xform's form
	virtual FL_FORM * form() const;

	///
	void updateBrowser( FL_OBJECT *, std::vector<string> const & ) const;
	///
	void setBibButtons( State ) const;
	///
	void setCiteButtons( State ) const;
	///
	void setSize( int, bool ) const;
	/// Type definition from the fdesign produced header file.
	FD_form_citation * build_citation();

	/// Real GUI implementation.
	FD_form_citation * dialog_;
	///
	std::vector<string> citekeys;
	///
	std::vector<string> bibkeys;
	///
	std::vector<string> bibkeysInfo;
};

#endif
