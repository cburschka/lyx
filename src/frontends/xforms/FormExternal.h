/* This file is part of
 * ====================================================== 
 *
 *           LyX, The Document Processor
 *
 *           Copyright 2000-2001 The LyX Team.
 *
 * ======================================================
 *
 * \file FormExternal.h
 * \author Asger Alstrup
 * \author John Levon
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
struct FD_form_external;

/// The class for editing External insets via a dialog
class FormExternal : public FormCB<ControlExternal, FormDB<FD_form_external> > {
public:
	///
	FormExternal(ControlExternal &);

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

	/// Fdesign generated method
	FD_form_external * build_external();
};

#endif // FORMEXTERNAL_H
