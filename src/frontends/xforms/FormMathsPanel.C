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
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "FormMathsPanel.h"
#include "ControlMath.h"
#include "forms/form_maths_panel.h"

#include "xformsBC.h"

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

namespace lyx {
namespace frontend {

typedef FormController<ControlMath, FormView<FD_maths_panel> > base_class;

FormMathsPanel::FormMathsPanel(Dialog & parent)
	: base_class(parent, _("Math Panel"))
{}


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
}


ButtonPolicy::SMInput FormMathsPanel::input(FL_OBJECT * ob, long)
{
	if (ob == dialog_->button_arrow) {
		controller().showDialog("matharrows");
	} else if (ob == dialog_->button_deco) {
		controller().showDialog("mathaccents");
	} else if (ob == dialog_->button_boperator) {
		controller().showDialog("mathoperators");
	} else if (ob == dialog_->button_brelats) {
		controller().showDialog("mathrelations");
	} else if (ob == dialog_->button_greek) {
		controller().showDialog("mathgreek");
	} else if (ob == dialog_->button_misc) {
		controller().showDialog("mathmisc");
	} else if (ob == dialog_->button_dots) {
		controller().showDialog("mathdots");
	} else if (ob == dialog_->button_varsize) {
		controller().showDialog("mathbigoperators");
	} else if (ob == dialog_->button_ams_misc) {
		controller().showDialog("mathamsmisc");
	} else if (ob == dialog_->button_ams_arrows) {
		controller().showDialog("mathamsarrows");
	} else if (ob == dialog_->button_ams_brel) {
		controller().showDialog("mathamsrelations");
	} else if (ob == dialog_->button_ams_nrel) {
		controller().showDialog("mathamsnegatedrelations");
	} else if (ob == dialog_->button_ams_ops) {
		controller().showDialog("mathamsoperators");
	} else if (ob == dialog_->button_delim) {
		controller().showDialog("mathdelimiter");
	} else if (ob == dialog_->button_matrix) {
		controller().showDialog("mathmatrix");
	} else if (ob == dialog_->button_space) {
		controller().showDialog("mathspace");
	} else if (ob == dialog_->button_style) {
		controller().showDialog("mathstyle");

	} else if (ob == dialog_->button_super) {
		controller().dispatchSuperscript();

	} else if (ob == dialog_->button_sub) {
		controller().dispatchSubscript();

	} else if (ob == dialog_->button_equation) {
		controller().dispatchToggleDisplay();

	} else if (ob == dialog_->button_frac) {
		controller().dispatchInsert("frac");

	} else if (ob == dialog_->button_sqrt) {
		controller().dispatchInsert("sqrt");

	} else if (ob == dialog_->browser_funcs) {
		int const i = fl_get_browser(dialog_->browser_funcs) - 1;
		controller().dispatchInsert(function_names[i]);
	}

	return ButtonPolicy::SMI_VALID;
}

} // namespace frontend
} // namespace lyx
