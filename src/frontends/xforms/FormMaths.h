/** 
 * \file FormMaths.h
 * Copyright 2001 The LyX Team.
 * See the file COPYING.
 * 
 * \author Alejandro Aguilar Sierra 
 * \author John Levon
 */

#ifndef FORM_MATHS_H
#define FORM_MATHS_H

#include <boost/smart_ptr.hpp>

#ifdef __GNUG_
#pragma interface
#endif

#include "FormBaseDeprecated.h"

class BitmapMenu; 
struct FD_form_panel;
struct FD_form_delim;
struct FD_form_matrix;
struct FD_form_deco;
struct FD_form_space;

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
	MM_MATRIX,
	MM_EQU,
	MM_DECO,
	MM_SPACE,
	MM_DOTS,
	MM_FUNC,
	MM_MAX,
	MM_CLOSE = 1024,
	MM_APPLY,
	MM_OK
};
 
/** 
 * This class provides an XForms implementation of the maths panel.
 */
class FormMaths : public FormBaseBD {
public:
	///
	FormMaths(LyXView *, Dialogs *);
	///
	/// input callback functions
	void InputCB(FL_OBJECT *, long);
	///
	void ButtonCB(FL_OBJECT *, long);
	///
	void DelimCB(FL_OBJECT *, long);
	///
	void MatrixCB(FL_OBJECT *, long);
	///
	void DecoCB(FL_OBJECT *, long);
	///
	void SpaceCB(FL_OBJECT *, long);
	///
	int AlignFilter(char const *, int);


	/// insert a matrix 
	void insertMatrix() const;
 
	/// insert a delimiter
	void insertDelim(string const & delim) const;
 
	/// open a bitmap menu 
	void openBitmapMenu(MathsCallbackValues val) const;
 
	/// dispatch a symbol insert 
	void insertSymbol(string const & sym) const;
 
	/// send LFUN_MATH_DISPLAY
	void mathDisplay() const;

	/// open a sub-dialog 
	void openSubDialog(MathsCallbackValues val) const;

	/// dispatch a function name insert
	void insertFunction() const;

private:
	/// Pointer to the actual instantiation of the ButtonController.
	virtual xformsBC & bc();
	
	/// Build the popup
	virtual void build();
   
	/// build the bitmap menus 
	void build_symbol_menus();
 
	/// Pointer to the actual instantiation of the xforms form
	virtual FL_FORM * form() const;

	// build the panels
	FD_form_panel * build_panel();
	FD_form_delim * build_delim();
	FD_form_matrix * build_matrix();
	FD_form_deco * build_deco();
	FD_form_space * build_space();
	
	// Real GUI implementations
	boost::scoped_ptr<FD_form_panel> dialog_;
	boost::scoped_ptr<FD_form_delim> delim_;
	boost::scoped_ptr<FD_form_matrix> matrix_;
	boost::scoped_ptr<FD_form_deco> deco_;
	boost::scoped_ptr<FD_form_space> space_;

	// bit map menus
	boost::scoped_ptr<BitmapMenu> greek_;
	boost::scoped_ptr<BitmapMenu> boperator_;
	boost::scoped_ptr<BitmapMenu> brelats_;
	boost::scoped_ptr<BitmapMenu> arrow_;
	boost::scoped_ptr<BitmapMenu> varsize_;
	boost::scoped_ptr<BitmapMenu> misc_;

	/// The ButtonController
	ButtonController<OkCancelReadOnlyPolicy, xformsBC> bc_;
};


inline
xformsBC & FormMaths::bc()
{
	return bc_;
}

#endif /* FORM_MATHS_H */
