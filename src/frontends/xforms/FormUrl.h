// -*- C++ -*-
/**
 * \file xforms/FormUrl.h
 * Copyright 2000-2001 the LyX Team
 * Read the file COPYING
 *
 * \author Angus Leeming, a.leeming@ic.ac.uk
 */

#ifndef FORMURL_H
#define FORMURL_H

#ifdef __GNUG__
#pragma interface
#endif

#include "FormBase.h"

class ControlUrl;
struct FD_url;

/** This class provides an XForms implementation of the Url Dialog.
 */
class FormUrl : public FormCB<ControlUrl, FormDB<FD_url> > {
public:
	///
	FormUrl(ControlUrl &, Dialogs &);
private:
	/// Apply from dialog (modify or create inset)
	virtual void apply();
	/// Build the dialog
	virtual void build();
	/// Update dialog before showing it
	virtual void update();
};

#endif // FORMURL_H
