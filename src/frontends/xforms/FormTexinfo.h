// -*- C++ -*-
/**
 * \file FormTexinfo.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Herbert Voss
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef FORMTEXINFO_H
#define FORMTEXINFO_H


#include "FormDialogView.h"
#include "ControlTexinfo.h"
#include <map>

struct FD_texinfo;

class FormTexinfo
	: public FormController<ControlTexinfo, FormView<FD_texinfo> > {
public:
	///
	FormTexinfo(Dialog &);
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

	///
	typedef std::vector<string> ContentsType;
	std::map<ControlTexinfo::texFileSuffix, ContentsType> texdata_;
};

#endif // FORMTEXINFO_H
