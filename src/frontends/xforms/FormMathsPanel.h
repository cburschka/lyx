// -*- C++ -*-
/**
 * \file FormMathsPanel.h
 * Copyright 2001 The LyX Team.
 * See the file COPYING.
 *
 * \author Alejandro Aguilar Sierra
 * \author John Levon, moz@compsoc.man.ac.uk
 * \author Angus Leeming, a.leeming@ic.ac.uk
 */

#ifndef FORM_MATHSPANEL_H
#define FORM_MATHSPANEL_H

#include "commandtags.h"
#include <boost/smart_ptr.hpp>

#ifdef __GNUG_
#pragma interface
#endif

#include "FormBaseDeprecated.h"
#include "MathsSymbols.h"

class FormMathsBitmap;
class FormMathsDeco;
class FormMathsDelim;
class FormMathsMatrix;
class FormMathsSpace;
class FormMathsStyle;
class FormMathsSub;
struct FD_form_maths_panel;

/**
 * This class provides an XForms implementation of the maths panel.
 */
class FormMathsPanel : public FormBaseBD {
public:
	///
	FormMathsPanel(LyXView *, Dialogs *);
	///
	void setActive(FormMathsSub *) const;
	/// dispatch a symbol insert
	void insertSymbol(string const & sym, bool bs = true) const;
	/// dispatch an LFUN:
	void dispatchFunc(kb_action action) const;
private:
	/// Pointer to the actual instantiation of the ButtonController.
	virtual xformsBC & bc();

	/// Build the dialog
	virtual void build();
	/// input handler
	virtual bool input(FL_OBJECT *, long);

	/// Pointer to the actual instantiation of the xforms form
	virtual FL_FORM * form() const;

	// build the panels
	FD_form_maths_panel * build_maths_panel();

	/// send LFUN_MATH_DISPLAY
	void mathDisplay() const;

	// Real GUI implementation
	boost::scoped_ptr<FD_form_maths_panel> dialog_;

	/// Subdialogs
	boost::scoped_ptr<FormMathsDeco>   deco_;
	boost::scoped_ptr<FormMathsDelim>  delim_;
	boost::scoped_ptr<FormMathsMatrix> matrix_;
	boost::scoped_ptr<FormMathsSpace>  space_;
	boost::scoped_ptr<FormMathsStyle>  style_;
	boost::scoped_ptr<FormMathsBitmap> arrow_;
	boost::scoped_ptr<FormMathsBitmap> boperator_;
	boost::scoped_ptr<FormMathsBitmap> brelats_;
	boost::scoped_ptr<FormMathsBitmap> greek_;
	boost::scoped_ptr<FormMathsBitmap> misc_;
	boost::scoped_ptr<FormMathsBitmap> dots_;
	boost::scoped_ptr<FormMathsBitmap> varsize_;
	boost::scoped_ptr<FormMathsBitmap> ams_misc_;
	boost::scoped_ptr<FormMathsBitmap> ams_arrows_;
	boost::scoped_ptr<FormMathsBitmap> ams_rel_;
	boost::scoped_ptr<FormMathsBitmap> ams_nrel_;
	boost::scoped_ptr<FormMathsBitmap> ams_ops_;

	/// A pointer to the currently active subdialog
	mutable FormMathsSub * active_;

	/// The ButtonController
	ButtonController<OkCancelReadOnlyPolicy, xformsBC> bc_;
};


class FormMathsSub : public FormBaseBD {
	friend class FormMathsPanel; // has subdialogs to manipulate

public:
	///
	FormMathsSub(LyXView *, Dialogs *, FormMathsPanel const &,
		     string const &, bool allowResize=true);

protected:
	/// Pointer to the actual instantiation of the ButtonController.
	virtual xformsBC & bc();
	/// The parent Maths Panel
	FormMathsPanel const & parent_;
private:
	///
	virtual void connect();
	///
	virtual void disconnect();
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
