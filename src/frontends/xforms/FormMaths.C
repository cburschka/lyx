/**
 * \file FormMaths.C
 * Copyright 2001 The LyX Team.
 * See the file COPYING.
 *
 * \author Alejandro Aguilar Sierra
 * \author Pablo De Napoli, pdenapo@dm.uba.ar
 * \author John Levon, moz@compsoc.man.ac.uk
 */

#include <config.h>

#ifdef __GNUG_
#pragma implementation
#endif

#include FORMS_H_LOCATION

#include "support/LAssert.h"
#include "FormMaths.h"
#include "form_maths.h"
#include "MathsSymbols.h"
#include "Dialogs.h"
#include "LyXView.h"
#include "lyxfunc.h"
#include "debug.h"
#include "bmtable.h"

#include "mathed/symbol_def.h"
#include "delim.xbm"
#include "delim0.xpm"
#include "delim.xpm"
#include "deco.xbm"
#include "deco.xpm"
#include "space.xpm"
#include "sqrt.xpm"
#include "frac.xpm"
#include "matrix.xpm"
#include "equation.xpm"
#include "greek.xbm"
#include "arrows.xbm"
#include "brel.xbm"
#include "bop.xbm"
#include "misc.xbm"
#include "varsz.xbm"
#include "dots.xbm"

using std::endl;
using SigC::slot;

static int const delim_rversion[] = { 
	1,1,3,3,4,5,7,7,9,9,10,11,
	13,13,14,15,16,17,19,19,20,21,22,23 };
 
static char const * function_names[] = {
	"arccos", "arcsin", "arctan", "arg", "bmod",
	"cos", "cosh", "cot", "coth", "csc", "deg",
	"det", "dim", "exp", "gcd", "hom", "inf", "ker",
	"lg", "lim", "liminf", "limsup", "ln", "log",
	"max", "min", "sec", "sin", "sinh", "sup",
	"tan", "tanh"
};

static int const nr_function_names = sizeof(function_names) / sizeof(char const *);

static char const * decoration_names[] = {
	"widehat", "widetilde", "overbrace", "overleftarrow", "overrightarrow", 
	"overline", "underbrace", "underline"
};

static int const nr_decoration_names = sizeof(decoration_names) / sizeof(char const *);
 
static int delim_values[] = {
	'(', ')', LM_lceil,  LM_rceil,  LM_uparrow,  LM_Uparrow,
	'[', ']', LM_lfloor,  LM_rfloor,  LM_updownarrow, LM_Updownarrow,
	'{', '}',  '/', LM_backslash,  LM_downarrow,  LM_Downarrow,
	LM_langle,  LM_rangle, '|', LM_Vert, '.', 0
};

extern "C" int C_FormMathsWMHideCB(FL_FORM * ob, void *)
{
	fl_hide_form(ob);
	return FL_CANCEL; 
}

 
extern "C" void C_FormMathsButtonCB(FL_OBJECT * ob, long data)
{
	FormMaths * form = static_cast<FormMaths*>(ob->form->u_vdata);
	MathsCallbackValues val = static_cast<MathsCallbackValues>(data);

	lyxerr[Debug::GUI] << "Maths button CB val " << val << endl;

	switch (val) {
		case MM_GREEK:
		case MM_ARROW:
		case MM_BOP:
		case MM_BRELATS:
		case MM_VARSIZE:
		case MM_MISC:
			form->openBitmapMenu(val);
			break;

		case MM_FRAC:
			form->insertSymbol("frac");
			break;
		case MM_SQRT:
			form->insertSymbol("sqrt");
			break;

		case MM_DELIM:
		case MM_MATRIX:
		case MM_DECO:
		case MM_SPACE:
			form->openSubDialog(val);
			break;

		case MM_EQU:
			form->mathDisplay();
			break;

		case MM_FUNC:
			form->insertFunction();
			break;

		case MM_MAX:
		case MM_CLOSE:
		case MM_APPLY:
		case MM_OK:
			Assert(false);
			break;
	}
}


extern "C" void C_FormMathsDelimCB(FL_OBJECT * ob, long data)
{
	FormMaths * form = static_cast<FormMaths*>(ob->form->u_vdata);
	MathsCallbackValues val = static_cast<MathsCallbackValues>(data);

	lyxerr[Debug::GUI] << "Maths delim CB val " << val << endl;

	int left = form->delim_->radio_left->u_ldata;
	int right= form->delim_->radio_right->u_ldata;
	int const side = (fl_get_button(form->delim_->radio_right) != 0);
	std::ostringstream ost;
 
	ost << delim_values[left] << ' ' << delim_values[right];
 
	switch (val) {
		case MM_OK:
	      		form->insertDelim(ost.str().c_str());
		case MM_CLOSE:
			fl_hide_form(form->delim_->form);
			break;

		case MM_APPLY:
			form->insertDelim(ost.str().c_str());
			break;

		/* the bmtable */
		case 2: {
			int const i = fl_get_bmtable(form->delim_->bmtable_delim);
			int const button = fl_get_bmtable_numb(form->delim_->bmtable_delim);
			bool const both = (button == FL_MIDDLE_MOUSE);
	
			if (i>= 0) {
				if (side || (button == FL_RIGHT_MOUSE))
					right = i;
				else {
					left = i;
					if (both)
						right = delim_rversion[i];
				}
			}
 
			Pixmap p1, p2;
 
			p1 = fl_get_pixmap_pixmap(form->delim_->button_delim_pix, &p1, &p2);
			fl_draw_bmtable_item(form->delim_->bmtable_delim, left, p1, 0, 0);
			fl_draw_bmtable_item(form->delim_->bmtable_delim, right, p1, 16, 0);
			fl_redraw_object(form->delim_->button_delim_pix);
	
			form->delim_->radio_left->u_ldata = left;
			form->delim_->radio_right->u_ldata = right;
			} break;
 
		/* left/right - ignore */
		case 3:
		case 4:
			break;
 
		default:
			Assert(false);
			break;
	}
}


extern "C" int align_filter(FL_OBJECT * ob, char const *, char const * cur, int c)
{
	FormMaths * form = static_cast<FormMaths*>(ob->u_vdata);
 
	int n = int(fl_get_slider_value(form->matrix_->slider_matrix_columns)+0.5) - strlen(cur);
	if (n < 0)
		return FL_INVALID;

	if (c == 'c' || c == 'l' || c == 'r') 
		return FL_VALID;
 
	return FL_INVALID;
}
 
 
static char h_align_str[80] = "c";
static char v_align_c[] = "tcb";
 
extern "C" void C_FormMathsMatrixCB(FL_OBJECT * ob, long data)
{
 
	FormMaths * form = static_cast<FormMaths*>(ob->form->u_vdata);
	MathsCallbackValues val = static_cast<MathsCallbackValues>(data);

	lyxerr[Debug::GUI] << "Maths matrix CB val " << val << endl;

	switch (val) {
		case MM_OK:
			form->insertMatrix();
		case MM_CLOSE:
			fl_hide_form(form->matrix_->form);
			break;

		case MM_APPLY:
			form->insertMatrix();
			break;

		/* rows slider etc.: ignore */
		case -1:
			break;
 
		/* the "columns" slider */
		case 2: {
			int const nx = int(fl_get_slider_value(form->matrix_->slider_matrix_columns)+0.5);
			for (int i = 0; i < nx; ++i) 
				h_align_str[i] = 'c';
 
			h_align_str[nx] = '\0';

			fl_set_input(form->matrix_->input_matrix_halign, h_align_str);
			fl_redraw_object(form->matrix_->input_matrix_halign);
			} break;
 
		default:
			Assert(false);
			break;
	}
}


extern "C" void C_FormMathsDecoCB(FL_OBJECT * ob, long data)
{
	FormMaths * form = static_cast<FormMaths*>(ob->form->u_vdata);
	MathsCallbackValues val = static_cast<MathsCallbackValues>(data);

	lyxerr[Debug::GUI] << "Maths deco CB val " << val << endl;

	int const i = fl_get_bmtable(form->deco_->bmtable_deco);
 
	if (i >= nr_decoration_names)
		return;
 
	string const deco_str = decoration_names[i];
 
	switch (val) {
		case MM_OK:
			form->insertSymbol(deco_str);
		case MM_CLOSE:
			fl_hide_form(form->deco_->form);
			break;

		case MM_APPLY:
			form->insertSymbol(deco_str);
			break;

		default:
			Assert(false);
			break;
	}
}


extern "C" void C_FormMathsSpaceCB(FL_OBJECT * ob, long data)
{
	extern char * latex_mathspace[];
	static short sp = -1;
 
	if (data >= 0 && data < 6) {
		sp = data;
		return;
	}
 
	FormMaths * form = static_cast<FormMaths*>(ob->form->u_vdata);
	MathsCallbackValues val = static_cast<MathsCallbackValues>(data);

	lyxerr[Debug::GUI] << "Maths space CB val " << val << endl;

	switch (val) {
		case MM_OK:
			if (sp > 0)
				form->insertSymbol(latex_mathspace[sp]);
		case MM_CLOSE:
			fl_hide_form(form->space_->form);
			break;

		case MM_APPLY:
			if (sp > 0)
				form->insertSymbol(latex_mathspace[sp]);
			break;

		default:
			Assert(false);
			break;
	}
}


FormMaths::FormMaths(LyXView * lv, Dialogs * d)
	: FormBaseBD(lv, d, _("Maths Symbols"))
{
	d->showMathPanel.connect(slot(this, &FormMaths::show));
}


FL_FORM * FormMaths::form() const
{
	if (dialog_.get())
 		return dialog_->form;
	return 0;
}


extern "C" int C_MathsSymbolsPeekCB(FL_FORM *, void *);
 
void FormMaths::build()
{
	dialog_.reset(build_panel());
	delim_.reset(build_delim());
	matrix_.reset(build_matrix());
	deco_.reset(build_deco());
	space_.reset(build_space());

	fl_set_form_atclose(delim_->form, C_FormMathsWMHideCB, delim_.get());
	fl_set_form_atclose(matrix_->form, C_FormMathsWMHideCB, matrix_.get());
	fl_set_form_atclose(deco_->form, C_FormMathsWMHideCB, deco_.get());
	fl_set_form_atclose(space_->form, C_FormMathsWMHideCB, space_.get());
 
	// work around dumb xforms sizing bug
	minw_ = form()->w;
	minh_ = form()->h;

	fl_set_button(space_->radio_space_thin, 1);
	fl_set_button(delim_->radio_left, 1);
	fl_set_pixmap_data(delim_->button_delim_pix, const_cast<char**>(delim0));
	delim_->radio_left->u_ldata = 0;
	delim_->radio_right->u_ldata = 1;
 
	fl_set_bmtable_data(delim_->bmtable_delim, 6, 4, delim_width, delim_height, delim_bits);
	fl_set_bmtable_maxitems(delim_->bmtable_delim, 23);
	
	for (int i = 0; i < nr_function_names; ++i)
		fl_add_browser_line(dialog_->browser_functions, function_names[i]);
	
	fl_set_pixmap_data(dialog_->button_sqrt, const_cast<char**>(sqrt_xpm));
	fl_set_pixmap_data(dialog_->button_frac, const_cast<char**>(frac));
	fl_set_pixmap_data(dialog_->button_delim, const_cast<char**>(delim));
	fl_set_pixmap_data(dialog_->button_deco, const_cast<char**>(deco));
	fl_set_pixmap_data(dialog_->button_space, const_cast<char**>(space_xpm));
	fl_set_pixmap_data(dialog_->button_matrix, const_cast<char**>(matrix));
	fl_set_pixmap_data(dialog_->button_equation, const_cast<char**>(equation));
	
	fl_addto_choice(matrix_->choice_matrix_valign, _("Top | Center | Bottom"));
	fl_set_choice(matrix_->choice_matrix_valign, 2);
	fl_set_input(matrix_->input_matrix_halign, h_align_str);
	matrix_->input_matrix_halign->u_vdata = this;
	fl_set_input_filter(matrix_->input_matrix_halign, align_filter);
	
	fl_set_bmtable_data(deco_->bmtable_deco, 3, 3, deco_width, deco_height, deco_bits);
	fl_set_bmtable_maxitems(deco_->bmtable_deco, 8);

	build_symbol_menus();

	// to close any bitmap menus on interacting with the main form
	fl_register_raw_callback(dialog_->form, KeyPressMask|ButtonPressMask, C_MathsSymbolsPeekCB);
 
	bc_.setCancel(dialog_->button_close);
	bc_.refresh();
}
 

void FormMaths::build_symbol_menus()
{
	FL_OBJECT * obj;

	greek_.reset(new BitmapMenu(this, 2, dialog_->button_greek));
	obj = greek_->AddBitmap(MM_GREEK, 6, 2,
				Greek_width, Greek_height, Greek_bits);
	fl_set_bmtable_maxitems(obj, 11);
	obj = greek_->AddBitmap(MM_GREEK, 7, 4,
				greek_width, greek_height, greek_bits);
	greek_->create();

	boperator_.reset(new BitmapMenu(this, 1, dialog_->button_boperator,
					greek_.get()));
	obj = boperator_->AddBitmap(MM_BOP, 4, 8,
				    bop_width, bop_height, bop_bits);
	fl_set_bmtable_maxitems(obj, 31);
	boperator_->create();

	brelats_.reset(new BitmapMenu(this, 1, dialog_->button_brelats,
				      boperator_.get()));
	obj = brelats_->AddBitmap(MM_BRELATS, 4, 9,
				  brel_width, brel_height, brel_bits);
	fl_set_bmtable_maxitems(obj, 35);
	brelats_->create();

	arrow_.reset(new BitmapMenu(this, 3, dialog_->button_arrow,
				    brelats_.get()));
	obj = arrow_->AddBitmap(MM_ARROW, 5, 4,
				arrow_width, arrow_height, arrow_bits);
	obj = arrow_->AddBitmap(MM_ARROW, 2, 4,
				larrow_width, larrow_height, larrow_bits,
				False);
	fl_set_bmtable_maxitems(obj, 7);
	obj = arrow_->AddBitmap(MM_ARROW, 2, 2,
				darrow_width, darrow_height, darrow_bits);
	arrow_->create();

	varsize_.reset(new BitmapMenu(this, 1, dialog_->button_varsize,
				      arrow_.get()));
	obj = varsize_->AddBitmap(MM_VARSIZE, 3, 5,
				  varsz_width, varsz_height, varsz_bits);
	fl_set_bmtable_maxitems(obj, 14);
	varsize_->create();

	misc_.reset(new BitmapMenu(this, 2, dialog_->button_misc,
				   varsize_.get()));
	obj = misc_->AddBitmap(MM_MISC, 5, 6,
			       misc_width, misc_height, misc_bits);
	fl_set_bmtable_maxitems(obj, 29);
	obj = misc_->AddBitmap(MM_DOTS, 4, 1,
			       dots_width, dots_height, dots_bits);
	misc_->create();
}


void FormMaths::insertMatrix() const
{
	char const c = v_align_c[fl_get_choice(matrix_->choice_matrix_valign) - 1];
	char const * sh = fl_get_input(matrix_->input_matrix_halign);
	int const nx = int(fl_get_slider_value(matrix_->slider_matrix_columns) + 0.5);
	int const ny = int(fl_get_slider_value(matrix_->slider_matrix_rows) + 0.5);
 
	std::ostringstream ost;
	ost << nx << ' ' << ny << ' ' << c << sh;
 
	lv_->getLyXFunc()->Dispatch(LFUN_INSERT_MATRIX, ost.str().c_str());
}

 
void FormMaths::insertDelim(string const & delim) const
{
	lv_->getLyXFunc()->Dispatch(LFUN_MATH_DELIM, delim);
}

 
void FormMaths::openBitmapMenu(MathsCallbackValues val) const
{
	switch (val) {
		case MM_GREEK:
			greek_->show();
			break;
		case MM_ARROW:
			arrow_->show();
			break;
		case MM_BOP:
			boperator_->show();
			break;
		case MM_BRELATS:
			brelats_->show();
			break;
		case MM_VARSIZE:
			varsize_->show();
			break;
		case MM_MISC:
			misc_->show();
			break;
		default:
			Assert(false);
	}
}


void FormMaths::insertSymbol(string const & sym) const
{
	lv_->getLyXFunc()->Dispatch(LFUN_INSERT_MATH, sym);
}


void FormMaths::mathDisplay() const
{
	lv_->getLyXFunc()->Dispatch(LFUN_MATH_DISPLAY);
}


void FormMaths::openSubDialog(MathsCallbackValues val) const
{
	FL_FORM * form = 0;
	string title;

	switch (val) {
		case MM_DELIM:
			form = delim_->form;
			title = "Delimiters";
			break;
		case MM_MATRIX:
			form = matrix_->form;
			title = "Matrix";
			break;
		case MM_DECO:
			form = deco_->form; 
			title = "Maths Decorations";
			break;
		case MM_SPACE:
			form = space_->form;
			title = "Maths Spacing";
			break;
		default:
			Assert(false);
	}

	if (form->visible) {
		fl_raise_form(form);
		XMapWindow(fl_get_display(), form->window);
	} else
		fl_show_form(form, FL_PLACE_MOUSE | FL_FREE_SIZE, 0, title.c_str());
}


void FormMaths::insertFunction() const
{
	int const i = fl_get_browser(dialog_->browser_functions) - 1;
	insertSymbol(function_names[i]);
}
