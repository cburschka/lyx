/**
 * \file FormTabular.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Jürgen Vigna
 *
 * Full author contact details are available in file CREDITS
 */

#include <config.h>

#ifdef __GNUG__
#pragma implementation
#endif

#include "debug.h"

#include "FormTabular.h"
#include "forms/form_tabular.h"
#include "frontends/LyXView.h"
#include "buffer.h"
#include "xforms_helpers.h"
#include "lyxrc.h" // to set the default length values
#include "helper_funcs.h"
#include "input_validators.h"

#include "insets/insettabular.h"

#include "support/lstrings.h"

#include <boost/bind.hpp>

#include <functional>
#include FORMS_H_LOCATION


using std::vector;
using std::bind2nd;
using std::remove_if;


FormTabular::FormTabular(LyXView & lv, Dialogs & d)
	: FormInset(lv, d, _("Tabular Layout")),
	  inset_(0), actCell_(-1), closing_(false)
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


FL_FORM * FormTabular::form() const
{
	if (dialog_.get())
		return dialog_->form;
	return 0;
}


void FormTabular::disconnect()
{
	inset_ = 0;
	FormInset::disconnect();
}


void FormTabular::showInset(InsetTabular * inset)
{
	if (inset == 0) return;

	// If connected to another inset, disconnect from it.
	if (inset_ != inset) {
		ih_.disconnect();
		ih_ = inset->hideDialog.connect(boost::bind(&FormTabular::hide, this));
		inset_ = inset;
	}

	show();
}


void FormTabular::updateInset(InsetTabular * inset)
{
	if (inset == 0 || inset_ == 0) return;

	// If connected to another inset, disconnect from it.
	if (inset_ != inset) {
		ih_.disconnect();
		ih_ = inset->hideDialog.connect(boost::bind(&FormTabular::hide, this));
		inset_ = inset;
	}

	update();
}


void FormTabular::build()
{
	dialog_.reset(build_tabular(this));

	// Allow the base class to control messages
	setMessageWidget(dialog_->text_warning);

	setPrehandler(dialog_->input_tabular_column);
	setPrehandler(dialog_->input_tabular_row);

	tabular_options_.reset(build_tabular_options(this));
	column_options_.reset(build_tabular_column(this));
	cell_options_.reset(build_tabular_cell(this));
	longtable_options_.reset(build_tabular_longtable(this));

	fl_set_input_return(column_options_->input_column_width,
			    FL_RETURN_END);
	fl_set_input_return(column_options_->input_special_alignment,
			    FL_RETURN_END);

	setPrehandler(column_options_->input_column_width);
	setPrehandler(column_options_->input_special_alignment);

	fl_set_input_return(cell_options_->input_mcolumn_width,
			    FL_RETURN_END);
	fl_set_input_return(cell_options_->input_special_multialign,
			    FL_RETURN_END);

	setPrehandler(cell_options_->input_mcolumn_width);
	setPrehandler(cell_options_->input_special_multialign);

	fl_addto_tabfolder(dialog_->tabfolder, _("Tabular"),
			   tabular_options_->form);
	fl_addto_tabfolder(dialog_->tabfolder, _("Column/Row"),
			   column_options_->form);
	fl_addto_tabfolder(dialog_->tabfolder, _("Cell"),
			   cell_options_->form);
	fl_addto_tabfolder(dialog_->tabfolder, _("LongTable"),
			   longtable_options_->form);

	// Create the contents of the unit choices
	// Don't include the "%" terms...
	vector<string> units_vec = getLatexUnits();
	vector<string>::iterator ret =
		remove_if(units_vec.begin(), units_vec.end(),
			  bind2nd(contains_functor(), "%"));
	units_vec.erase(ret, units_vec.end());

	string units = getStringFromVector(units_vec, "|");

	fl_addto_choice(column_options_->choice_value_column_width,
			units.c_str());
	fl_addto_choice(cell_options_->choice_value_mcolumn_width,
			units.c_str());
}


void FormTabular::update()
{
	if (closing_)
		return;

	if (!inset_ || !inset_->tabular.get())
		return;

	LyXTabular * tabular = inset_->tabular.get();
	int align;
	char buf[12];
	LyXLength pwidth;
	string special;

	int cell = inset_->getActCell();
	actCell_ = cell;
	int column = tabular->column_of_cell(cell) + 1;
	clearMessage();
	fl_activate_object(column_options_->input_special_alignment);
	fl_activate_object(cell_options_->input_special_multialign);
	fl_activate_object(column_options_->input_column_width);
	fl_activate_object(column_options_->choice_value_column_width);
	sprintf(buf, "%d", column);
	fl_set_input(dialog_->input_tabular_column, buf);
	fl_deactivate_object(dialog_->input_tabular_column);
	int row = tabular->row_of_cell(cell);
	sprintf(buf, "%d", row + 1);
	fl_set_input(dialog_->input_tabular_row, buf);
	fl_deactivate_object(dialog_->input_tabular_row);
	if (tabular->IsMultiColumn(cell)) {
		fl_set_button(cell_options_->check_multicolumn, 1);
		fl_set_button(cell_options_->check_border_top,
			      tabular->TopLine(cell)?1:0);
		setEnabled(cell_options_->check_border_top, true);
		fl_set_button(cell_options_->check_border_bottom,
			      tabular->BottomLine(cell)?1:0);
		setEnabled(cell_options_->check_border_bottom, true);
		// pay attention to left/right lines they are only allowed
		// to set if we are in first/last cell of row or if the left/right
		// cell is also a multicolumn.
		if (tabular->IsFirstCellInRow(cell) ||
		    tabular->IsMultiColumn(cell-1)) {
			fl_set_button(cell_options_->check_border_left,
				      tabular->LeftLine(cell)?1:0);
			setEnabled(cell_options_->check_border_left, true);
		} else {
			fl_set_button(cell_options_->check_border_left, 0);
			setEnabled(cell_options_->check_border_left, false);
		}
		if (tabular->IsLastCellInRow(cell) ||
		    tabular->IsMultiColumn(cell+1)) {
			fl_set_button(cell_options_->check_border_right,
				      tabular->RightLine(cell)?1:0);
			setEnabled(cell_options_->check_border_right, true);
		} else {
			fl_set_button(cell_options_->check_border_right, 0);
			setEnabled(cell_options_->check_border_right, false);
		}
		pwidth = tabular->GetMColumnPWidth(cell);
		align = tabular->GetAlignment(cell);
		if (!pwidth.zero() || (align == LYX_ALIGN_LEFT))
			fl_set_button(cell_options_->radio_align_left, 1);
		else if (align == LYX_ALIGN_RIGHT)
			fl_set_button(cell_options_->radio_align_right, 1);
		else
			fl_set_button(cell_options_->radio_align_center, 1);

		align = tabular->GetVAlignment(cell);
		fl_set_button(cell_options_->radio_valign_top, 0);
		fl_set_button(cell_options_->radio_valign_bottom, 0);
		fl_set_button(cell_options_->radio_valign_center, 0);
		if (pwidth.zero() || (align == LyXTabular::LYX_VALIGN_CENTER))
			fl_set_button(cell_options_->radio_valign_center, 1);
		else if (align == LyXTabular::LYX_VALIGN_BOTTOM)
			fl_set_button(cell_options_->radio_valign_bottom, 1);
		else
			fl_set_button(cell_options_->radio_valign_top, 1);

		special = tabular->GetAlignSpecial(cell, LyXTabular::SET_SPECIAL_MULTI);
		fl_set_input(cell_options_->input_special_multialign, special.c_str());
		bool const metric = lyxrc.default_papersize > 3;
		string const default_unit = metric ? "cm" : "in";
		updateWidgetsFromLength(cell_options_->input_mcolumn_width,
					cell_options_->choice_value_mcolumn_width,
					pwidth, default_unit);

		if (!lv_.buffer()->isReadonly()) {
			setEnabled(cell_options_->input_special_multialign, true);
			setEnabled(cell_options_->input_mcolumn_width, true);
			setEnabled(cell_options_->choice_value_mcolumn_width, true);
		}

		setEnabled(cell_options_->radio_valign_top,    !pwidth.zero());
		setEnabled(cell_options_->radio_valign_bottom, !pwidth.zero());
		setEnabled(cell_options_->radio_valign_center, !pwidth.zero());

		setEnabled(cell_options_->radio_align_left,   pwidth.zero());
		setEnabled(cell_options_->radio_align_right,  pwidth.zero());
		setEnabled(cell_options_->radio_align_center, pwidth.zero());
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

		fl_set_button(cell_options_->radio_valign_center, 0);
		setEnabled(cell_options_->radio_valign_center, false);

		fl_set_input(cell_options_->input_special_multialign, "");
		setEnabled(cell_options_->input_special_multialign, false);

		fl_set_input(cell_options_->input_mcolumn_width, "");
		setEnabled(cell_options_->input_mcolumn_width, false);
		setEnabled(cell_options_->choice_value_mcolumn_width, false);
	}
	if (tabular->GetRotateCell(cell))
		fl_set_button(cell_options_->check_rotate_cell, 1);
	else
		fl_set_button(cell_options_->check_rotate_cell, 0);
	if (tabular->TopLine(cell, true))
		fl_set_button(column_options_->check_border_top, 1);
	else
		fl_set_button(column_options_->check_border_top, 0);
	if (tabular->BottomLine(cell, true))
		fl_set_button(column_options_->check_border_bottom, 1);
	else
		fl_set_button(column_options_->check_border_bottom, 0);
	if (tabular->LeftLine(cell, true))
		fl_set_button(column_options_->check_border_left, 1);
	else
		fl_set_button(column_options_->check_border_left, 0);
	if (tabular->RightLine(cell, true))
		fl_set_button(column_options_->check_border_right, 1);
	else
		fl_set_button(column_options_->check_border_right, 0);
	special = tabular->GetAlignSpecial(cell, LyXTabular::SET_SPECIAL_COLUMN);
	fl_set_input(column_options_->input_special_alignment, special.c_str());

	bool const isReadonly = lv_.buffer()->isReadonly();
	setEnabled(column_options_->input_special_alignment, !isReadonly);

	pwidth = tabular->GetColumnPWidth(cell);
	bool const metric = lyxrc.default_papersize > 3;
	string const default_unit = metric ? "cm" : "in";
	updateWidgetsFromLength(column_options_->input_column_width,
				column_options_->choice_value_column_width,
				pwidth, default_unit);
	setEnabled(column_options_->input_column_width, !isReadonly);
	setEnabled(column_options_->choice_value_column_width, !isReadonly);

	setEnabled(cell_options_->check_useminipage, !pwidth.zero());
	if (!pwidth.zero()) {
		if (tabular->GetUsebox(cell) == 2)
			fl_set_button(cell_options_->check_useminipage, 1);
		else
			fl_set_button(cell_options_->check_useminipage, 0);
	} else {
		fl_set_button(cell_options_->check_useminipage, 0);
	}
	align = tabular->GetAlignment(cell, true);
	fl_set_button(column_options_->radio_align_left, 0);
	fl_set_button(column_options_->radio_align_right, 0);
	fl_set_button(column_options_->radio_align_center, 0);
	if (!pwidth.zero() || (align == LYX_ALIGN_LEFT))
		fl_set_button(column_options_->radio_align_left, 1);
	else if (align == LYX_ALIGN_RIGHT)
		fl_set_button(column_options_->radio_align_right, 1);
	else
		fl_set_button(column_options_->radio_align_center, 1);
	align = tabular->GetVAlignment(cell, true);
	fl_set_button(column_options_->radio_valign_top, 0);
	fl_set_button(column_options_->radio_valign_bottom, 0);
	fl_set_button(column_options_->radio_valign_center, 0);
	if (pwidth.zero() || (align == LyXTabular::LYX_VALIGN_CENTER))
		fl_set_button(column_options_->radio_valign_center, 1);
	else if (align == LyXTabular::LYX_VALIGN_BOTTOM)
		fl_set_button(column_options_->radio_valign_bottom, 1);
	else
		fl_set_button(column_options_->radio_valign_top, 1);

	setEnabled(column_options_->radio_align_left,   pwidth.zero());
	setEnabled(column_options_->radio_align_right,  pwidth.zero());
	setEnabled(column_options_->radio_align_center, pwidth.zero());

	setEnabled(column_options_->radio_valign_top,    !pwidth.zero());
	setEnabled(column_options_->radio_valign_bottom, !pwidth.zero());
	setEnabled(column_options_->radio_valign_center, !pwidth.zero());

	fl_set_button(tabular_options_->check_longtable,
		      tabular->IsLongTabular());

	bool const enable = tabular->IsLongTabular();

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
		bool row_set = tabular->GetRowOfLTHead(row, ltt);
		fl_set_button(longtable_options_->check_lt_head, row_set);
		if (ltt.set) {
			fl_set_button(longtable_options_->check_head_2border_above,
				      ltt.topDL);
			fl_set_button(longtable_options_->check_head_2border_above,
				      ltt.topDL);
			use_empty = true;
		} else {
			setEnabled(longtable_options_->check_head_2border_above, 0);
			setEnabled(longtable_options_->check_head_2border_below, 0);
			fl_set_button(longtable_options_->check_head_2border_above,0);
			fl_set_button(longtable_options_->check_head_2border_above,0);
			fl_set_button(longtable_options_->check_1head_empty,0);
			setEnabled(longtable_options_->check_1head_empty, 0);
			use_empty = false;
		}
		//
		row_set = tabular->GetRowOfLTFirstHead(row, ltt);
		fl_set_button(longtable_options_->check_lt_firsthead, row_set);
		if (ltt.set && (!ltt.empty || !use_empty)) {
			fl_set_button(longtable_options_->check_1head_2border_above,
				      ltt.topDL);
			fl_set_button(longtable_options_->check_1head_2border_above,
				      ltt.topDL);
		} else {
			setEnabled(longtable_options_->check_1head_2border_above, 0);
			setEnabled(longtable_options_->check_1head_2border_below, 0);
			fl_set_button(longtable_options_->check_1head_2border_above,0);
			fl_set_button(longtable_options_->check_1head_2border_above,0);
			if (use_empty) {
				fl_set_button(longtable_options_->check_1head_empty,ltt.empty);
				if (ltt.empty)
					setEnabled(longtable_options_->check_lt_firsthead, 0);
			}
		}
		//
		row_set = tabular->GetRowOfLTFoot(row, ltt);
		fl_set_button(longtable_options_->check_lt_foot, row_set);
		if (ltt.set) {
			fl_set_button(longtable_options_->check_foot_2border_above,
				      ltt.topDL);
			fl_set_button(longtable_options_->check_foot_2border_above,
				      ltt.topDL);
			use_empty = true;
		} else {
			setEnabled(longtable_options_->check_foot_2border_above, 0);
			setEnabled(longtable_options_->check_foot_2border_below, 0);
			fl_set_button(longtable_options_->check_foot_2border_above,0);
			fl_set_button(longtable_options_->check_foot_2border_above,0);
			fl_set_button(longtable_options_->check_lastfoot_empty, 0);
			setEnabled(longtable_options_->check_lastfoot_empty, 0);
			use_empty = false;
		}
		//
		row_set = tabular->GetRowOfLTLastFoot(row, ltt);
		fl_set_button(longtable_options_->check_lt_lastfoot, row_set);
		if (ltt.set && (!ltt.empty || !use_empty)) {
			fl_set_button(longtable_options_->check_lastfoot_2border_above,
				      ltt.topDL);
			fl_set_button(longtable_options_->check_lastfoot_2border_above,
				      ltt.topDL);
		} else {
			setEnabled(longtable_options_->check_lastfoot_2border_above,0);
			setEnabled(longtable_options_->check_lastfoot_2border_below,0);
			fl_set_button(longtable_options_->check_lastfoot_2border_above, 0);
			fl_set_button(longtable_options_->check_lastfoot_2border_above, 0);
			if (use_empty) {
				fl_set_button(longtable_options_->check_lastfoot_empty,
					      ltt.empty);
				if (ltt.empty)
					setEnabled(longtable_options_->check_lt_lastfoot, 0);
			}
		}
		fl_set_button(longtable_options_->check_lt_newpage,
			      tabular->GetLTNewPage(row));
	} else {
		fl_set_button(longtable_options_->check_lt_firsthead, 0);
		fl_set_button(longtable_options_->check_1head_2border_above, 0);
		fl_set_button(longtable_options_->check_1head_2border_above, 0);
		fl_set_button(longtable_options_->check_1head_empty, 0);
		fl_set_button(longtable_options_->check_lt_head, 0);
		fl_set_button(longtable_options_->check_head_2border_above, 0);
		fl_set_button(longtable_options_->check_head_2border_above, 0);
		fl_set_button(longtable_options_->check_lt_foot, 0);
		fl_set_button(longtable_options_->check_foot_2border_above, 0);
		fl_set_button(longtable_options_->check_foot_2border_above, 0);
		fl_set_button(longtable_options_->check_lt_lastfoot, 0);
		fl_set_button(longtable_options_->check_lastfoot_2border_above, 0);
		fl_set_button(longtable_options_->check_lastfoot_2border_above, 0);
		fl_set_button(longtable_options_->check_lastfoot_empty, 0);
		fl_set_button(longtable_options_->check_lt_newpage, 0);
	}
	fl_set_button(tabular_options_->check_rotate_tabular,
		      tabular->GetRotateTabular());
}


bool FormTabular::input(FL_OBJECT * ob, long)
{
	if (!inset_)
		return false;

	int s;
	LyXTabular::Feature num = LyXTabular::LAST_ACTION;
	string special;;

	int cell = inset_->getActCell();

	// ugly hack to auto-apply the stuff that hasn't been
	// yet. don't let this continue to exist ...
	if (ob == dialog_->button_close) {
		closing_ = true;
		LyXTabular * tabular = inset_->tabular.get();
		string str1 =
			getLengthFromWidgets(column_options_->input_column_width,
					     column_options_->choice_value_column_width);
		string str2;
		LyXLength llen = tabular->GetColumnPWidth(cell);
		if (!llen.zero())
			str2 = llen.asString();
		if (str1 != str2)
			input(column_options_->input_column_width, 0);
		str1 = getLengthFromWidgets(cell_options_->input_mcolumn_width,
					    cell_options_->choice_value_mcolumn_width);
		llen = tabular->GetMColumnPWidth(cell);
		if (llen.zero())
			str2 = "";
		else
			str2 = llen.asString();
		if (str1 != str2)
			input(cell_options_->input_mcolumn_width, 0);
		str1 = fl_get_input(column_options_->input_special_alignment);
		str2 = tabular->GetAlignSpecial(cell, LyXTabular::SET_SPECIAL_COLUMN);
		if (str1 != str2)
			input(column_options_->input_special_alignment, 0);
		str1 = fl_get_input(cell_options_->input_special_multialign);
		str2 = tabular->GetAlignSpecial(cell, LyXTabular::SET_SPECIAL_MULTI);
		if (str1 != str2)
			input(cell_options_->input_special_multialign, 0);
		closing_ = false;
		ok();
		return true;
	}

	if (actCell_ != cell) {
		update();
		postWarning(_("Wrong Cursor position, updated window"));
		return false;
	}
	// No point in processing directives that you can't do anything with
	// anyhow, so exit now if the buffer is read-only.
	if (lv_.buffer()->isReadonly()) {
		update();
		return false;
	}
	if ((ob == column_options_->input_column_width) ||
	    (ob == column_options_->choice_value_column_width)) {
		string const str =
			getLengthFromWidgets(column_options_->input_column_width,
					     column_options_->choice_value_column_width);
		inset_->tabularFeatures(lv_.view().get(), LyXTabular::SET_PWIDTH, str);

		//check if the input is valid
		string const input =
			fl_get_input(column_options_->input_column_width);
		if (!input.empty() && !isValidLength(input) && !isStrDbl(input)) {
			postWarning(_("Invalid Length (valid example: 10mm)"));
			return false;
		}

		update(); // update for alignment
		return true;
	}

	if ((ob == cell_options_->input_mcolumn_width) ||
	    (ob == cell_options_->choice_value_mcolumn_width)) {
		string const str =
			getLengthFromWidgets(cell_options_->input_mcolumn_width,
					     cell_options_->choice_value_mcolumn_width);
		inset_->tabularFeatures(lv_.view().get(), LyXTabular::SET_MPWIDTH, str);

		//check if the input is valid
		string const input =
			fl_get_input(cell_options_->input_mcolumn_width);
		if (!input.empty() && !isValidLength(input) && !isStrDbl(input)) {
			postWarning(_("Invalid Length (valid example: 10mm)"));
			return false;
		}
		update(); // update for alignment
		return true;
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
	else if (ob == column_options_->radio_valign_top)
		num = LyXTabular::VALIGN_TOP;
	else if (ob == column_options_->radio_valign_bottom)
		num = LyXTabular::VALIGN_BOTTOM;
	else if (ob == column_options_->radio_valign_center)
		num = LyXTabular::VALIGN_CENTER;
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
		special = fl_get_input(column_options_->input_special_alignment);
		num = LyXTabular::SET_SPECIAL_COLUMN;
	} else if (ob == cell_options_->input_special_multialign) {
		special = fl_get_input(cell_options_->input_special_multialign);
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
	else if (ob == cell_options_->radio_valign_center)
		num = LyXTabular::M_VALIGN_CENTER;
	else
		return false;

	inset_->tabularFeatures(lv_.view().get(), num, special);
	update();

	return true;
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
