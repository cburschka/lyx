// -*- C++ -*-
/**
 * \file FormExternal.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Asger Alstrup Nielsen
 * \author John Levon
 * \author Angus Leeming
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef FORMEXTERNAL_H
#define FORMEXTERNAL_H


#include "FormDialogView.h"


class ControlExternal;
struct FD_external;

/// The class for editing External insets via a dialog
class FormExternal
	: public FormController<ControlExternal, FormView<FD_external> > {
public:
	///
	FormExternal(Dialog &);
private:
	/// apply changes
	virtual void apply();

	/// build the dialog
	virtual void build();

	/// update the dialog
	virtual void update();

	/// Filter the inputs on callback from xforms
	virtual ButtonPolicy::SMInput input(FL_OBJECT *, long);

	///
	void updateComboChange();
};

#endif // FORMEXTERNAL_H
