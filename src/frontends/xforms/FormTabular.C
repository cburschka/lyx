// -*- C++ -*-
/* This file is part of
 * ======================================================
 * 
 *           LyX, The Document Processor
 * 	 
 *	    Copyright 1995 Matthias Ettrich
 *          Copyright 1995-2000 The LyX Team.
 *
 *======================================================*/
/* FormTabular.C
 * FormTabular Interface Class Implementation
 */

#include <config.h>

#include "FormTabular.h"
#include "form_tabular.h"
#include "LyXView.h"
#include "Dialogs.h"
#include "insets/insettabular.h"
#include "buffer.h"


FormTabular::FormTabular(LyXView * lv, Dialogs * d)
	: FormInset( lv, d, _("Tabular Layout") ),
	  dialog_(0), tabular_options_(0), column_options_(0),
	  cell_options_(0), longtable_options_(0),
	  inset_(0), actCell_(-1)
{
	// let the dialog be shown
	// This is a permanent connection so we won't bother
	// storing a copy because we won't be disconnecting.
	d->showTabular.connect(slot(this, &FormTabular::showInset));
	d->updateTabular.connect(slot(this, &FormTabular::updateInset));
}


FormTabular::~FormTabular()
{
	delete dialog_;
	delete tabular_options_;
	delete column_options_;
	delete cell_options_;
	delete longtable_options_;
}


FL_FORM * FormTabular::form() const
{
	if (dialog_ ) return dialog_->form;
	return 0;
}


void FormTabular::disconnect()
{
	inset_ = 0;
	FormInset::disconnect();
}


void FormTabular::showInset( InsetTabular * inset )
{
	if (inset == 0) return;

	// If connected to another inset, disconnect from it.
	if (inset_ != inset) {
		ih_.disconnect();
		ih_ = inset->hideDialog.connect(slot(this, &FormTabular::hide));
		inset_ = inset;
	}

	show();
}


void FormTabular::updateInset( InsetTabular * inset )
{
	if (inset == 0 || inset_ == 0) return;

	// If connected to another inset, disconnect from it.
	if (inset_ != inset) {
		ih_.disconnect();
		ih_ = inset->hideDialog.connect(slot(this, &FormTabular::hide));
		inset_ = inset;
	}

	update();
}


void FormTabular::build()
{
	dialog_ = build_tabular();
	tabular_options_ = build_tabular_options();
	column_options_ = build_column_options();
	cell_options_ = build_cell_options();
	longtable_options_ = build_longtable_options();

	// Workaround dumb xforms sizing bug
	minw_ = form()->w;
	minh_ = form()->h;

	fl_set_input_return(column_options_->input_column_width,
			    FL_RETURN_END);
	fl_set_input_return(column_options_->input_special_alignment,
			    FL_RETURN_CHANGED);
	fl_set_input_return(cell_options_->input_mcolumn_width,
			    FL_RETURN_CHANGED);
	fl_set_input_return(cell_options_->input_special_multialign,
			    FL_RETURN_CHANGED);

	fl_addto_tabfolder(dialog_->tabFolder, _("Tabular"),
			   tabular_options_->form);
	fl_addto_tabfolder(dialog_->tabFolder, _("Column/Row"),
			   column_options_->form);
	fl_addto_tabfolder(dialog_->tabFolder, _("Cell"),
			   cell_options_->form);
	fl_addto_tabfolder(dialog_->tabFolder, _("LongTable"),
			   longtable_options_->form);
}


void FormTabular::update()
{
	if (!inset_ || !inset_->tabular)
		return;

	LyXTabular
		* tabular = inset_->tabular;
	int
		align,
		cell,
		column,row;
	char
		buf[12];
	string
		pwidth, special;

	actCell_ = cell = inset_->GetActCell();
	column = tabular->column_of_cell(cell)+1;
	fl_set_object_label(dialog_->text_warning,"");
	fl_activate_object(column_options_->input_special_alignment);
	fl_activate_object(cell_options_->input_special_multialign);
	fl_activate_object(column_options_->input_column_width);
	sprintf(buf,"%d",column);
	fl_set_input(dialog_->input_tabular_column, buf);
	fl_deactivate_object(dialog_->input_tabular_column);
	row = tabular->row_of_cell(cell)+1;
	sprintf(buf,"%d",row);
	fl_set_input(dialog_->input_tabular_row, buf);
	fl_deactivate_object(dialog_->input_tabular_row);
	if (tabular->IsMultiColumn(cell)) {
		fl_set_button(cell_options_->radio_multicolumn, 1);
		fl_set_button(cell_options_->radio_border_top,
			      tabular->TopLine(cell)?1:0);
		fl_activate_object(cell_options_->radio_border_top);
		fl_set_object_lcol(cell_options_->radio_border_top, FL_BLACK);
		fl_set_button(cell_options_->radio_border_bottom,
			      tabular->BottomLine(cell)?1:0);
		fl_activate_object(cell_options_->radio_border_bottom);
		fl_set_object_lcol(cell_options_->radio_border_bottom, FL_BLACK);
		fl_set_button(cell_options_->radio_border_left,
			      tabular->LeftLine(cell)?1:0);
		fl_activate_object(cell_options_->radio_border_left);
		fl_set_object_lcol(cell_options_->radio_border_left, FL_BLACK);
		fl_set_button(cell_options_->radio_border_right,
			      tabular->RightLine(cell)?1:0);
		fl_activate_object(cell_options_->radio_border_right);
		fl_set_object_lcol(cell_options_->radio_border_right, FL_BLACK);
		pwidth = tabular->GetMColumnPWidth(cell);
		align = tabular->GetAlignment(cell);
		if (!pwidth.empty() || (align == LYX_ALIGN_LEFT))
			fl_set_button(cell_options_->radio_align_left, 1);
		else if (align == LYX_ALIGN_RIGHT)
			fl_set_button(cell_options_->radio_align_right, 1);
		else
			fl_set_button(cell_options_->radio_align_center, 1);
		fl_activate_object(cell_options_->radio_align_left);
		fl_set_object_lcol(cell_options_->radio_align_left, FL_BLACK);
		fl_activate_object(cell_options_->radio_align_right);
		fl_set_object_lcol(cell_options_->radio_align_right, FL_BLACK);
		fl_activate_object(cell_options_->radio_align_center);
		fl_set_object_lcol(cell_options_->radio_align_center, FL_BLACK);
		align = tabular->GetVAlignment(cell);
		fl_set_button(cell_options_->radio_valign_top, 0);
		fl_set_button(cell_options_->radio_valign_bottom, 0);
		fl_set_button(cell_options_->radio_valign_center, 0);
		if (pwidth.empty() || (align == LyXTabular::LYX_VALIGN_CENTER))
			fl_set_button(cell_options_->radio_valign_center, 1);
		else if (align == LyXTabular::LYX_VALIGN_BOTTOM)
			fl_set_button(cell_options_->radio_valign_bottom, 1);
		else
			fl_set_button(cell_options_->radio_valign_top, 1);
		fl_activate_object(cell_options_->radio_valign_top);
		fl_set_object_lcol(cell_options_->radio_valign_top, FL_BLACK);
		fl_activate_object(cell_options_->radio_valign_bottom);
		fl_set_object_lcol(cell_options_->radio_valign_bottom, FL_BLACK);
		fl_activate_object(cell_options_->radio_valign_center);
		fl_set_object_lcol(cell_options_->radio_valign_center, FL_BLACK);
		special = tabular->GetAlignSpecial(cell,LyXTabular::SET_SPECIAL_MULTI);
		fl_set_input(cell_options_->input_special_multialign, special.c_str());
		fl_set_input(cell_options_->input_mcolumn_width,pwidth.c_str());
		if (!lv_->buffer()->isReadonly()) {
			fl_activate_object(cell_options_->input_special_multialign);
			fl_set_object_lcol(cell_options_->input_special_multialign,
					   FL_BLACK);
			fl_activate_object(cell_options_->input_mcolumn_width);
			fl_set_object_lcol(cell_options_->input_mcolumn_width, FL_BLACK);
		}
		if (!pwidth.empty()) {
			fl_deactivate_object(cell_options_->radio_align_left);
			fl_deactivate_object(cell_options_->radio_align_right);
			fl_deactivate_object(cell_options_->radio_align_center);
			fl_set_object_lcol(cell_options_->radio_align_left, FL_INACTIVE);
			fl_set_object_lcol(cell_options_->radio_align_right, FL_INACTIVE);
			fl_set_object_lcol(cell_options_->radio_align_center, FL_INACTIVE);
			fl_activate_object(cell_options_->radio_valign_top);
			fl_activate_object(cell_options_->radio_valign_bottom);
			fl_activate_object(cell_options_->radio_valign_center);
			fl_set_object_lcol(cell_options_->radio_valign_top, FL_BLACK);
			fl_set_object_lcol(cell_options_->radio_valign_bottom, FL_BLACK);
			fl_set_object_lcol(cell_options_->radio_valign_center, FL_BLACK);
		} else {
			fl_activate_object(cell_options_->radio_align_left);
			fl_activate_object(cell_options_->radio_align_right);
			fl_activate_object(cell_options_->radio_align_center);
			fl_set_object_lcol(cell_options_->radio_align_left, FL_BLACK);
			fl_set_object_lcol(cell_options_->radio_align_right, FL_BLACK);
			fl_set_object_lcol(cell_options_->radio_align_center, FL_BLACK);
			fl_deactivate_object(cell_options_->radio_valign_top);
			fl_deactivate_object(cell_options_->radio_valign_bottom);
			fl_deactivate_object(cell_options_->radio_valign_center);
			fl_set_object_lcol(cell_options_->radio_valign_top, FL_INACTIVE);
			fl_set_object_lcol(cell_options_->radio_valign_bottom,FL_INACTIVE);
			fl_set_object_lcol(cell_options_->radio_valign_center,FL_INACTIVE);
		}
	} else {
		fl_set_button(cell_options_->radio_multicolumn, 0);
		fl_set_button(cell_options_->radio_border_top, 0);
		fl_deactivate_object(cell_options_->radio_border_top);
		fl_set_object_lcol(cell_options_->radio_border_top, FL_INACTIVE);
		fl_set_button(cell_options_->radio_border_bottom, 0);
		fl_deactivate_object(cell_options_->radio_border_bottom);
		fl_set_object_lcol(cell_options_->radio_border_bottom, FL_INACTIVE);
		fl_set_button(cell_options_->radio_border_left, 0);
		fl_deactivate_object(cell_options_->radio_border_left);
		fl_set_object_lcol(cell_options_->radio_border_left, FL_INACTIVE);
		fl_set_button(cell_options_->radio_border_right, 0);
		fl_deactivate_object(cell_options_->radio_border_right);
		fl_set_object_lcol(cell_options_->radio_border_right, FL_INACTIVE);
		fl_set_button(cell_options_->radio_align_left, 0);
		fl_deactivate_object(cell_options_->radio_align_left);
		fl_set_object_lcol(cell_options_->radio_align_left, FL_INACTIVE);
		fl_set_button(cell_options_->radio_align_right, 0);
		fl_deactivate_object(cell_options_->radio_align_right);
		fl_set_object_lcol(cell_options_->radio_align_right, FL_INACTIVE);
		fl_set_button(cell_options_->radio_align_center, 0);
		fl_deactivate_object(cell_options_->radio_align_center);
		fl_set_object_lcol(cell_options_->radio_align_center, FL_INACTIVE);
		fl_set_button(cell_options_->radio_valign_top, 0);
		fl_deactivate_object(cell_options_->radio_valign_top);
		fl_set_object_lcol(cell_options_->radio_valign_top, FL_INACTIVE);
		fl_set_button(cell_options_->radio_valign_bottom, 0);
		fl_deactivate_object(cell_options_->radio_valign_bottom);
		fl_set_object_lcol(cell_options_->radio_valign_bottom, FL_INACTIVE);
		fl_set_button(cell_options_->radio_valign_center, 0);
		fl_deactivate_object(cell_options_->radio_valign_center);
		fl_set_object_lcol(cell_options_->radio_valign_center, FL_INACTIVE);
		fl_set_input(cell_options_->input_special_multialign, "");
		fl_deactivate_object(cell_options_->input_special_multialign);
		fl_set_object_lcol(cell_options_->input_special_multialign, FL_INACTIVE);
		fl_set_input(cell_options_->input_mcolumn_width,"");
		fl_deactivate_object(cell_options_->input_mcolumn_width);
		fl_set_object_lcol(cell_options_->input_mcolumn_width, FL_INACTIVE);
	}
	if (tabular->GetRotateCell(cell))
		fl_set_button(cell_options_->radio_rotate_cell, 1);
	else
		fl_set_button(cell_options_->radio_rotate_cell, 0);
	if (tabular->TopLine(cell, true))
		fl_set_button(column_options_->radio_border_top, 1);
	else
		fl_set_button(column_options_->radio_border_top, 0);
	if (tabular->BottomLine(cell, true))
		fl_set_button(column_options_->radio_border_bottom, 1);
	else
		fl_set_button(column_options_->radio_border_bottom, 0);
	if (tabular->LeftLine(cell, true))
		fl_set_button(column_options_->radio_border_left, 1);
	else
		fl_set_button(column_options_->radio_border_left, 0);
	if (tabular->RightLine(cell, true))
		fl_set_button(column_options_->radio_border_right, 1);
	else
		fl_set_button(column_options_->radio_border_right, 0);
	special = tabular->GetAlignSpecial(cell,LyXTabular::SET_SPECIAL_COLUMN);
	fl_set_input(column_options_->input_special_alignment, special.c_str());
	if (lv_->buffer()->isReadonly()) 
		fl_deactivate_object(column_options_->input_special_alignment);
	else
		fl_activate_object(column_options_->input_special_alignment);
	pwidth = tabular->GetColumnPWidth(cell);
	fl_set_input(column_options_->input_column_width,pwidth.c_str());
	if (lv_->buffer()->isReadonly()) {
		fl_deactivate_object(column_options_->input_column_width);
	} else {
		fl_activate_object(column_options_->input_column_width);
	}
	if (!pwidth.empty()) {
		fl_activate_object(cell_options_->radio_useminipage);
		fl_set_object_lcol(cell_options_->radio_useminipage, FL_BLACK);
		if (tabular->GetUsebox(cell) == 2)
			fl_set_button(cell_options_->radio_useminipage, 1);
		else
			fl_set_button(cell_options_->radio_useminipage, 0);
	} else {
		fl_deactivate_object(cell_options_->radio_useminipage);
		fl_set_object_lcol(cell_options_->radio_useminipage, FL_INACTIVE);
		fl_set_button(cell_options_->radio_useminipage,0);
	}
	align = tabular->GetAlignment(cell, true);
	fl_set_button(column_options_->radio_align_left, 0);
	fl_set_button(column_options_->radio_align_right, 0);
	fl_set_button(column_options_->radio_align_center, 0);
	if (!pwidth.empty() || (align == LYX_ALIGN_LEFT))
		fl_set_button(column_options_->radio_align_left, 1);
	else if (align == LYX_ALIGN_RIGHT)
		fl_set_button(column_options_->radio_align_right, 1);
	else
		fl_set_button(column_options_->radio_align_center, 1);
	align = tabular->GetVAlignment(cell, true);
	fl_set_button(column_options_->radio_valign_top, 0);
	fl_set_button(column_options_->radio_valign_bottom, 0);
	fl_set_button(column_options_->radio_valign_center, 0);
	if (pwidth.empty() || (align == LyXTabular::LYX_VALIGN_CENTER))
		fl_set_button(column_options_->radio_valign_center, 1);
	else if (align == LyXTabular::LYX_VALIGN_BOTTOM)
		fl_set_button(column_options_->radio_valign_bottom, 1);
	else
		fl_set_button(column_options_->radio_valign_top, 1);
	if (!pwidth.empty()) {
		fl_deactivate_object(column_options_->radio_align_left);
		fl_deactivate_object(column_options_->radio_align_right);
		fl_deactivate_object(column_options_->radio_align_center);
		fl_set_object_lcol(column_options_->radio_align_left, FL_INACTIVE);
		fl_set_object_lcol(column_options_->radio_align_right, FL_INACTIVE);
		fl_set_object_lcol(column_options_->radio_align_center, FL_INACTIVE);
		fl_activate_object(column_options_->radio_valign_top);
		fl_activate_object(column_options_->radio_valign_bottom);
		fl_activate_object(column_options_->radio_valign_center);
		fl_set_object_lcol(column_options_->radio_valign_top, FL_BLACK);
		fl_set_object_lcol(column_options_->radio_valign_bottom, FL_BLACK);
		fl_set_object_lcol(column_options_->radio_valign_center, FL_BLACK);
	} else {
		fl_activate_object(column_options_->radio_align_left);
		fl_activate_object(column_options_->radio_align_right);
		fl_activate_object(column_options_->radio_align_center);
		fl_set_object_lcol(column_options_->radio_align_left, FL_BLACK);
		fl_set_object_lcol(column_options_->radio_align_right, FL_BLACK);
		fl_set_object_lcol(column_options_->radio_align_center, FL_BLACK);
		fl_deactivate_object(column_options_->radio_valign_top);
		fl_deactivate_object(column_options_->radio_valign_bottom);
		fl_deactivate_object(column_options_->radio_valign_center);
		fl_set_object_lcol(column_options_->radio_valign_top, FL_INACTIVE);
		fl_set_object_lcol(column_options_->radio_valign_bottom, FL_INACTIVE);
		fl_set_object_lcol(column_options_->radio_valign_center, FL_INACTIVE);
	}
	fl_set_button(tabular_options_->radio_longtable,
		      tabular->IsLongTabular());
	if (tabular->IsLongTabular()) {
		fl_activate_object(longtable_options_->radio_lt_firsthead);
		fl_activate_object(longtable_options_->radio_lt_head);
		fl_activate_object(longtable_options_->radio_lt_foot);
		fl_activate_object(longtable_options_->radio_lt_lastfoot);
		fl_activate_object(longtable_options_->radio_lt_newpage);
		fl_set_object_lcol(longtable_options_->radio_lt_firsthead, FL_BLACK);
		fl_set_object_lcol(longtable_options_->radio_lt_head, FL_BLACK);
		fl_set_object_lcol(longtable_options_->radio_lt_foot, FL_BLACK);
		fl_set_object_lcol(longtable_options_->radio_lt_lastfoot, FL_BLACK);
		fl_set_object_lcol(longtable_options_->radio_lt_newpage, FL_BLACK);
		int dummy;
		fl_set_button(longtable_options_->radio_lt_firsthead,
			      tabular->GetRowOfLTFirstHead(cell, dummy));
		fl_set_button(longtable_options_->radio_lt_head,
			      tabular->GetRowOfLTHead(cell, dummy));
		fl_set_button(longtable_options_->radio_lt_foot,
			      tabular->GetRowOfLTFoot(cell, dummy));
		fl_set_button(longtable_options_->radio_lt_lastfoot,
			      tabular->GetRowOfLTLastFoot(cell, dummy));
		fl_set_button(longtable_options_->radio_lt_newpage,
			      tabular->GetLTNewPage(cell));
	} else {
		fl_deactivate_object(longtable_options_->radio_lt_firsthead);
		fl_deactivate_object(longtable_options_->radio_lt_head);
		fl_deactivate_object(longtable_options_->radio_lt_foot);
		fl_deactivate_object(longtable_options_->radio_lt_lastfoot);
		fl_deactivate_object(longtable_options_->radio_lt_newpage);
		fl_set_button(longtable_options_->radio_lt_firsthead,0);
		fl_set_button(longtable_options_->radio_lt_head,0);
		fl_set_button(longtable_options_->radio_lt_foot,0);
		fl_set_button(longtable_options_->radio_lt_lastfoot,0);
		fl_set_button(longtable_options_->radio_lt_newpage,0);
		fl_set_object_lcol(longtable_options_->radio_lt_firsthead,FL_INACTIVE);
		fl_set_object_lcol(longtable_options_->radio_lt_head, FL_INACTIVE);
		fl_set_object_lcol(longtable_options_->radio_lt_foot, FL_INACTIVE);
		fl_set_object_lcol(longtable_options_->radio_lt_lastfoot, FL_INACTIVE);
		fl_set_object_lcol(longtable_options_->radio_lt_newpage, FL_INACTIVE);
	}
	fl_set_button(tabular_options_->radio_rotate_tabular,
		      tabular->GetRotateTabular());
}

bool FormTabular::input(FL_OBJECT * ob, long)
{
    if (!inset_)
        return false;

    LyXTabular
        * tabular = inset_->tabular;
    int
        cell,
        s;
    LyXTabular::Feature
        num = LyXTabular::LAST_ACTION;
    string
        special,
        str;

    cell = inset_->GetActCell();
    if (actCell_ != cell) {
        update();
        fl_set_object_label(dialog_->text_warning,
                     _("Warning: Wrong Cursor position, updated window"));
        fl_show_object(dialog_->text_warning);
        return false;
    }
    // No point in processing directives that you can't do anything with
    // anyhow, so exit now if the buffer is read-only.
    if (lv_->buffer()->isReadonly()) {
      update();
      return false;
    }
    if (ob == column_options_->input_column_width) {
        string
            str;
        str = fl_get_input(ob);
        if (!str.empty() && !isValidLength(str)) {
            fl_set_object_label(dialog_->text_warning,
                 _("Warning: Invalid Length (valid example: 10mm)"));
            fl_show_object(dialog_->text_warning);
            return false;
        }
        inset_->TabularFeatures(lv_->view(), LyXTabular::SET_PWIDTH,str);
        update(); // update for alignment
        return true;
    }
    if (ob == cell_options_->input_mcolumn_width) {
        string
            str;
        str = fl_get_input(ob);
        if (!str.empty() && !isValidLength(str)) {
            fl_set_object_label(dialog_->text_warning,
                 _("Warning: Invalid Length (valid example: 10mm)"));
            fl_show_object(dialog_->text_warning);
            return false;
        }
        inset_->TabularFeatures(lv_->view(), LyXTabular::SET_MPWIDTH,str);
        update(); // update for alignment
        return true;
    }
    str = fl_get_input(column_options_->input_column_width);
    if (!str.empty() && !isValidLength(str)) {
        fl_set_object_label(
	    dialog_->text_warning,
	    _("Warning: Invalid Length (valid example: 10mm)"));
        fl_show_object(dialog_->text_warning);
        return false;
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
    else if (ob == column_options_->radio_border_top)
        num = LyXTabular::TOGGLE_LINE_TOP;
    else if (ob == column_options_->radio_border_bottom)
        num = LyXTabular::TOGGLE_LINE_BOTTOM;
    else if (ob == column_options_->radio_border_left)
        num = LyXTabular::TOGGLE_LINE_LEFT;
    else if (ob == column_options_->radio_border_right)
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
    else if (ob == cell_options_->radio_multicolumn)
        num = LyXTabular::MULTICOLUMN;
    else if (ob == tabular_options_->radio_longtable) {
        s=fl_get_button(tabular_options_->radio_longtable);
        if (s) {
            num = LyXTabular::SET_LONGTABULAR;
            fl_activate_object(longtable_options_->radio_lt_firsthead);
            fl_activate_object(longtable_options_->radio_lt_head);
            fl_activate_object(longtable_options_->radio_lt_foot);
            fl_activate_object(longtable_options_->radio_lt_lastfoot);
            fl_activate_object(longtable_options_->radio_lt_newpage);
	    int dummy;
            fl_set_button(longtable_options_->radio_lt_firsthead,
                          tabular->GetRowOfLTFirstHead(cell, dummy));
            fl_set_button(longtable_options_->radio_lt_head,
			  tabular->GetRowOfLTHead(cell, dummy));
            fl_set_button(longtable_options_->radio_lt_foot,
			  tabular->GetRowOfLTFoot(cell, dummy));
            fl_set_button(longtable_options_->radio_lt_lastfoot,
                          tabular->GetRowOfLTLastFoot(cell, dummy));
            fl_set_button(longtable_options_->radio_lt_firsthead,
			  tabular->GetLTNewPage(cell));
        } else {
	    num = LyXTabular::UNSET_LONGTABULAR;
            fl_deactivate_object(longtable_options_->radio_lt_firsthead);
            fl_deactivate_object(longtable_options_->radio_lt_head);
            fl_deactivate_object(longtable_options_->radio_lt_foot);
            fl_deactivate_object(longtable_options_->radio_lt_lastfoot);
            fl_deactivate_object(longtable_options_->radio_lt_newpage);
            fl_set_button(longtable_options_->radio_lt_firsthead,0);
            fl_set_button(longtable_options_->radio_lt_head,0);
            fl_set_button(longtable_options_->radio_lt_foot,0);
            fl_set_button(longtable_options_->radio_lt_lastfoot,0);
            fl_set_button(longtable_options_->radio_lt_newpage,0);
	    fl_set_object_lcol(longtable_options_->radio_lt_firsthead,
			       FL_INACTIVE);
	    fl_set_object_lcol(longtable_options_->radio_lt_head, FL_INACTIVE);
	    fl_set_object_lcol(longtable_options_->radio_lt_foot, FL_INACTIVE);
	    fl_set_object_lcol(longtable_options_->radio_lt_lastfoot,
			       FL_INACTIVE);
	    fl_set_object_lcol(longtable_options_->radio_lt_newpage,
			       FL_INACTIVE);
        }
    } else if (ob == tabular_options_->radio_rotate_tabular) {
        s=fl_get_button(tabular_options_->radio_rotate_tabular);
	if (s)
            num = LyXTabular::SET_ROTATE_TABULAR;
	else
	    num = LyXTabular::UNSET_ROTATE_TABULAR;
    } else if (ob == cell_options_->radio_rotate_cell) {
        s=fl_get_button(cell_options_->radio_rotate_cell);
	if (s)
            num = LyXTabular::SET_ROTATE_CELL;
	else
	    num = LyXTabular::UNSET_ROTATE_CELL;
    } else if (ob == cell_options_->radio_useminipage) {
        num = LyXTabular::SET_USEBOX;
	special = "2";
    } else if (ob == longtable_options_->radio_lt_firsthead) {
        num = LyXTabular::SET_LTFIRSTHEAD;
    } else if (ob == longtable_options_->radio_lt_head) {
        num = LyXTabular::SET_LTHEAD;
    } else if (ob == longtable_options_->radio_lt_foot) {
        num = LyXTabular::SET_LTFOOT;
    } else if (ob == longtable_options_->radio_lt_lastfoot) {
        num = LyXTabular::SET_LTLASTFOOT;
    } else if (ob == longtable_options_->radio_lt_newpage) {
        num = LyXTabular::SET_LTNEWPAGE;
    } else if (ob == column_options_->input_special_alignment) {
        special = fl_get_input(column_options_->input_special_alignment);
        num = LyXTabular::SET_SPECIAL_COLUMN;
    } else if (ob == cell_options_->input_special_multialign) {
        special = fl_get_input(cell_options_->input_special_multialign);
        num = LyXTabular::SET_SPECIAL_MULTI;
    } else if (ob == cell_options_->radio_border_top)
        num = LyXTabular::M_TOGGLE_LINE_TOP;
    else if (ob == cell_options_->radio_border_bottom)
        num = LyXTabular::M_TOGGLE_LINE_BOTTOM;
    else if (ob == cell_options_->radio_border_left)
        num = LyXTabular::M_TOGGLE_LINE_LEFT;
    else if (ob == cell_options_->radio_border_right)
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
    
    inset_->TabularFeatures(lv_->view(), num, special);
    update();

    return true;
}
