/**
 * \file FormMathsPanel.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Alejandro Aguilar Sierra
 * \author Pablo De Napoli
 * \author John Levon
 * \author Angus Leeming
 *
 * Full author contact details are available in file CREDITS
 */

#include <config.h>


#include "ControlMath.h"
#include "FormMathsPanel.h"
#include "forms/form_maths_panel.h"
#include "xformsBC.h"

#include "FormMathsBitmap.h"

#include "forms/form_maths_delim.h"
#include "forms/form_maths_matrix.h"
#include "forms/form_maths_space.h"
#include "forms/form_maths_style.h"

#include "lyx_forms.h"

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
#include "deco.xbm"
#include "dots.xbm"
#include "greek.xbm"
#include "misc.xbm"
#include "varsz.xbm"

#include "ams_misc.xbm"
#include "ams_arrows.xbm"
#include "ams_rel.xbm"
#include "ams_nrel.xbm"
#include "ams_ops.xbm"


typedef FormCB<ControlMath, FormDB<FD_maths_panel> > base_class;

FormMathsPanel::FormMathsPanel()
	: base_class(_("Math Panel"))
{}


FormMathsBitmap * FormMathsPanel::addDaughter(void * key,
					      string const & title,
					      char const * const * data,
					      int size)
{
	char const * const * const end = data + size;
	FormMathsBitmap * const view =
		new FormMathsBitmap(title, std::vector<string>(data, end));

	controller().addDaughter(key, view,
				 new xformsBC(controller().bc()),
				 new IgnorantPolicy);
	return view;
}


void FormMathsPanel::build()
{
	dialog_.reset(build_maths_panel(this));

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

	FormMathsBitmap * bitmap;
	bitmap = addDaughter(dialog_->button_deco,
			     _("Maths Decorations & Accents"),
			     latex_deco, nr_latex_deco);
	bitmap->addBitmap(12, 3, 4, deco1_width, deco1_height, deco1_bits);
	bitmap->addBitmap(10, 4, 3, deco2_width, deco2_height, deco2_bits);

	bitmap = addDaughter(dialog_->button_arrow, _("Arrows"),
			     latex_arrow, nr_latex_arrow);
	bitmap->addBitmap(20, 5, 4, arrow_width,  arrow_height,  arrow_bits);
	bitmap->addBitmap(7,  2, 4, larrow_width, larrow_height, larrow_bits,
			  false);
	bitmap->addBitmap(4,  2, 2, darrow_width,  darrow_height, darrow_bits);

	bitmap = addDaughter(dialog_->button_boperator, _("Binary Ops"),
			     latex_bop, nr_latex_bop);
	bitmap->addBitmap(31, 4, 8, bop_width, bop_height, bop_bits);

	bitmap = addDaughter(dialog_->button_brelats, _("Bin Relations"),
			     latex_brel, nr_latex_brel);
	bitmap->addBitmap(35, 4, 9, brel_width, brel_height, brel_bits);

	bitmap = addDaughter(dialog_->button_greek, _("Greek"),
			     latex_greek, nr_latex_greek);
	bitmap->addBitmap(11, 6, 2, Greek_width, Greek_height, Greek_bits);
	bitmap->addBitmap(28, 7, 4, greek_width, greek_height, greek_bits);

	bitmap = addDaughter(dialog_->button_misc,_("Misc"),
			     latex_misc, nr_latex_misc);
	bitmap->addBitmap(29, 5, 6, misc_width, misc_height, misc_bits);
	bitmap->addBitmap(5, 5, 1, misc4_width, misc4_height, misc4_bits);
	bitmap->addBitmap(6, 3, 2, misc2_width, misc2_height, misc2_bits,
			  false);
	bitmap->addBitmap(4, 2, 2, misc3_width, misc3_height, misc3_bits);

	bitmap = addDaughter(dialog_->button_dots, _("Dots"),
			     latex_dots, nr_latex_dots);
	bitmap->addBitmap(4, 4, 1, dots_width, dots_height, dots_bits);

	bitmap = addDaughter(dialog_->button_varsize, _("Big Operators"),
		     latex_varsz, nr_latex_varsz);
	bitmap->addBitmap(14, 3, 5, varsz_width, varsz_height, varsz_bits);

	bitmap = addDaughter(dialog_->button_ams_misc, _("AMS Misc"),
			     latex_ams_misc, nr_latex_ams_misc);
	bitmap->addBitmap(9, 5, 2, ams1_width, ams1_height, ams1_bits);
	bitmap->addBitmap(26, 3, 9, ams7_width, ams7_height, ams7_bits);

	bitmap = addDaughter(dialog_->button_ams_arrows, _("AMS Arrows"),
			     latex_ams_arrows, nr_latex_ams_arrows);
	bitmap->addBitmap(32, 3, 11, ams2_width, ams2_height, ams2_bits);
	bitmap->addBitmap(6, 3, 2, ams3_width, ams3_height, ams3_bits);

	bitmap = addDaughter(dialog_->button_ams_brel, _("AMS Relations"),
			     latex_ams_rel, nr_latex_ams_rel);
	bitmap->addBitmap(66, 6, 11, ams_rel_width, ams_rel_height,
			  ams_rel_bits);

	bitmap = addDaughter(dialog_->button_ams_nrel, _("AMS Negated Rel"),
			     latex_ams_nrel, nr_latex_ams_nrel);
	bitmap->addBitmap(51, 6, 9, ams_nrel_width, ams_nrel_height,
			  ams_nrel_bits);

	bitmap = addDaughter(dialog_->button_ams_ops, _("AMS Operators"),
			     latex_ams_ops, nr_latex_ams_ops);
	bitmap->addBitmap(23, 3, 8, ams_ops_width, ams_ops_height,
			  ams_ops_bits);
}


ButtonPolicy::SMInput FormMathsPanel::input(FL_OBJECT * ob, long)
{
	if (ob == dialog_->button_arrow ||
	    ob == dialog_->button_boperator ||
	    ob == dialog_->button_brelats ||
	    ob == dialog_->button_greek ||
	    ob == dialog_->button_misc ||
	    ob == dialog_->button_dots ||
	    ob == dialog_->button_varsize ||
	    ob == dialog_->button_ams_misc ||
	    ob == dialog_->button_ams_arrows ||
	    ob == dialog_->button_ams_brel ||
	    ob == dialog_->button_ams_nrel ||
	    ob == dialog_->button_ams_ops ||
	    ob == dialog_->button_deco) {
		controller().showDaughter(ob);
		
	} else if (ob == dialog_->button_delim) {
		controller().showDialog("mathdelimiter");

	} else if (ob == dialog_->button_matrix) {
		controller().showDialog("mathmatrix");

	} else if (ob == dialog_->button_space) {
		controller().showDialog("mathspace");

	} else if (ob == dialog_->button_style) {
		controller().showDialog("mathstyle");

	} else if (ob == dialog_->button_super) {
		controller().dispatchFunc(LFUN_SUPERSCRIPT);

	} else if (ob == dialog_->button_sub) {
		controller().dispatchFunc(LFUN_SUBSCRIPT);

//	} else if (ob == dialog_->???) {
//		controller().dispatchFunc(LFUN_SUBSCRIPT);
//		controller().dispatchFunc(LFUN_LEFT);
//		controller().dispatchFunc(LFUN_SUPERSCRIPT);

	} else if (ob == dialog_->button_equation) {
		controller().dispatchFunc(LFUN_MATH_DISPLAY);

	} else if (ob == dialog_->button_frac) {
		controller().insertSymbol("frac");

	} else if (ob == dialog_->button_sqrt) {
		controller().insertSymbol("sqrt");

	} else if (ob == dialog_->browser_funcs) {
		int const i = fl_get_browser(dialog_->browser_funcs) - 1;
		controller().insertSymbol(function_names[i]);
	}

	return ButtonPolicy::SMI_VALID;
}
