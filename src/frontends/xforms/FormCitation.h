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

#include "FormCommand.h"
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
	/// Build the dialog
	virtual void build();
	/// Filter the inputs
	virtual bool input( long );
	/// Update dialog before showing it
	virtual void update();
	/// Apply from dialog (modify or create inset)
	virtual void apply();
	/// delete derived class variables from hide()
	virtual void clearStore();
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
	///
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
