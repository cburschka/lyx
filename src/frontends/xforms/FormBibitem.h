/**
 * \file FormBibitem.h
 * Copyright 2001 the LyX Team
 * Read the file COPYING
 *
 * \author Angus Leeming
 * \author John Levon
 */

#ifndef FORMBIBITEM_H
#define FORMBIBITEM_H

#ifdef __GNUG__
#pragma interface
#endif

#include "FormInset.h"
struct FD_form_bibitem;

/**
 * For bibliography entry editing
 */
class FormBibitem : public FormCommand {
public:
	///
	FormBibitem(LyXView *, Dialogs *);
	///
	~FormBibitem();
private:
	/// Connect signals etc. Set form's max size.
	virtual void connect();
	/// Build the dialog
	virtual void build();
	/// Update dialog before showing it
	virtual void update();
	/// input handler
	virtual bool input(FL_OBJECT *,long);
	/// Apply from dialog (modify or create inset)
	virtual void apply();
	/// Pointer to the actual instantiation of the xform's form
	virtual FL_FORM * form() const;
	///
	FD_form_bibitem * build_bibitem();
	/// Real GUI implementation.
	FD_form_bibitem * dialog_;
};

#endif // FORMBIBITEM_H
