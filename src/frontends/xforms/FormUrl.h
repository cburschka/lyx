/* \file FormUrl.h
 * This file is part of
 * ====================================================== 
 *
 *           LyX, The Document Processor
 *
 *           Copyright 2000-2001 The LyX Team.
 *
 * ======================================================
 *
 * \author Angus Leeming, a.leeming@.ac.uk
 */

#ifndef FORMURL_H
#define FORMURL_H

#ifdef __GNUG__
#pragma interface
#endif

#include "FormBase.h"

class ControlUrl;
struct FD_form_url;

/** This class provides an XForms implementation of the Url Dialog.
 */
class FormUrl : public FormCB<ControlUrl, FormDB<FD_form_url> > {
public:
	///
	FormUrl(ControlUrl &);

private:
	/// Apply from dialog (modify or create inset)
	virtual void apply();
	/// Build the dialog
	virtual void build();
	/// Update dialog before showing it
	virtual void update();
	/// Filter the inputs on callback from xforms
	FD_form_url * build_url();
};

#endif // FORMURL_H
