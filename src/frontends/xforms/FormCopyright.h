/**
 * \file FormCopyright.h
 * This file is part of
 * ====================================================== 
 *
 *           LyX, The Document Processor
 *
 *           Copyright 1995 Matthias Ettrich
 *           Copyright 1995-2001 The LyX Team.
 *
 *           This file Copyright 2000-2001
 *           Allan Rae
 * ======================================================
 *
 * \author Allan Rae
 * \author Angus Leeming, a.leeming@.ac.uk
 */

#ifndef FORMCOPYRIGHT_H
#define FORMCOPYRIGHT_H

#ifdef __GNUG__
#pragma interface
#endif

#include "FormBase.h"

class ControlCopyright;
struct FD_form_copyright;

/** This class provides an XForms implementation of the FormCopyright Dialog.
 */
class FormCopyright
	: public FormCB<ControlCopyright, FormDB<FD_form_copyright> > {
public:
	///
	FormCopyright(ControlCopyright &);

private:
	/// not needed.
	virtual void apply() {}
	/// not needed.
	virtual void update() {}
	/// Build the dialog
	virtual void build();

	/// Fdesign generated method
	FD_form_copyright * build_copyright();
};

#endif // FORMCOPYRIGHT_H
