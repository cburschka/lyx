// -*- C++ -*-
/**
 * \file FormTexinfo.h
 * Copyright 2001 the LyX Team
 * Read the file COPYING
 *
 * \author Herbert Voss, voss@perce.de
 */

#ifndef FORMTEXINFO_H
#define FORMTEXINFO_H

#ifdef __GNUG__
#pragma interface
#endif

#include "FormBase.h"
#include "ControlTexinfo.h"

struct FD_texinfo;

class FormTexinfo : public FormCB<ControlTexinfo, FormDB<FD_texinfo> > {
public:
	///
	FormTexinfo();
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
	void updateStyles(ControlTexinfo::texFileSuffix);
	///
	ControlTexinfo::texFileSuffix activeStyle;
};

#endif // FORMTEXINFO_H
