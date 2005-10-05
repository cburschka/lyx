/**
 * \file FormTabular.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Jürgen Vigna
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "FormTabular.h"
#include "forms/form_tabular.h"

#include "xforms_helpers.h"
#include "xformsBC.h"

#include "controllers/ButtonController.h"
#include "controllers/ControlTabular.h"
#include "controllers/helper_funcs.h"

#include "support/convert.h"
#include "support/lstrings.h"

#include "lyx_forms.h"

#include <boost/bind.hpp>

using boost::bind;

using std::string;
using std::vector;

namespace lyx {

using support::contains;
using support::getStringFromVector;
using support::isStrDbl;

namespace frontend {

namespace {

#if FL_VERSION == 0 || (FL_REVISION == 0 && FL_FIXLEVEL < 2)
bool const scalableTabfolders = false;
#else
bool const scalableTabfolders = true;
#endif

} // namespace anon


typedef FormController<ControlTabular, FormView<FD_tabular> > base_class;

FormTabular::FormTabular(Dialog & parent)
	: base_class(parent, _("Table Settings"), scalableTabfolders),
	closing_(false), actCell_(LyXTabular::npos)
{
}


void FormTabular::redraw()
{
	if (form() && form()->visible)
		fl_redraw_form(form());
	else
		return;

	FL_FORM * outer_form = fl_get_active_folder(dialog_->tabfolder);
	if (outer_form && outer_form->visible)
		fl_redraw_form(outer_form);
}


void FormTabular::build()
{
	dialog_.reset(build_tabular(this));

	// Allow the base class to control messages
	setMessageWidget(dialog_->text_warning);

	// trigger an input event for cut&paste with middle mouse button.
	setPrehandler(dialog_->input_tabular_column);
	setPrehandler(dialog_->input_tabular_row);

	// Create the contents of the unit choices; don't include the "%" terms.
	vector<string> units_vec = getLatexUnits();
	vector<string>::iterator ret =
		remove_if(units_vec.begin(), units_vec.end(),
			  bind(contains<char>, _1, '%'));
	units_vec.erase(ret, units_vec.end());
	string const units = getStringFromVector(units_vec, "|");

	// tabular options form
	tabular_options_.reset(build_tabular_options(this));

	// column options form
	column_options_.reset(build_tabular_column(this));

	fl_set_input_return(column_options_->input_column_width,
			    FL_RETURN_END);
	fl_set_input_return(column_options_->input_special_alignment,
			    FL_RETURN_END);

	// trigger an input event for cut&paste with middle mouse button.
	setPrehandler(column_options_->input_column_width);
	setPrehandler(column_options_->input_special_alignment);

	fl_addto_choice(column_options_->choice_value_column_width,
			units.c_str());

	// cell options form
	cell_options_.reset(build_tabular_cell(this));

	fl_set_input_return(cell_options_->input_mcolumn_width,
			    FL_RETURN_END);
	fl_set_input_return(cell_options_->input_special_multialign,
			    FL_RETURN_END);

	// trigger an input event for cut&paste with middle mouse button.
	setPrehandler(cell_options_->input_mcolumn_width);
	setPrehandler(cell_options_->input_special_multialign);

	fl_addto_choice(cell_options_->choice_value_mcolumn_width,
			units.c_str());

	longtable_options_.reset(build_tabular_longtable(this));

	// Enable the tabfolder to be rescaled correctly.
	if (scalableTabfolders)
		fl_set_tabfolder_autofit(dialog_->tabfolder, FL_FIT);

	// Stack tabs
	fl_addto_tabfolder(dialog_->tabfolder, _("Table").c_str(),
			   tabular_options_->form);
	fl_addto_tabfolder(dialog_->tabfolder, _("Column/Row").c_str(),
			   column_options_->form);
	fl_addto_tabfolder(dialog_->tabfolder, _("Cell").c_str(),
			   cell_options_->form);
	fl_addto_tabfolder(dialog_->tabfolder, _("LongTable").c_str(),
			   longtable_options_->form);

	//  FIXME: addReadOnly everything
}


void FormTabular::update()
{
	if (closing_)
		return;

	LyXTabular const & tabular = controller().tabular();

	int align;
	LyXLength pwidth;
	string special;

	LyXTabular::idx_type const cell = controller().getActiveCell();
	actCell_ = cell;
	LyXTabular::col_type column = tabular.column_of_cell(cell) + 1;
	clearMessage();
	fl_activate_object(column_options_->input_special_alignment);
	fl_activate_object(cell_options_->input_special_multialign);
	fl_activate_object(column_options_->input_column_width);
	fl_activate_object(column_options_->choice_value_column_width);
	fl_set_input(dialog_->input_tabular_column,
	             convert<string>(column).c_str());
	fl_deactivate_object(dialog_->input_tabular_column);
	LyXTabular::row_type row = tabular.row_of_cell(cell);
	fl_set_input(dialog_->input_tabular_row,
	             convert<string>(row + 1).c_str());
	fl_deactivate_object(dialog_->input_tabular_row);
	if (tabular.isMultiColumn(cell)) {
		fl_set_button(cell_options_->check_multicolumn, 1);
		fl_set_button(cell_options_->check_border_top,
			      tabular.topLine(cell)?1:0);
		setEnabled(cell_options_->check_border_top, true);
		fl_set_button(cell_options_->check_border_bottom,
			      tabular.bottomLine(cell)?1:0);
		setEnabled(cell_options_->check_border_bottom, true);
		// pay attention to left/right lines they are only allowed
		// to set if we are in first/last cell of row or if the left/right
		// cell is also a multicolumn.
		if (tabular.isFirstCellInRow(cell) ||
		    tabular.isMultiColumn(cell-1)) {
			fl_set_button(cell_options_->check_border_left,
				      tabular.leftLine(cell)?1:0);
			setEnabled(cell_options_->check_border_left, true);
		} else {
			fl_set_button(cell_options_->check_border_left, 0);
			setEnabled(cell_options_->check_border_left, false);
		}
		if (tabular.isLastCellInRow(cell) ||
		    tabular.isMultiColumn(cell+1)) {
			fl_set_button(cell_options_->check_border_right,
				      tabular.rightLine(cell)?1:0);
			setEnabled(cell_options_->check_border_right, true);
		} else {
			fl_set_button(cell_options_->check_border_right, 0);
			setEnabled(cell_options_->check_border_right, false);
		}
		pwidth = tabular.getMColumnPWidth(cell);
		align = tabular.getAlignment(cell);
		// set the horiz. alignment, default is left here
		fl_set_button(cell_options_->radio_align_left, 0);
		fl_set_button(cell_options_->radio_align_right, 0);
		fl_set_button(cell_options_->radio_align_center, 0);
		if (align == LYX_ALIGN_RIGHT)
			fl_set_button(cell_options_->radio_align_right, 1);
		else if (align == LYX_ALIGN_CENTER)
			fl_set_button(cell_options_->radio_align_center, 1);
		else
			fl_set_button(cell_options_->radio_align_left, 1);

		align = tabular.getVAlignment(cell);
		fl_set_button(cell_options_->radio_valign_top, 0);
		fl_set_button(cell_options_->radio_valign_bottom, 0);
		fl_set_button(cell_options_->radio_valign_middle, 0);
		if (pwidth.zero() || (align == LyXTabular::LYX_VALIGN_MIDDLE))
			fl_set_button(cell_options_->radio_valign_middle, 1);
		else if (align == LyXTabular::LYX_VALIGN_BOTTOM)
			fl_set_button(cell_options_->radio_valign_bottom, 1);
		else
			fl_set_button(cell_options_->radio_valign_top, 1);

		special = tabular.getAlignSpecial(cell, LyXTabular::SET_SPECIAL_MULTI);
		fl_set_input(cell_options_->input_special_multialign, special.c_str());
		bool const metric(controller().useMetricUnits());
		string const default_unit = metric ? "cm" : "in";
		updateWidgetsFromLength(cell_options_->input_mcolumn_width,
					cell_options_->choice_value_mcolumn_width,
					pwidth, default_unit);

		if (!bc().bp().isReadOnly()) {
			setEnabled(cell_options_->input_special_multialign, true);
			setEnabled(cell_options_->input_mcolumn_width, true);
			setEnabled(cell_options_->choice_value_mcolumn_width, true);
		}

		setEnabled(cell_options_->radio_valign_top,    !pwidth.zero());
		setEnabled(cell_options_->radio_valign_bottom, !pwidth.zero());
		setEnabled(cell_options_->radio_valign_middle, !pwidth.zero());

		setEnabled(cell_options_->radio_align_left,   true);
		setEnabled(cell_options_->radio_align_right,  true);
		setEnabled(cell_options_->radio_align_center, true);
	} else {
		fl_set_button(cell_options_->check_multicolumn, 0);

		fl_set_button(cell_options_->check_border_top, 0);
		setEnabled(cell_options_->check_border_top, false);

		fl_set_button(cell_options_->check_border_bottom, 0);
		setEnabled(cell_options_->check_border_bottom, false);

		fl_set_button(cell_options_->check_border_left, 0);
		setEnabled(cell_options_->check_border_left, false);

		fl_set_button(cell_options_->check_border_right, 0);
		setEnabled(cell_options_->check_border_right, false);

		fl_set_button(cell_options_->radio_align_left, 0);
		setEnabled(cell_options_->radio_align_left, false);

		fl_set_button(cell_options_->radio_align_right, 0);
		setEnabled(cell_options_->radio_align_right, false);

		fl_set_button(cell_options_->radio_align_center, 0);
		setEnabled(cell_options_->radio_align_center, false);

		fl_set_button(cell_options_->radio_valign_top, 0);
		setEnabled(cell_options_->radio_valign_top, false);

		fl_set_button(cell_options_->radio_valign_bottom, 0);
		setEnabled(cell_options_->radio_valign_bottom, false);

		fl_set_button(cell_options_->radio_valign_middle, 0);
		setEnabled(cell_options_->radio_valign_middle, false);

		fl_set_input(cell_options_->input_special_multialign, "");
		setEnabled(cell_options_->input_special_multialign, false);

		fl_set_input(cell_options_->input_mcolumn_width, "");
		setEnabled(cell_options_->input_mcolumn_width, false);
		setEnabled(cell_options_->choice_value_mcolumn_width, false);
	}
	if (tabular.getRotateCell(cell))
		fl_set_button(cell_options_->check_rotate_cell, 1);
	else
		fl_set_button(cell_options_->check_rotate_cell, 0);
	if (tabular.topLine(cell, true))
		fl_set_button(column_options_->check_border_top, 1);
	else
		fl_set_button(column_options_->check_border_top, 0);
	if (tabular.bottomLine(cell, true))
		fl_set_button(column_options_->check_border_bottom, 1);
	else
		fl_set_button(column_options_->check_border_bottom, 0);
	if (tabular.leftLine(cell, true))
		fl_set_button(column_options_->check_border_left, 1);
	else
		fl_set_button(column_options_->check_border_left, 0);
	if (tabular.rightLine(cell, true))
		fl_set_button(column_options_->check_border_right, 1);
	else
		fl_set_button(column_options_->check_border_right, 0);
	special = tabular.getAlignSpecial(cell, LyXTabular::SET_SPECIAL_COLUMN);
	fl_set_input(column_options_->input_special_alignment, special.c_str());

	bool const isReadonly = bc().bp().isReadOnly();
	setEnabled(column_options_->input_special_alignment, !isReadonly);

	pwidth = tabular.getColumnPWidth(cell);
	bool const metric = controller().useMetricUnits();
	string const default_unit = metric ? "cm" : "in";
	updateWidgetsFromLength(column_options_->input_column_width,
				column_options_->choice_value_column_width,
				pwidth, default_unit);
	setEnabled(column_options_->input_column_width, !isReadonly);
	setEnabled(column_options_->choice_value_column_width, !isReadonly);

	setEnabled(cell_options_->check_useminipage, !pwidth.zero());
	if (!pwidth.zero()) {
		if (tabular.getUsebox(cell) == 2)
			fl_set_button(cell_options_->check_useminipage, 1);
		else
			fl_set_button(cell_options_->check_useminipage, 0);
	} else {
		fl_set_button(cell_options_->check_useminipage, 0);
	}
	align = tabular.getAlignment(cell, true);
	fl_set_button(column_options_->radio_align_left, 0);
	fl_set_button(column_options_->radio_align_right, 0);
	fl_set_button(column_options_->radio_align_center, 0);
	fl_set_button(column_options_->radio_align_block, 0);
	if (align == LYX_ALIGN_LEFT)
		fl_set_button(column_options_->radio_align_left, 1);
	else if (align == LYX_ALIGN_RIGHT)
		fl_set_button(column_options_->radio_align_right, 1);
	else if (align == LYX_ALIGN_CENTER)
		fl_set_button(column_options_->radio_align_center, 1);
	else
		fl_set_button(column_options_->radio_align_block, 1);
	align = tabular.getVAlignment(cell, true);
	fl_set_button(column_options_->radio_valign_top, 0);
	fl_set_button(column_options_->radio_valign_bottom, 0);
	fl_set_button(column_options_->radio_valign_middle, 0);
	if (pwidth.zero() || (align == LyXTabular::LYX_VALIGN_MIDDLE))
		fl_set_button(column_options_->radio_valign_middle, 1);
	else if (align == LyXTabular::LYX_VALIGN_BOTTOM)
		fl_set_button(column_options_->radio_valign_bottom, 1);
	else
		fl_set_button(column_options_->radio_valign_top, 1);

	setEnabled(column_options_->radio_align_left,   true);
	setEnabled(column_options_->radio_align_right,  true);
	setEnabled(column_options_->radio_align_center, true);
	setEnabled(column_options_->radio_align_block,   !pwidth.zero());
	setEnabled(column_options_->radio_valign_top,    !pwidth.zero());
	setEnabled(column_options_->radio_valign_bottom, !pwidth.zero());
	setEnabled(column_options_->radio_valign_middle, !pwidth.zero());

	fl_set_button(tabular_options_->check_longtable,
		      tabular.isLongTabular());

	bool const enable = tabular.isLongTabular();

	setEnabled(longtable_options_->check_lt_firsthead, enable);
	setEnabled(longtable_options_->check_1head_2border_above, enable);
	setEnabled(longtable_options_->check_1head_2border_below, enable);
	setEnabled(longtable_options_->check_1head_empty, enable);
	setEnabled(longtable_options_->check_lt_head, enable);
	setEnabled(longtable_options_->check_head_2border_above, enable);
	setEnabled(longtable_options_->check_head_2border_below, enable);
	setEnabled(longtable_options_->check_lt_foot, enable);
	setEnabled(longtable_options_->check_foot_2border_above, enable);
	setEnabled(longtable_options_->check_foot_2border_below, enable);
	setEnabled(longtable_options_->check_lt_lastfoot, enable);
	setEnabled(longtable_options_->check_lastfoot_2border_above, enable);
	setEnabled(longtable_options_->check_lastfoot_2border_below, enable);
	setEnabled(longtable_options_->check_lastfoot_empty, enable);
	setEnabled(longtable_options_->check_lt_newpage, enable);

	if (enable) {
		LyXTabular::ltType ltt;
		bool use_empty;
		bool row_set = tabular.getRowOfLTHead(row, ltt);
		fl_set_button(longtable_options_->check_lt_head, row_set);
		if (ltt.set) {
			fl_set_button(longtable_options_->check_head_2border_above,
				      ltt.topDL);
			fl_set_button(longtable_options_->check_head_2border_below,
				      ltt.bottomDL);
			use_empty = true;
		} else {
			setEnabled(longtable_options_->check_head_2border_above, 0);
			setEnabled(longtable_options_->check_head_2border_below, 0);
			fl_set_button(longtable_options_->check_head_2border_above,0);
			fl_set_button(longtable_options_->check_head_2border_below,0);
			fl_set_button(longtable_options_->check_1head_empty,0);
			setEnabled(longtable_options_->check_1head_empty, 0);
			use_empty = false;
		}
		//
		row_set = tabular.getRowOfLTFirstHead(row, ltt);
		fl_set_button(longtable_options_->check_lt_firsthead, row_set);
		if (ltt.set && (!ltt.empty || !use_empty)) {
			fl_set_button(longtable_options_->check_1head_2border_above,
				      ltt.topDL);
			fl_set_button(longtable_options_->check_1head_2border_below,
				      ltt.bottomDL);
		} else {
			setEnabled(longtable_options_->check_1head_2border_above, 0);
			setEnabled(longtable_options_->check_1head_2border_below, 0);
			fl_set_button(longtable_options_->check_1head_2border_above,0);
			fl_set_button(longtable_options_->check_1head_2border_below,0);
			if (use_empty) {
				fl_set_button(longtable_options_->check_1head_empty,ltt.empty);
				if (ltt.empty)
					setEnabled(longtable_options_->check_lt_firsthead, 0);
			}
		}
		//
		row_set = tabular.getRowOfLTFoot(row, ltt);
		fl_set_button(longtable_options_->check_lt_foot, row_set);
		if (ltt.set) {
			fl_set_button(longtable_options_->check_foot_2border_above,
				      ltt.topDL);
			fl_set_button(longtable_options_->check_foot_2border_below,
				      ltt.bottomDL);
			use_empty = true;
		} else {
			setEnabled(longtable_options_->check_foot_2border_above, 0);
			setEnabled(longtable_options_->check_foot_2border_below, 0);
			fl_set_button(longtable_options_->check_foot_2border_above,0);
			fl_set_button(longtable_options_->check_foot_2border_below,0);
			fl_set_button(longtable_options_->check_lastfoot_empty, 0);
			setEnabled(longtable_options_->check_lastfoot_empty, 0);
			use_empty = false;
		}
		//
		row_set = tabular.getRowOfLTLastFoot(row, ltt);
		fl_set_button(longtable_options_->check_lt_lastfoot, row_set);
		if (ltt.set && (!ltt.empty || !use_empty)) {
			fl_set_button(longtable_options_->check_lastfoot_2border_above,
				      ltt.topDL);
			fl_set_button(longtable_options_->check_lastfoot_2border_below,
				      ltt.bottomDL);
		} else {
			setEnabled(longtable_options_->check_lastfoot_2border_above,0);
			setEnabled(longtable_options_->check_lastfoot_2border_below,0);
			fl_set_button(longtable_options_->check_lastfoot_2border_above, 0);
			fl_set_button(longtable_options_->check_lastfoot_2border_below, 0);
			if (use_empty) {
				fl_set_button(longtable_options_->check_lastfoot_empty,
					      ltt.empty);
				if (ltt.empty)
					setEnabled(longtable_options_->check_lt_lastfoot, 0);
			}
		}
		fl_set_button(longtable_options_->check_lt_newpage,
			      tabular.getLTNewPage(row));
	} else {
		fl_set_button(longtable_options_->check_lt_firsthead, 0);
		fl_set_button(longtable_options_->check_1head_2border_above, 0);
		fl_set_button(longtable_options_->check_1head_2border_below, 0);
		fl_set_button(longtable_options_->check_1head_empty, 0);
		fl_set_button(longtable_options_->check_lt_head, 0);
		fl_set_button(longtable_options_->check_head_2border_above, 0);
		fl_set_button(longtable_options_->check_head_2border_below, 0);
		fl_set_button(longtable_options_->check_lt_foot, 0);
		fl_set_button(longtable_options_->check_foot_2border_above, 0);
		fl_set_button(longtable_options_->check_foot_2border_below, 0);
		fl_set_button(longtable_options_->check_lt_lastfoot, 0);
		fl_set_button(longtable_options_->check_lastfoot_2border_above, 0);
		fl_set_button(longtable_options_->check_lastfoot_2border_below, 0);
		fl_set_button(longtable_options_->check_lastfoot_empty, 0);
		fl_set_button(longtable_options_->check_lt_newpage, 0);
	}
	fl_set_button(tabular_options_->check_rotate_tabular,
		      tabular.getRotateTabular());
}


ButtonPolicy::SMInput FormTabular::input(FL_OBJECT * ob, long)
{
	int s;
	LyXTabular::Feature num = LyXTabular::LAST_ACTION;
	string special;

	LyXTabular const & tabular = controller().tabular();

	LyXTabular::idx_type const cell = controller().getActiveCell();

	// ugly hack to auto-apply the stuff that hasn't been
	// yet. don't let this continue to exist ...
	if (ob == dialog_->button_close) {
		closing_ = true;
		string w1 =
			getLengthFromWidgets(column_options_->input_column_width,
					     column_options_->choice_value_column_width);
		string w2;
		LyXLength llen = tabular.getColumnPWidth(cell);
		if (!llen.zero())
			w2 = llen.asString();

		string mw1 = getLengthFromWidgets(cell_options_->input_mcolumn_width,
					    cell_options_->choice_value_mcolumn_width);
		llen = tabular.getMColumnPWidth(cell);
		string mw2;
		if (llen.zero())
			mw2 = "";
		else
			mw2 = llen.asString();

		string al1 = getString(column_options_->input_special_alignment);
		string al2 = tabular.getAlignSpecial(cell, LyXTabular::SET_SPECIAL_COLUMN);
		string mal1 = getString(cell_options_->input_special_multialign);
		string mal2 = tabular.getAlignSpecial(cell, LyXTabular::SET_SPECIAL_MULTI);

		// we must do these all at the end
		if (w1 != w2)
			input(column_options_->input_column_width, 0);
		if (mw1 != mw2)
			input(cell_options_->input_mcolumn_width, 0);
		if (al1 != al2)
			input(column_options_->input_special_alignment, 0);
		if (mal1 != mal2)
			input(cell_options_->input_special_multialign, 0);
		closing_ = false;
		dialog().OKButton();
		return ButtonPolicy::SMI_VALID;
	}

	if (actCell_ != cell) {
		update();
		postWarning(_("Wrong Cursor position, updated window"));
		return ButtonPolicy::SMI_VALID;
	}

	// No point in processing directives that you can't do anything with
	// anyhow, so exit now if the buffer is read-only.
	if (bc().bp().isReadOnly()) {
		update();
		return ButtonPolicy::SMI_VALID;
	}

	if ((ob == column_options_->input_column_width) ||
	    (ob == column_options_->choice_value_column_width)) {
		string const str =
			getLengthFromWidgets(column_options_->input_column_width,
					     column_options_->choice_value_column_width);
		controller().set(LyXTabular::SET_PWIDTH, str);

		//check if the input is valid
		string const input = getString(column_options_->input_column_width);
		if (!input.empty() && !isValidLength(input) && !isStrDbl(input)) {
			postWarning(_("Invalid Length (valid example: 10mm)"));
			return ButtonPolicy::SMI_INVALID;
		}

		update(); // update for alignment
		return ButtonPolicy::SMI_VALID;
	}

	if ((ob == cell_options_->input_mcolumn_width) ||
	    (ob == cell_options_->choice_value_mcolumn_width)) {
		string const str =
			getLengthFromWidgets(cell_options_->input_mcolumn_width,
					     cell_options_->choice_value_mcolumn_width);
		controller().set(LyXTabular::SET_MPWIDTH, str);

		//check if the input is valid
		string const input = getString(cell_options_->input_mcolumn_width);
		if (!input.empty() && !isValidLength(input) && !isStrDbl(input)) {
			postWarning(_("Invalid Length (valid example: 10mm)"));
			return ButtonPolicy::SMI_INVALID;
		}
		update(); // update for alignment
		return ButtonPolicy::SMI_VALID;
	}

	if (ob == tabular_options_->button_append_row)
		num = LyXTabular::APPEND_ROW;
	else if (ob == tabular_options_->button_append_column)
		num = LyXTabular::APPEND_COLUMN;
	else if (ob == tabular_options_->button_delete_row)
		num = LyXTabular::DELETE_ROW;
	else if (ob == tabular_options_->button_delete_column)
		num = LyXTabular::DELETE_COLUMN;
	else if (ob == tabular_options_->button_set_borders)
		num = LyXTabular::SET_ALL_LINES;
	else if (ob == tabular_options_->button_unset_borders)
		num = LyXTabular::UNSET_ALL_LINES;
	else if (ob == column_options_->check_border_top)
		num = LyXTabular::TOGGLE_LINE_TOP;
	else if (ob == column_options_->check_border_bottom)
		num = LyXTabular::TOGGLE_LINE_BOTTOM;
	else if (ob == column_options_->check_border_left)
		num = LyXTabular::TOGGLE_LINE_LEFT;
	else if (ob == column_options_->check_border_right)
		num = LyXTabular::TOGGLE_LINE_RIGHT;
	else if (ob == column_options_->radio_align_left)
		num = LyXTabular::ALIGN_LEFT;
	else if (ob == column_options_->radio_align_right)
		num = LyXTabular::ALIGN_RIGHT;
	else if (ob == column_options_->radio_align_center)
		num = LyXTabular::ALIGN_CENTER;
	else if (ob == column_options_->radio_align_block)
		num = LyXTabular::ALIGN_BLOCK;
	else if (ob == column_options_->radio_valign_top)
		num = LyXTabular::VALIGN_TOP;
	else if (ob == column_options_->radio_valign_bottom)
		num = LyXTabular::VALIGN_BOTTOM;
	else if (ob == column_options_->radio_valign_middle)
		num = LyXTabular::VALIGN_MIDDLE;
	else if (ob == cell_options_->check_multicolumn)
		num = LyXTabular::MULTICOLUMN;
	else if (ob == tabular_options_->check_longtable) {
		if (fl_get_button(tabular_options_->check_longtable))
			num = LyXTabular::SET_LONGTABULAR;
		else
			num = LyXTabular::UNSET_LONGTABULAR;
	} else if (ob == tabular_options_->check_rotate_tabular) {
		s = fl_get_button(tabular_options_->check_rotate_tabular);
		if (s)
			num = LyXTabular::SET_ROTATE_TABULAR;
		else
			num = LyXTabular::UNSET_ROTATE_TABULAR;
	} else if (ob == cell_options_->check_rotate_cell) {
		s = fl_get_button(cell_options_->check_rotate_cell);
		if (s)
			num = LyXTabular::SET_ROTATE_CELL;
		else
			num = LyXTabular::UNSET_ROTATE_CELL;
	} else if (ob == cell_options_->check_useminipage) {
		num = LyXTabular::SET_USEBOX;
		special = "2";
	} else if ((ob == longtable_options_->check_lt_firsthead) ||
		   (ob == longtable_options_->check_1head_2border_above) ||
		   (ob == longtable_options_->check_1head_2border_below) ||
		   (ob == longtable_options_->check_1head_empty) ||
		   (ob == longtable_options_->check_lt_head) ||
		   (ob == longtable_options_->check_head_2border_above) ||
		   (ob == longtable_options_->check_head_2border_below) ||
		   (ob == longtable_options_->check_lt_foot) ||
		   (ob == longtable_options_->check_foot_2border_above) ||
		   (ob == longtable_options_->check_foot_2border_below) ||
		   (ob == longtable_options_->check_lt_lastfoot) ||
		   (ob == longtable_options_->check_lastfoot_2border_above) ||
		   (ob == longtable_options_->check_lastfoot_2border_below) ||
		   (ob == longtable_options_->check_lastfoot_empty)) {
		num = static_cast<LyXTabular::Feature>(checkLongtableOptions(ob, special));
	} else if (ob == longtable_options_->check_lt_newpage) {
		num = LyXTabular::SET_LTNEWPAGE;
	} else if (ob == column_options_->input_special_alignment) {
		special = getString(column_options_->input_special_alignment);
		num = LyXTabular::SET_SPECIAL_COLUMN;
	} else if (ob == cell_options_->input_special_multialign) {
		special = getString(cell_options_->input_special_multialign);
		num = LyXTabular::SET_SPECIAL_MULTI;
	} else if (ob == cell_options_->check_border_top)
		num = LyXTabular::M_TOGGLE_LINE_TOP;
	else if (ob == cell_options_->check_border_bottom)
		num = LyXTabular::M_TOGGLE_LINE_BOTTOM;
	else if (ob == cell_options_->check_border_left)
		num = LyXTabular::M_TOGGLE_LINE_LEFT;
	else if (ob == cell_options_->check_border_right)
		num = LyXTabular::M_TOGGLE_LINE_RIGHT;
	else if (ob == cell_options_->radio_align_left)
		num = LyXTabular::M_ALIGN_LEFT;
	else if (ob == cell_options_->radio_align_right)
		num = LyXTabular::M_ALIGN_RIGHT;
	else if (ob == cell_options_->radio_align_center)
		num = LyXTabular::M_ALIGN_CENTER;
	else if (ob == cell_options_->radio_valign_top)
		num = LyXTabular::M_VALIGN_TOP;
	else if (ob == cell_options_->radio_valign_bottom)
		num = LyXTabular::M_VALIGN_BOTTOM;
	else if (ob == cell_options_->radio_valign_middle)
		num = LyXTabular::M_VALIGN_MIDDLE;
	else
		return ButtonPolicy::SMI_VALID;

	controller().set(num, special);
	update();

	return ButtonPolicy::SMI_VALID;
}


int FormTabular::checkLongtableOptions(FL_OBJECT * ob, string & special)
{
	bool flag = fl_get_button(ob);
	if ((ob == longtable_options_->check_1head_2border_above) ||
	    (ob == longtable_options_->check_head_2border_above) ||
	    (ob == longtable_options_->check_foot_2border_above) ||
	    (ob == longtable_options_->check_lastfoot_2border_above)) {
		special = "dl_above";
	} else if ((ob == longtable_options_->check_1head_2border_below) ||
		   (ob == longtable_options_->check_head_2border_below) ||
		   (ob == longtable_options_->check_foot_2border_below) ||
		   (ob == longtable_options_->check_lastfoot_2border_below)) {
		special = "dl_below";
	} else if ((ob == longtable_options_->check_1head_empty) ||
		   (ob == longtable_options_->check_lastfoot_empty)) {
		special = "empty";
	} else {
		special = "";
	}
	if ((ob == longtable_options_->check_lt_firsthead) ||
	    (ob == longtable_options_->check_1head_2border_above) ||
	    (ob == longtable_options_->check_1head_2border_below) ||
	    (ob == longtable_options_->check_1head_empty)) {
		return (flag ? LyXTabular::SET_LTFIRSTHEAD :
			LyXTabular::UNSET_LTFIRSTHEAD);
	} else if ((ob == longtable_options_->check_lt_head) ||
			   (ob == longtable_options_->check_head_2border_above) ||
			   (ob == longtable_options_->check_head_2border_below)) {
		return (flag ? LyXTabular::SET_LTHEAD : LyXTabular::UNSET_LTHEAD);
	} else if ((ob == longtable_options_->check_lt_foot) ||
		   (ob == longtable_options_->check_foot_2border_above) ||
		   (ob == longtable_options_->check_foot_2border_below)) {
		return (flag ? LyXTabular::SET_LTFOOT : LyXTabular::UNSET_LTFOOT);
	} else if ((ob == longtable_options_->check_lt_lastfoot) ||
		   (ob == longtable_options_->check_lastfoot_2border_above) ||
		   (ob == longtable_options_->check_lastfoot_2border_below) ||
		   (ob == longtable_options_->check_lastfoot_empty)) {
		return (flag ? LyXTabular::SET_LTLASTFOOT :
			LyXTabular::UNSET_LTLASTFOOT);
	}

	return LyXTabular::LAST_ACTION;
}

} // namespace frontend
} // namespace lyx
