// -*- C++ -*-
/**
 * \file FormNote.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Martin Vermeer
 *
 * Full author contact details are available in file CREDITS
 */

#ifndef FORMNOTE_H
#define FORMNOTE_H


#include "FormDialogView.h"


class ControlNote;
struct FD_note;

/** This class provides an XForms implementation of the Note Dialog.
 */
class FormNote : public FormController<ControlNote, FormView<FD_note> > {
public:
	/// Constructor
	FormNote(Dialog &);
private:
	/// 
	virtual void apply();
	/// Build the dialog
	virtual void build();
	/// Update dialog before showing it
	virtual void update();
	///
	string const predefineds() const;
};

#endif // FORMNOTE_H
