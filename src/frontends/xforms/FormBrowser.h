// -*- C++ -*-
/**
 * \file FormBrowser.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author John Levon
 * \author Angus Leeming
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef FORMBROWSER_H
#define FORMBROWSER_H

#include "FormDialogView.h"

namespace lyx {
namespace frontend {

struct FD_browser;

/**
 * This class provides an XForms implementation of a read only
 * text browser.
 */

class FormBrowser : public FormView<FD_browser> {
public:
	///
	FormBrowser(Dialog &, std::string const &, bool allowResize = true);
private:
	/// Build the dialog.
	virtual void build();
};

} // namespace frontend
} // namespace lyx

#endif // FORMBROWSER_H
