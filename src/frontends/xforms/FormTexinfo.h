// -*- C++ -*-
/**
 * \file FormTexinfo.h
 * Copyright 2001 the LyX Team
 * Read the file COPYING
 *
 * \author Herbert Voss <voss@perce.de>
 */

#ifndef FORMTEXINFO_H
#define FORMTEXINFO_H

#ifdef __GNUG__
#pragma interface
#endif

#include "FormBase.h"

class ControlTexinfo;
struct FD_form_texinfo;

class FormTexinfo : public FormCB<ControlTexinfo, FormDB<FD_form_texinfo> > {
public:
	///
	FormTexinfo(ControlTexinfo &);
	/// the file extensions
	enum texFileSuffix {cls, sty, bst};

	/// preemptive handler for feedback messages
	void feedbackCB(FL_OBJECT *, int);

private:
	/// not needed
	virtual void apply() {}
	/// Build the dialog.
	virtual void build();
	/// not needed
	virtual void update() {}
	/// Filter the inputs on callback from xforms
	virtual ButtonPolicy::SMInput input(FL_OBJECT *, long);
	///
	void feedback(FL_OBJECT *);
	///
	void updateStyles(texFileSuffix);
	/// Fdesign generated method
	FD_form_texinfo * build_texinfo();
	///
	bool warningPosted;
	///
	texFileSuffix activeStyle;
};

#endif // FORMTEXINFO_H
