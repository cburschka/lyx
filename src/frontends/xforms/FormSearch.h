/**
 * \file FormSearch.h
 * Copyright 2001 The LyX Team.
 * See the file COPYING.
 *
 * \author Edwin Leuven
 */

#ifndef FORMSEARCH_H
#define FORMSEARCH_H

#include <boost/smart_ptr.hpp>

#ifdef __GNUG__
#pragma interface
#endif

#include "FormBaseDeprecated.h"

struct FD_form_search;

/** This class provides an XForms implementation of the FormSearch Dialog.
 */
class FormSearch : public FormBaseBD {
public:
	///
	FormSearch(LyXView *, Dialogs *);
   
private:
	/// Pointer to the actual instantiation of the ButtonController.
	virtual xformsBC & bc();

	/// Filter the inputs
	virtual bool input(FL_OBJECT *, long);
   
	/// Build the popup
	virtual void build();

	/// Update the popup
	virtual void update();

	/// Searches occurance of string
	/// if argument=true forward search otherwise backward search
	void Find(bool const = true);
	/// if argument=false replace once otherwise replace all
	/// Replaces occurance of string
	void Replace(bool const = false);
   
	///
	virtual FL_FORM * form() const;
   
	/// Typedefinitions from the fdesign produced Header file
	FD_form_search  * build_search();
   
	/// Real GUI implementation.
	boost::scoped_ptr<FD_form_search> dialog_;
	/// The ButtonController
	ButtonController<NoRepeatedApplyReadOnlyPolicy, xformsBC> bc_;
};


inline
xformsBC & FormSearch::bc()
{
	return bc_;
}
#endif
