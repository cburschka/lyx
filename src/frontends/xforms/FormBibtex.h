// -*- C++ -*-
/**
 * \file FormBibtex.h
 * Copyright 2001 the LyX Team
 * Read the file COPYING
 *
 * \author Angus Leeming
 * \author John Levon
 */

#ifndef FORMBIBTEX_H
#define FORMBIBTEX_H

#include <boost/smart_ptr.hpp>

#ifdef __GNUG__
#pragma interface
#endif

#include "FormInset.h"

struct FD_form_bibtex;

/**
 * For bibtex database setting
 */
class FormBibtex : public FormCommand {
public:
	///
	FormBibtex(LyXView *, Dialogs *);
private:
	/// Pointer to the actual instantiation of the ButtonController.
	virtual xformsBC & bc();
	/// Connect signals etc. Set form's max size.
	virtual void connect();
	/// Build the dialog
	virtual void build();
	/// Update dialog before showing it
	virtual void update();
	/// input handler
	virtual bool input(FL_OBJECT *, long);
	/// Apply from dialog (modify or create inset)
	virtual void apply();
	/// Pointer to the actual instantiation of the xforms form
	virtual FL_FORM * form() const;
	///
	FD_form_bibtex * build_bibtex();
	/// Real GUI implementation.
	boost::scoped_ptr<FD_form_bibtex> dialog_;
	/// The ButtonController
	ButtonController<OkCancelReadOnlyPolicy, xformsBC> bc_;
};


inline
xformsBC & FormBibtex::bc()
{
  return bc_;
}
#endif // FORMBIBTEX_H
