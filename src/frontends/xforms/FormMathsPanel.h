// -*- C++ -*-
/**
 * \file FormMathsPanel.h
 * Copyright 2001 The LyX Team.
 * See the file COPYING.
 *
 * \author Alejandro Aguilar Sierra
 * \author John Levon, moz@compsoc.man.ac.uk
 * \author Angus Leeming <leeming@lyx.org>
 */

#ifndef FORM_MATHSPANEL_H
#define FORM_MATHSPANEL_H

#include "commandtags.h"

#ifdef __GNUG__
#pragma interface
#endif

/// values used by the xforms callbacks
enum MathsCallbackValues {
	MM_GREEK,
	MM_ARROW,
	MM_BOP,
	MM_BRELATS,
	MM_VARSIZE,
	MM_MISC,
	MM_FRAC,
	MM_SQRT,
	MM_DELIM,
	MM_SUPER,
	MM_SUB,
	MM_SUBSUPER,
	MM_MATRIX,
	MM_EQU,
	MM_DECO,
	MM_SPACE,
	MM_STYLE,
	MM_DOTS,
	MM_AMS_MISC,
	MM_AMS_ARROWS,
	MM_AMS_BREL,
	MM_AMS_NREL,
	MM_AMS_OPS,
	MM_FUNC
};

#include "FormBaseDeprecated.h"

#include <boost/scoped_ptr.hpp>

class FormMathsBitmap;
class FormMathsDeco;
class FormMathsDelim;
class FormMathsMatrix;
class FormMathsSpace;
class FormMathsStyle;
class FormMathsSub;
struct FD_maths_panel;

/**
 * This class provides an XForms implementation of the maths panel.
 */
class FormMathsPanel : public FormBaseBD {
public:
	///
	FormMathsPanel(LyXView &, Dialogs &);
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

	// Real GUI implementation
	boost::scoped_ptr<FD_maths_panel> dialog_;

	/// send LFUN_MATH_DISPLAY
	void mathDisplay() const;

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
	FormMathsSub(LyXView &, Dialogs &, FormMathsPanel const &,
		     string const &, bool allowResize = true);

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
