/* This file is part of
 * ======================================================
 * 
 *           LyX, The Document Processor
 * 	 
 *          Copyright 2001 The LyX Team.
 *
 *======================================================
 *
 * \file FormMinipage.h
 * \author Juergen Vigna, jug@sad.it
 */

#ifndef FORMMINIPAGE_H
#define FORMMINIPAGE_H

#ifdef __GNUG__
#pragma interface
#endif

#include "FormBase.h"

class LyXView;
class Dialogs;
class InsetMinipage;
struct FD_form_minipage;

/** This class provides an XForms implementation of the Minipage
    Dialog.
 */
class ControlMinipage;
struct FD_form_minipage;

class FormMinipage : public FormCB<ControlMinipage, FormDB<FD_form_minipage> > {
public:
	///
	FormMinipage(ControlMinipage &);

private:
	/// Set the Params variable for the Controller.
	virtual void apply();
	/// Build the dialog.
	virtual void build();
	/// Update dialog before/whilst showing it.
	virtual void update();

	/// Fdesign generated method
	FD_form_minipage * build_minipage();
};

#endif // FORMMINIPAGE_H
