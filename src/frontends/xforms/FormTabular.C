/* FormTabular.C
 * FormTabular Interface Class Implementation
 */

#include <config.h>
#include "lyx_gui_misc.h"
#include "gettext.h"
#include FORMS_H_LOCATION

#include "FormTabular.h"
#include "form_tabular.h"
#include "xform_macros.h"
#include "input_validators.h"
#include "LyXView.h"
#include "lyxfunc.h"
#include "Dialogs.h"
#include "lyxrc.h"
#include "debug.h"
#include "insets/insettabular.h"
#include "buffer.h"


#ifdef SIGC_CXX_NAMESPACES
using SigC::slot;
#endif

C_RETURNCB(FormTabular,  WMHideCB)
C_GENERICCB(FormTabular, CloseCB)
C_GENERICCB(FormTabular, InputCB)
C_GENERICCB(FormTabular, OKCB)
C_GENERICCB(FormTabular, ApplyCB)
C_GENERICCB(FormTabular, CancelCB)


FormTabular::FormTabular(LyXView * lv, Dialogs * d)
	: dialog_(0), tabular_options_(0), column_options_(0),
	  cell_options_(0), longtable_options_(0), create_tabular_(0),
	  lv_(lv), d_(d), u_(0), h_(0), inset_(0), actCell_(-1) 
{
    // let the dialog be shown
    // This is a permanent connection so we won't bother
    // storing a copy because we won't be disconnecting.
    d->showTabular.connect(slot(this, &FormTabular::showInset));
    d->updateTabular.connect(slot(this, &FormTabular::updateInset));
    d->hideTabular.connect(slot(this, &FormTabular::hideInset));
    d->showTabularCreate.connect(slot(this, &FormTabular::show_create));
}


FormTabular::~FormTabular()
{
    // we don't need to disconnect u and h here because
    // their destructors do that.
    delete dialog_;
    delete tabular_options_;
    delete column_options_;
    delete cell_options_;
    delete longtable_options_;
    delete create_tabular_;
}


void FormTabular::build()
{
    dialog_ = build_tabular();
    tabular_options_ = build_tabular_options();
    column_options_ = build_column_options();
    cell_options_ = build_cell_options();
    longtable_options_ = build_longtable_options();
    create_tabular_ = build_create_tabular();

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
    
    fl_set_form_atclose(dialog_->form,
			C_FormTabularWMHideCB, 0);

    fl_set_slider_bounds(create_tabular_->slider_rows, 1, 50);
    fl_set_slider_bounds(create_tabular_->slider_columns, 1, 50);
    fl_set_slider_value(create_tabular_->slider_rows, 5);
    fl_set_slider_value(create_tabular_->slider_columns, 5);
    fl_set_slider_precision(create_tabular_->slider_rows, 0);
    fl_set_slider_precision(create_tabular_->slider_columns, 0);
    fl_set_form_atclose(create_tabular_->form, 
                        C_FormTabularWMHideCB, 0);
}


void FormTabular::show()
{
    if (!dialog_) {
	build();
    }
    update();  // make sure its up-to-date

    if (dialog_->form->visible) {
	fl_raise_form(dialog_->form);
    } else {
	fl_show_form(dialog_->form,
		     FL_PLACE_MOUSE | FL_FREE_SIZE,
		     FL_TRANSIENT,
		     _("Tabular Layout"));
//	u_ = d_->updateBufferDependent.connect(slot(this,
//						    &FormTabular::update));
	h_ = d_->hideBufferDependent.connect(slot(this,
						  &FormTabular::hide));
    }
}

void FormTabular::showInset(InsetTabular * ti)
{
    inset_ = ti;
    if (ti) {
	show();
    }
}

void FormTabular::hide()
{
    if (dialog_ && dialog_->form && dialog_->form->visible) {
	fl_hide_form(dialog_->form);
	u_.disconnect();
	h_.disconnect();
	inset_ = 0;
    }
}


void FormTabular::hideInset(InsetTabular * ti)
{
    if (inset_ == ti) {
	inset_ = 0;
	hide();
    }
}


void FormTabular::update()
{
    if (dialog_) {
	local_update(true);
    }
}

void FormTabular::updateInset(InsetTabular * ti)
{
    inset_ = ti;
    if (ti && dialog_ && dialog_->form->visible) {
	update();
    }
}


int FormTabular::WMHideCB(FL_FORM * form, void *)
{
    // Ensure that the signals (u and h) are disconnected even if the
    // window manager is used to close the dialog.
    FormTabular * pre = static_cast<FormTabular*>(form->u_vdata);
    pre->hide();
    return FL_CANCEL;
}


void FormTabular::CloseCB(FL_OBJECT * ob, long)
{
    FormTabular * pre = static_cast<FormTabular*>(ob->form->u_vdata);
    pre->hide();
}


void FormTabular::InputCB(FL_OBJECT * ob, long l)
{
    FormTabular * pre = static_cast<FormTabular*>(ob->form->u_vdata);
    pre->SetTabularOptions(ob, l);
}


bool FormTabular::local_update(bool)
{
    if (!inset_ || !inset_->tabular)
        return false;

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
    return true;
}

void FormTabular::SetTabularOptions(FL_OBJECT * ob, long)
{
    if (!inset_)
	return;

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
        local_update(false);
        fl_set_object_label(dialog_->text_warning,
                     _("Warning: Wrong Cursor position, updated window"));
        fl_show_object(dialog_->text_warning);
        return;
    }
    // No point in processing directives that you can't do anything with
    // anyhow, so exit now if the buffer is read-only.
    if (lv_->buffer()->isReadonly()) {
      local_update(false);
      return;
    }
    if (ob == column_options_->input_column_width) {
        string
            str;
        str = fl_get_input(ob);
        if (!str.empty() && !isValidLength(str)) {
            fl_set_object_label(dialog_->text_warning,
                 _("Warning: Invalid Length (valid example: 10mm)"));
            fl_show_object(dialog_->text_warning);
            return;
        }
        inset_->TabularFeatures(lv_->view(), LyXTabular::SET_PWIDTH,str);
        local_update(false); // update for alignment
        return;
    }
    if (ob == cell_options_->input_mcolumn_width) {
        string
            str;
        str = fl_get_input(ob);
        if (!str.empty() && !isValidLength(str)) {
            fl_set_object_label(dialog_->text_warning,
                 _("Warning: Invalid Length (valid example: 10mm)"));
            fl_show_object(dialog_->text_warning);
            return;
        }
        inset_->TabularFeatures(lv_->view(), LyXTabular::SET_MPWIDTH,str);
        local_update(false); // update for alignment
        return;
    }
    str = fl_get_input(column_options_->input_column_width);
    if (!str.empty() && !isValidLength(str)) {
        fl_set_object_label(
	    dialog_->text_warning,
	    _("Warning: Invalid Length (valid example: 10mm)"));
        fl_show_object(dialog_->text_warning);
        return;
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
        return;
    
    inset_->TabularFeatures(lv_->view(), num, special);
    local_update(false);
}

// +-----------------------------------------------------------------------+
// |          Functions/Dialogs for creating tabular insets                |
// +-----------------------------------------------------------------------+

void FormTabular::show_create()
{
    if (!dialog_) {
	build();
    }
    if (create_tabular_->form->visible) {
        fl_raise_form(create_tabular_->form);
    } else {
        fl_show_form(create_tabular_->form,
		     FL_PLACE_MOUSE | FL_FREE_SIZE,
                     FL_FULLBORDER, _("Insert Tabular"));
    }
}


void FormTabular::apply_create()
{
    int
        xsize,ysize;

//    comm->setMinibuffer(_("Inserting tabular inset..."));
    ysize = int(fl_get_slider_value(create_tabular_->slider_columns) + 0.5);
    xsize = int(fl_get_slider_value(create_tabular_->slider_rows) + 0.5);
    InsetTabular * in = new InsetTabular(*lv_->buffer(),xsize,ysize);
    if (!lv_->view()->open_new_inset(in)) {
	delete in;
    }
//    comm->setMinibuffer(_("Tabular mode"));
}


void FormTabular::hide_create()
{
    if (create_tabular_->form &&
	create_tabular_->form->visible)
        fl_hide_form(create_tabular_->form);
}


void FormTabular::OKCB(FL_OBJECT * ob, long)
{
    FormTabular * pre = (FormTabular*)ob->form->u_vdata;
    pre->apply_create();
    pre->hide_create();
}


void FormTabular::ApplyCB(FL_OBJECT * ob, long)
{
    FormTabular * pre = (FormTabular*)ob->form->u_vdata;
    pre->apply_create();
}


void FormTabular::CancelCB(FL_OBJECT * ob, long)
{
    FormTabular * pre = (FormTabular*)ob->form->u_vdata;
    pre->hide_create();
}


