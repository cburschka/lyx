/* FormTabular.C
 * FormTabular Interface Class Implementation
 */

#include <config.h>
#include "lyx_gui_misc.h"
#include "gettext.h"
#include FORMS_H_LOCATION

#include "FormTabular.h"
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
C_GENERICCB(FormTabular, TabularCloseCB)
C_GENERICCB(FormTabular, TabularInputCB)


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
    free();
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
			FL_RETURN_CHANGED);
    fl_set_input_return(column_options_->input_special_alignment,
			FL_RETURN_CHANGED);
    fl_set_input_return(cell_options_->input_mcolumn_width,
			FL_RETURN_CHANGED);
    fl_set_input_return(cell_options_->input_special_multialign,
			FL_RETURN_CHANGED);

    fl_addto_tabfolder(dialog_->tabFolder, _("Tabular"),
		       tabular_options_->form_tabular_options);
    fl_addto_tabfolder(dialog_->tabFolder, _("Column/Row"),
		       column_options_->form_column_options);
    fl_addto_tabfolder(dialog_->tabFolder, _("Cell"),
		       cell_options_->form_cell_options);
    fl_addto_tabfolder(dialog_->tabFolder, _("LongTable"),
		       longtable_options_->form_longtable_options);
    
    fl_set_form_atclose(dialog_->form_tabular,
			C_FormTabularWMHideCB, 0);

    fl_set_slider_bounds(create_tabular_->slider_rows, 1, 50);
    fl_set_slider_bounds(create_tabular_->slider_columns, 1, 50);
    fl_set_slider_value(create_tabular_->slider_rows, 5);
    fl_set_slider_value(create_tabular_->slider_columns, 5);
    fl_set_slider_precision(create_tabular_->slider_rows, 0);
    fl_set_slider_precision(create_tabular_->slider_columns, 0);
    fl_set_form_atclose(create_tabular_->form_create_tabular, 
                        C_FormTabularWMHideCB, 0);
}


void FormTabular::show()
{
    if (!dialog_) {
	build();
    }
    update();  // make sure its up-to-date

    if (dialog_->form_tabular->visible) {
	fl_raise_form(dialog_->form_tabular);
    } else {
	fl_show_form(dialog_->form_tabular,
		     FL_PLACE_MOUSE | FL_FREE_SIZE,
		     FL_FULLBORDER,
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
    if (dialog_ && dialog_->form_tabular && dialog_->form_tabular->visible) {
	fl_hide_form(dialog_->form_tabular);
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


void FormTabular::apply()
{
}


void FormTabular::update()
{
    if (dialog_) {
	local_update(true);
    }
}

void FormTabular::updateInset(InsetTabular *)
{
    update();
}

void FormTabular::input()
{
    bool activate = true;
    //
    // whatever checks you need
    //
    if (activate) {
    } else {
    }
}


void FormTabular::free()
{
    // we don't need to delete u and h here because
    // hide() does that after disconnecting.
    if (dialog_) {
	if (dialog_->form_tabular
	    && dialog_->form_tabular->visible) {
	    hide();
	}
	fl_free_form(dialog_->form_tabular);
	delete dialog_;
	dialog_ = 0;
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


void FormTabular::TabularCloseCB(FL_OBJECT * ob, long)
{
    FormTabular * pre = static_cast<FormTabular*>(ob->form->u_vdata);
    pre->hide();
}


void FormTabular::TabularInputCB(FL_OBJECT * ob, long)
{
    FormTabular * pre = static_cast<FormTabular*>(ob->form->u_vdata);
    pre->input();
}


bool FormTabular::local_update(bool flag)
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
    confirmed_ = false;
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
    if (tabular->IsMultiColumn(cell))
        fl_set_button(cell_options_->radio_multicolumn, 1);
    else
        fl_set_button(cell_options_->radio_multicolumn, 0);
    if (tabular->GetRotateCell(cell))
        fl_set_button(cell_options_->radio_rotate_cell, 1);
    else
        fl_set_button(cell_options_->radio_rotate_cell, 0);
    if (tabular->TopLine(cell))
        fl_set_button(column_options_->radio_border_top, 1);
    else
        fl_set_button(column_options_->radio_border_top, 0);
    if (tabular->BottomLine(cell))
        fl_set_button(column_options_->radio_border_bottom, 1);
    else
        fl_set_button(column_options_->radio_border_bottom, 0);
    if (tabular->LeftLine(cell))
        fl_set_button(column_options_->radio_border_left, 1);
    else
        fl_set_button(column_options_->radio_border_left, 0);
    if (tabular->RightLine(cell))
        fl_set_button(column_options_->radio_border_right, 1);
    else
        fl_set_button(column_options_->radio_border_right, 0);
    align = tabular->GetAlignment(cell);
    fl_set_button(column_options_->radio_align_left, 0);
    fl_set_button(column_options_->radio_align_right, 0);
    fl_set_button(column_options_->radio_align_center, 0);
    special = tabular->GetAlignSpecial(cell,LyXTabular::SET_SPECIAL_COLUMN);
    if (flag)
        fl_set_input(column_options_->input_special_alignment,
		     special.c_str());
    if (lv_->buffer()->isReadonly()) 
        fl_deactivate_object(column_options_->input_special_alignment);
    special = tabular->GetAlignSpecial(cell,LyXTabular::SET_SPECIAL_MULTI);
    if (flag)
        fl_set_input(cell_options_->input_special_multialign, special.c_str());
    if (lv_->buffer()->isReadonly()) 
        fl_deactivate_object(cell_options_->input_special_multialign);
    pwidth = tabular->GetPWidth(cell);
    if (flag)
        fl_set_input(column_options_->input_column_width,pwidth.c_str());
    if (lv_->buffer()->isReadonly()) {
        fl_deactivate_object(column_options_->input_column_width);
    } else {
        fl_activate_object(column_options_->input_column_width);
    }
    if (!pwidth.empty()) {
        fl_activate_object(cell_options_->radio_linebreak_cell);
        fl_set_object_lcol(cell_options_->radio_linebreak_cell, FL_BLACK);
        fl_set_button(cell_options_->radio_linebreak_cell,
                      tabular->GetLinebreaks(cell));
    } else {
        fl_deactivate_object(cell_options_->radio_linebreak_cell);
        fl_set_object_lcol(cell_options_->radio_linebreak_cell, FL_INACTIVE);
        fl_set_button(cell_options_->radio_linebreak_cell,0);
    }
    if ((!pwidth.empty() && !tabular->IsMultiColumn(cell)) ||
        (align == LYX_ALIGN_LEFT))
        fl_set_button(column_options_->radio_align_left, 1);
    else if (align == LYX_ALIGN_RIGHT)
        fl_set_button(column_options_->radio_align_right, 1);
    else
        fl_set_button(column_options_->radio_align_center, 1);
    if (!pwidth.empty() && !tabular->IsMultiColumn(cell)) {
        fl_deactivate_object(column_options_->radio_align_left);
        fl_deactivate_object(column_options_->radio_align_right);
        fl_deactivate_object(column_options_->radio_align_center);
        fl_set_object_lcol(column_options_->radio_align_left, FL_INACTIVE);
        fl_set_object_lcol(column_options_->radio_align_right, FL_INACTIVE);
        fl_set_object_lcol(column_options_->radio_align_center, FL_INACTIVE);
    } else {
        fl_activate_object(column_options_->radio_align_left);
        fl_activate_object(column_options_->radio_align_right);
        fl_activate_object(column_options_->radio_align_center);
        fl_set_object_lcol(column_options_->radio_align_left, FL_BLACK);
        fl_set_object_lcol(column_options_->radio_align_right, FL_BLACK);
        fl_set_object_lcol(column_options_->radio_align_center, FL_BLACK);
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
        fl_set_button(longtable_options_->radio_lt_firsthead,
                      tabular->GetRowOfLTFirstHead(cell));
        fl_set_button(longtable_options_->radio_lt_head,
		      tabular->GetRowOfLTHead(cell));
        fl_set_button(longtable_options_->radio_lt_foot,
		      tabular->GetRowOfLTFoot(cell));
        fl_set_button(longtable_options_->radio_lt_lastfoot,
                      tabular->GetRowOfLTLastFoot(cell));
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

// +-----------------------------------------------------------------------+
// |          Functions/Dialogs for creating tabular insets                |
// +-----------------------------------------------------------------------+

void FormTabular::show_create()
{
    if (!dialog_) {
	build();
    }
    if (create_tabular_->form_create_tabular->visible) {
        fl_raise_form(create_tabular_->form_create_tabular);
    } else {
        fl_show_form(create_tabular_->form_create_tabular,
		     FL_PLACE_MOUSE | FL_FREE_SIZE,
                     FL_FULLBORDER, _("Insert Tabular"));
    }
}


void FormTabular::hide_create()
{
    if (create_tabular_->form_create_tabular)
        fl_hide_form(create_tabular_->form_create_tabular);
}


void FormTabular::apply_create()
{
    int
        xsize,ysize;

//    comm->setMinibuffer(_("Inserting tabular inset..."));
    ysize = int(fl_get_slider_value(create_tabular_->slider_columns) + 0.5);
    xsize = int(fl_get_slider_value(create_tabular_->slider_rows) + 0.5);
    InsetTabular * in = new InsetTabular(lv_->buffer(),xsize,ysize);
    if (!lv_->view()->open_new_inset(in)) {
	delete in;
    }
//    comm->setMinibuffer(_("Tabular mode"));
}


void FormTabular::TabularOKCB(FL_OBJECT * ob, long)
{
    FormTabular * pre = (FormTabular*)ob->form->u_vdata;
    pre->apply_create();
    pre->hide_create();
}


void FormTabular::TabularApplyCB(FL_OBJECT * ob, long)
{
    FormTabular * pre = (FormTabular*)ob->form->u_vdata;
    pre->apply_create();
}


void FormTabular::TabularCancelCB(FL_OBJECT * ob, long)
{
    FormTabular * pre = (FormTabular*)ob->form->u_vdata;
    pre->hide_create();
}
