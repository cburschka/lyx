// -*- C++ -*-
/**
 * \file FormMathsPanel.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Alejandro Aguilar Sierra
 * \author John Levon
 * \author Angus Leeming 
 *
 * Full author contact details are available in file CREDITS
 */

#ifndef FORM_MATHSPANEL_H
#define FORM_MATHSPANEL_H

#include "commandtags.h"

#ifdef __GNUG__
#pragma interface
#endif

#include "FormBaseDeprecated.h"

#include <boost/shared_ptr.hpp>
#include <map>

class FormMathsBitmap;
class FormMathsSub;
struct FD_maths_panel;

/**
 * This class provides an XForms implementation of the maths panel.
 */
class FormMathsPanel : public FormBaseBD {
public:
	///
	FormMathsPanel(LyXView &, Dialogs &);
	/// dispatch an LFUN:
	void dispatchFunc(kb_action action,
			  string const & arg = string()) const;
	/// dispatch a symbol insert
	void insertSymbol(string const & sym, bool bs = true) const;

private:
	/// Pointer to the actual instantiation of the ButtonController.
	virtual xformsBC & bc();

	/// Build the dialog
	virtual void build();
	/// input handler
	virtual bool input(FL_OBJECT *, long);

	/// Pointer to the actual instantiation of the xforms form
	virtual FL_FORM * form() const;

	// Real GUI implementation
	boost::scoped_ptr<FD_maths_panel> dialog_;

	/** Add a bitmap dialog to the store of all daughters_ and
	 *  return a pointer to the dialog, so that bitmaps can be added to it.
	 */
	FormMathsBitmap * addDaughter(FL_OBJECT * button, string const & title,
				      char const * const * data, int size);

	///
	void showDaughter(FL_OBJECT *);

	///
	typedef boost::shared_ptr<FormMathsSub> DaughterDialog;
	typedef std::map<FL_OBJECT *, DaughterDialog> Store;

	/** The store of all daughter dialogs.
	 *  The map uses the button on the main panel to identify them.
	 */
	Store daughters_;

	/// A pointer to the currently active daughter dialog.
	FormMathsSub * active_;

	/// The ButtonController.
	ButtonController<OkCancelReadOnlyPolicy, xformsBC> bc_;
};


class FormMathsSub : public FormBaseBD {
public:
	///
	FormMathsSub(LyXView &, Dialogs &, FormMathsPanel const &,
		     string const &, bool allowResize = true);

	///
	bool isVisible() const;

protected:
	/// Pointer to the actual instantiation of the ButtonController.
	virtual xformsBC & bc();
	/// The parent Maths Panel
	FormMathsPanel const & parent_;
private:
	/// The ButtonController
	ButtonController<IgnorantPolicy, xformsBC> bc_;
};


inline
xformsBC & FormMathsSub::bc()
{
	return bc_;
}

inline
xformsBC & FormMathsPanel::bc()
{
	return bc_;
}
#endif //  FORM_MATHSPANEL_H
