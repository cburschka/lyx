/**
 * \file FormSplash.h
 * Copyright 2001 the LyX Team
 * Read the file COPYING
 *
 * \author Allan Rae
 * \author John Levon
 */

#ifndef FORMSPLASH_H
#define FORMSPLASH_H

#include "FormBase.h"

#ifdef __GNUG__
#pragma interface
#endif

struct FD_form_splash;

/** The startup splash screen
 */
class FormSplash : public FormBaseBI {
public:
	FormSplash(LyXView *, Dialogs *);

	~FormSplash();

	/// close the dialog
	static void CloseCB(FL_OBJECT *);
 
private:
	/// show the dialog
	virtual void show(); 
	/// Build the dialog
	virtual void build();
	/// Pointer to the actual instantiation of the xforms form
	virtual FL_FORM * form() const;
	/// Fdesign generated method
	FD_form_splash * build_splash();

	/// Real GUI implementation.
	FD_form_splash * dialog_;
};

#endif // FORMSPLASH_H
