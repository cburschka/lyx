// -*- C++ -*-
/**
 * \file FormExternal.h
 * Copyright 2000-2001 the LyX Team
 * Read the file COPYING
 *
 * \author Asger Alstrup Nielsen
 * \author John Levon, moz@compsoc.man.ac.uk
 * \author Angus Leeming, a.leeming@ic.ac.uk
 */

#ifndef FORMEXTERNAL_H
#define FORMEXTERNAL_H

#ifdef __GNUG__
#pragma interface
#endif

#include "FormBase.h"
#include "insets/insetexternal.h"

class ControlExternal;
struct FD_external;

/// The class for editing External insets via a dialog
class FormExternal : public FormCB<ControlExternal, FormDB<FD_external> > {
public:
	///
	FormExternal();
private:
	/// apply changes
	void apply();

	/// build the dialog
	void build();

	/// update the dialog
	void update();

	/// Filter the inputs on callback from xforms
	ButtonPolicy::SMInput input(FL_OBJECT *, long);

	///
	void updateComboChange();
};

#endif // FORMEXTERNAL_H
