// -*- C++ -*-
/**
 * \file FormBrowser.h
 * Read the file COPYING
 *
 * \author John Levon
 * \author Angus Leeming 
 *
 * Full author contact details are available in file CREDITS
 */

#ifndef FORMBROWSER_H
#define FORMBROWSER_H

#ifdef __GNUG__
#pragma interface
#endif

#include "FormBase.h"

/**
 * This class provides an XForms implementation of a read only
 * text browser.
 */
struct FD_browser;

class FormBrowser : public FormDB<FD_browser> {
public:
	///
	FormBrowser(string const &, bool allowResize = true);
private:
	/// Build the dialog.
	virtual void build();
};

#endif // FORMBROWSER_H
