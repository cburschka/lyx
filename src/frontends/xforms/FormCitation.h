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
class FormCitation : public FormCommand, public noncopyable {
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

private:
	/**@name Slot Methods */
	//@{
	virtual void update();
	/// Apply from dialog
	virtual void apply();
	/// Filter the inputs
	virtual void input( long );
	/// Build the dialog
	virtual void build();
	///
	virtual FL_FORM * const form() const;
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
	//@}

	/// Real GUI implementation.
	FD_form_citation * dialog_;
};

#endif
