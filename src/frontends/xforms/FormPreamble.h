// -*- C++ -*-
/**
 * \file FormPreamble.h
 * Copyright 2001 The LyX Team.
 * See the file COPYING.
 *
 * \author Edwin Leuven, leuven@fee.uva.nl
 */

#ifndef FORMPREAMBLE_H
#define FORMPREAMBLE_H

#include <boost/smart_ptr.hpp>

#ifdef __GNUG__
#pragma interface
#endif

#include "FormBaseDeprecated.h"

struct FD_form_preamble;

/** This class provides an XForms implementation of the FormPreamble Dialog.
 */
class FormPreamble : public FormBaseBD {
public:
	///
	FormPreamble(LyXView *, Dialogs *);
private:
	/// Pointer to the actual instantiation of the ButtonController.
	virtual xformsBC & bc();

	/// Filter the inputs
	// virtual bool input(FL_OBJECT *, long);
   
	/// Build the popup
	virtual void build();
	/// Apply from popup
	virtual void apply();
	/// Update the popup.
	virtual void update();
	///
	virtual FL_FORM * form() const;
   
	/// Typedefinitions from the fdesign produced Header file
	FD_form_preamble  * build_preamble();
   
	/// Real GUI implementation.
	boost::scoped_ptr<FD_form_preamble> dialog_;
	/// The ButtonController
	ButtonController<NoRepeatedApplyReadOnlyPolicy, xformsBC> bc_;
};


inline
xformsBC & FormPreamble::bc()
{
  return bc_;
}
#endif
