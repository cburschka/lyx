/**
 * \file FormMathsPanel.C
 * Copyright 2001 The LyX Team.
 * See the file COPYING.
 *
 * \author Alejandro Aguilar Sierra
 * \author Pablo De Napoli, pdenapo@dm.uba.ar
 * \author John Levon, moz@compsoc.man.ac.uk
 * \author Angus Leeming, a.leeming@ic.ac.uk
 */

#include <config.h>

#ifdef __GNUG_
#pragma implementation
#endif

#include "Dialogs.h"
#include "LyXView.h"
#include "FormMathsPanel.h"
#include "form_maths_panel.h"
#include "MathsSymbols.h"
#include "debug.h"
#include "lyxfunc.h"

#include "form_maths_deco.h"
#include "form_maths_delim.h"
#include "form_maths_matrix.h"
#include "form_maths_space.h"
#include "form_maths_style.h"

#include "FormMathsBitmap.h"
#include "FormMathsDeco.h"
#include "FormMathsDelim.h"
#include "FormMathsMatrix.h"
#include "FormMathsSpace.h"
#include "FormMathsStyle.h"

#include "ButtonController.tmpl"

#include "deco.xpm"
#include "delim.xpm"
#include "equation.xpm"
#include "frac.xpm"
#include "matrix.xpm"
#include "space.xpm"
#include "style.xpm"
#include "sqrt.xpm"
#include "sub.xpm"
#include "super.xpm"

#include "arrows.xbm"
#include "bop.xbm"
#include "brel.xbm"
#include "dots.xbm"
#include "greek.xbm"
#include "misc.xbm"
#include "varsz.xbm"

using std::vector;
using SigC::slot;

FormMathsPanel::FormMathsPanel(LyXView * lv, Dialogs * d)
	: FormBaseBD(lv, d, _("Maths Panel"), false),
	  active_(0), bc_("Close")
{
	deco_.reset(  new FormMathsDeco(  lv, d, *this));
	delim_.reset( new FormMathsDelim( lv, d, *this));
	matrix_.reset(new FormMathsMatrix(lv, d, *this));
	space_.reset( new FormMathsSpace( lv, d, *this));
	style_.reset( new FormMathsStyle( lv, d, *this));

	typedef vector<string> StringVec;
	
	StringVec latex(nr_latex_arrow);
	for (StringVec::size_type i = 0; i < latex.size(); ++i) {
		latex[i] = latex_arrow[i];
	}
	arrow_.reset(new FormMathsBitmap(lv, d, *this, latex));

	latex.resize(nr_latex_bop);
	for (StringVec::size_type i = 0; i < latex.size(); ++i) {
		latex[i] = latex_bop[i];
	}
	boperator_.reset(new FormMathsBitmap(lv, d, *this, latex));

	latex.resize(nr_latex_brel);
	for (StringVec::size_type i = 0; i < latex.size(); ++i) {
		latex[i] = latex_brel[i];
	}
	brelats_.reset(new FormMathsBitmap(lv, d, *this, latex));

	latex.resize(nr_latex_greek);
	for (StringVec::size_type i = 0; i < latex.size(); ++i) {
		latex[i] = latex_greek[i];
	}
	greek_.reset(new FormMathsBitmap(lv, d, *this, latex));

	latex.resize(nr_latex_misc);
	for (StringVec::size_type i = 0; i < latex.size(); ++i) {
		latex[i] = latex_misc[i];
	}
	misc_.reset(new FormMathsBitmap(lv, d, *this, latex));

	latex.resize(nr_latex_dots);
	for (StringVec::size_type i = 0; i<latex.size(); ++i) {
		latex[i] = latex_dots[i];
	}

	dots_.reset(new FormMathsBitmap(lv, d, *this, latex));

	latex.resize(nr_latex_varsz);
	for (StringVec::size_type i = 0; i<latex.size(); ++i) {
		latex[i] = latex_varsz[i];
	}
	varsize_.reset(new FormMathsBitmap(lv, d, *this, latex));

	d->showMathPanel.connect(slot(this, &FormMathsPanel::show));
}


FL_FORM * FormMathsPanel::form() const
{
	if (dialog_.get())
 		return dialog_->form;
	return 0;
}


void FormMathsPanel::setActive(FormMathsSub * a) const
{
	active_ = a;
}


void FormMathsPanel::build()
{
	dialog_.reset(build_maths_panel());
	
	for (int i = 0; i < nr_function_names; ++i)
		fl_add_browser_line(dialog_->browser_funcs,
				    function_names[i]);
	
	fl_set_pixmap_data(dialog_->button_sqrt,
			   const_cast<char**>(sqrt_xpm));
	fl_set_pixmap_data(dialog_->button_frac,
			   const_cast<char**>(frac));
	fl_set_pixmap_data(dialog_->button_super,
			   const_cast<char**>(super_xpm));
	fl_set_pixmap_data(dialog_->button_sub,
			   const_cast<char**>(sub_xpm));
	fl_set_pixmap_data(dialog_->button_delim,
			   const_cast<char**>(delim));
	fl_set_pixmap_data(dialog_->button_deco,
			   const_cast<char**>(deco));
	fl_set_pixmap_data(dialog_->button_space,
			   const_cast<char**>(space_xpm));
	fl_set_pixmap_data(dialog_->button_style,
			   const_cast<char**>(style_xpm));
	fl_set_pixmap_data(dialog_->button_matrix,
			   const_cast<char**>(matrix));
	fl_set_pixmap_data(dialog_->button_equation,
			   const_cast<char**>(equation));

	arrow_->addBitmap(20, 5, 4, arrow_width,  arrow_height,  arrow_bits);
	arrow_->addBitmap(7,  2, 4, larrow_width, larrow_height, larrow_bits,
			  false);
	arrow_->addBitmap(4,  2, 2, darrow_width,  darrow_height, darrow_bits);
	
	boperator_->addBitmap(31, 4, 8, bop_width, bop_height, bop_bits);

	brelats_->addBitmap(35, 4, 9, brel_width, brel_height, brel_bits);

	greek_->addBitmap(11, 6, 2, Greek_width, Greek_height, Greek_bits);
	greek_->addBitmap(28, 7, 4, greek_width, greek_height, greek_bits);

	misc_->addBitmap(29, 5, 6, misc_width, misc_height, misc_bits);
	misc_->addBitmap(6, 3, 2, misc2_width, misc2_height, misc2_bits, false);
	misc_->addBitmap(4, 2, 2, misc3_width, misc3_height, misc3_bits);
 
	dots_->addBitmap(4, 4, 1, dots_width, dots_height, dots_bits);

	varsize_->addBitmap(14, 3, 5, varsz_width, varsz_height, varsz_bits);

	bc().setCancel(dialog_->button_close);
}

bool FormMathsPanel::input(FL_OBJECT *, long data)
{
	MathsCallbackValues val = static_cast<MathsCallbackValues>(data);

	switch (val) {
	case MM_GREEK:
		if (active_ && active_ != greek_.get())
			active_->hide();
		greek_->show();
		break;

	case MM_ARROW:
		if (active_ && active_ != arrow_.get())
			active_->hide();
		arrow_->show();
		break;

	case MM_BOP:
		if (active_ && active_ != boperator_.get())
			active_->hide();
		boperator_->show();
		break;

	case MM_BRELATS:
		if (active_ && active_ != brelats_.get())
			active_->hide();
		brelats_->show();
		break;

	case MM_MISC:
		if (active_ && active_ != misc_.get())
			active_->hide();
		misc_->show();
		break;

	case MM_DOTS:
		if (active_ && active_ != dots_.get())
			active_->hide();
		dots_->show();
		break;
 
	case MM_VARSIZE:
		if (active_ && active_ != varsize_.get())
			active_->hide();
		varsize_->show();
		break;

	case MM_FRAC:
		insertSymbol("frac");
		break;

	case MM_SQRT:
		insertSymbol("sqrt");
		break;

	case MM_SUPER:
		lv_->getLyXFunc()->dispatch(LFUN_MATH_MODE);
		lv_->getLyXFunc()->dispatch(LFUN_SUPERSCRIPT);
		break;

	case MM_SUB:
		lv_->getLyXFunc()->dispatch(LFUN_MATH_MODE);
		lv_->getLyXFunc()->dispatch(LFUN_SUBSCRIPT);
		break;

	case MM_SUBSUPER:
		lv_->getLyXFunc()->dispatch(LFUN_SUBSCRIPT);
		lv_->getLyXFunc()->dispatch(LFUN_LEFT);
		lv_->getLyXFunc()->dispatch(LFUN_SUPERSCRIPT);
		break;

	case MM_DELIM:
		if (active_ && active_ != delim_.get())
			active_->hide();
		delim_->show();
		break;

	case MM_MATRIX:
		if (active_ && active_ != matrix_.get())
			active_->hide();
		matrix_->show();
		break;

	case MM_DECO:
		if (active_ && active_ != deco_.get())
			active_->hide();
		deco_->show();
		break;

	case MM_SPACE:
		if (active_ && active_ != space_.get())
			active_->hide();
		space_->show();
		break;

	case MM_STYLE:
		if (active_ && active_ != style_.get())
			active_->hide();
		style_->show();
		break;

	case MM_EQU:
		mathDisplay();
		break;

	case MM_FUNC:
		int const i = fl_get_browser(dialog_->browser_funcs) - 1;
		insertSymbol(function_names[i]);
		break;
	}

	return true;
}


void FormMathsPanel::insertSymbol(string const & sym, bool bs) const
{
	if (bs) 
		lv_->getLyXFunc()->dispatch(LFUN_INSERT_MATH, '\\' + sym);
	else 
		lv_->getLyXFunc()->dispatch(LFUN_INSERT_MATH, sym);
}


void FormMathsPanel::dispatchFunc(kb_action action) const
{
	lv_->getLyXFunc()->dispatch(action);
}


void FormMathsPanel::mathDisplay() const
{
	lv_->getLyXFunc()->dispatch(LFUN_MATH_DISPLAY);
}



FormMathsSub::FormMathsSub(LyXView * lv, Dialogs * d, FormMathsPanel const & p,
			   string const & t, bool allowResize)
    : FormBaseBD(lv, d, t, allowResize), parent_(p), bc_("Close")
{}


void FormMathsSub::connect()
{
	parent_.setActive(this);
	FormBaseBD::connect();
}


void FormMathsSub::disconnect()
{
	parent_.setActive(0);
	FormBaseBD::disconnect();
}


