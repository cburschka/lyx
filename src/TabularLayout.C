#include <config.h>

#include <cstdlib>
#include FORMS_H_LOCATION

#include "layout_forms.h"
#include "lyx_main.h"
#include "lyxrc.h"
#include "LString.h"
#include "support/filetools.h"
#include "buffer.h"
#include "vspace.h"
#include "lyx_gui_misc.h"
#include "BufferView.h"
#include "gettext.h"
#include "layout.h"
#include "tabular.h"
#include "insets/insettabular.h"


// Prototypes
extern FD_form_table_options * fd_form_table_options;
extern FD_form_table_extra * fd_form_table_extra;
extern BufferView * current_view;

extern void OpenLayoutTableExtra();
extern bool UpdateLayoutTabular(bool, InsetTabular *);

static int Confirmed = false;
static int ActCell;

// hack to keep the cursor from jumping to the end of the text in the Extra
// form input fields during editing. The values in LyXTabular itself is
// changed in real-time, but we have no callbacks for the input fields,
// so I simply store and restore the cursor position for now.
// (too much of a hazzle to do it proper; we'll trash all this code
// in 1.1 anyway)
static int extra_col_cursor_x; // need no y's, one-line input fields
static int extra_multicol_cursor_x;
// Joacim

static InsetTabular * inset = 0;

void MenuLayoutTabular(bool flag, InsetTabular * ins)
{
    inset = ins;
    if (!inset)
	return;
    if (UpdateLayoutTabular(flag, ins)) {
        if (fd_form_table_options->form_table_options->visible) {
            fl_raise_form(fd_form_table_options->form_table_options);
        }
        else {
            fl_show_form(fd_form_table_options->form_table_options,
                         FL_PLACE_MOUSE, FL_FULLBORDER,
                         _("Tabular Layout"));
        }
    }
}

bool UpdateLayoutTabular(bool flag, InsetTabular *ins)
{
    if (!ins)
	return false;

    inset = ins;

    bool update = true;
    if (!current_view->available())
        update = false;
    
    if (update) {
        char buf[12];
        string pwidth, special;
   
	LyXTabular * table = inset->tabular;

        int cell = inset->GetActCell();
        ActCell = cell;
        int column = table->column_of_cell(cell)+1;
        fl_set_object_label(fd_form_table_options->text_warning, "");
        Confirmed = false;
        fl_activate_object(fd_form_table_extra->input_special_alignment);
        fl_activate_object(fd_form_table_extra->input_special_multialign);
        fl_activate_object(fd_form_table_options->input_column_width);
        sprintf(buf, "%d", column);
        fl_set_input(fd_form_table_options->input_table_column, buf);
        fl_deactivate_object(fd_form_table_options->input_table_column);
        int row = table->row_of_cell(cell)+1;
        sprintf(buf, "%d", row);
        fl_set_input(fd_form_table_options->input_table_row, buf);
        fl_deactivate_object(fd_form_table_options->input_table_row);
        if (table->IsMultiColumn(cell))
            fl_set_button(fd_form_table_options->radio_multicolumn, 1);
        else
            fl_set_button(fd_form_table_options->radio_multicolumn, 0);
        if (table->GetRotateCell(cell))
            fl_set_button(fd_form_table_options->radio_rotate_cell, 1);
        else
            fl_set_button(fd_form_table_options->radio_rotate_cell, 0);
        if (table->TopLine(cell))
            fl_set_button(fd_form_table_options->radio_border_top, 1);
        else
            fl_set_button(fd_form_table_options->radio_border_top, 0);
        if (table->BottomLine(cell))
            fl_set_button(fd_form_table_options->radio_border_bottom, 1);
        else
            fl_set_button(fd_form_table_options->radio_border_bottom, 0);
        if (table->LeftLine(cell))
            fl_set_button(fd_form_table_options->radio_border_left, 1);
        else
            fl_set_button(fd_form_table_options->radio_border_left, 0);
        if (table->RightLine(cell))
            fl_set_button(fd_form_table_options->radio_border_right, 1);
        else
            fl_set_button(fd_form_table_options->radio_border_right, 0);
        int align = table->GetAlignment(cell);
        fl_set_button(fd_form_table_options->radio_align_left, 0);
        fl_set_button(fd_form_table_options->radio_align_right, 0);
        fl_set_button(fd_form_table_options->radio_align_center, 0);
        special = table->GetAlignSpecial(cell, LyXTabular::SET_SPECIAL_COLUMN);
        if (flag)
	{
            fl_set_input(fd_form_table_extra->input_special_alignment,
                         special.c_str());
	    fl_set_input_cursorpos(fd_form_table_extra->input_special_alignment,
		    extra_col_cursor_x, 0); // restore the cursor
	}
        if (current_view->buffer()->isReadonly()) 
            fl_deactivate_object(fd_form_table_extra->input_special_alignment);
        special = table->GetAlignSpecial(cell, LyXTabular::SET_SPECIAL_MULTI);
        if (flag)
	{
            fl_set_input(fd_form_table_extra->input_special_multialign,
                         special.c_str());
	    fl_set_input_cursorpos(fd_form_table_extra->input_special_multialign,
		    extra_multicol_cursor_x, 0); // restore the cursor
	}
        if (current_view->buffer()->isReadonly()) 
            fl_deactivate_object(fd_form_table_extra->input_special_multialign);
        pwidth = table->GetPWidth(cell);
        if (flag)
            fl_set_input(fd_form_table_options->input_column_width, pwidth.c_str());
        if (current_view->buffer()->isReadonly()) 
            fl_deactivate_object(fd_form_table_options->input_column_width);
        if (!pwidth.empty()) {
            fl_activate_object(fd_form_table_options->radio_linebreak_cell);
	    fl_set_object_lcol(fd_form_table_options->radio_linebreak_cell,
			       FL_BLACK);
            fl_set_button(fd_form_table_options->radio_linebreak_cell,
                          table->GetLinebreaks(cell));
        } else {
            fl_deactivate_object(fd_form_table_options->radio_linebreak_cell);
	    fl_set_object_lcol(fd_form_table_options->radio_linebreak_cell,
			       FL_INACTIVE);
            fl_set_button(fd_form_table_options->radio_linebreak_cell, 0);
        }
        if ((!pwidth.empty() && !table->IsMultiColumn(cell)) ||
            (align == LYX_ALIGN_LEFT))
            fl_set_button(fd_form_table_options->radio_align_left, 1);
        else if (align == LYX_ALIGN_RIGHT)
            fl_set_button(fd_form_table_options->radio_align_right, 1);
        else
            fl_set_button(fd_form_table_options->radio_align_center, 1);
        if (!pwidth.empty() && !table->IsMultiColumn(cell)) {
            fl_deactivate_object(fd_form_table_options->radio_align_left);
            fl_deactivate_object(fd_form_table_options->radio_align_right);
            fl_deactivate_object(fd_form_table_options->radio_align_center);
	    fl_set_object_lcol(fd_form_table_options->radio_align_left,
			       FL_INACTIVE);
	    fl_set_object_lcol(fd_form_table_options->radio_align_right,
			       FL_INACTIVE);
	    fl_set_object_lcol(fd_form_table_options->radio_align_center,
			       FL_INACTIVE);
        } else {
            fl_activate_object(fd_form_table_options->radio_align_left);
            fl_activate_object(fd_form_table_options->radio_align_right);
            fl_activate_object(fd_form_table_options->radio_align_center);
	    fl_set_object_lcol(fd_form_table_options->radio_align_left,
			       FL_BLACK);
	    fl_set_object_lcol(fd_form_table_options->radio_align_right,
			       FL_BLACK);
	    fl_set_object_lcol(fd_form_table_options->radio_align_center,
			       FL_BLACK);
        }
        fl_set_button(fd_form_table_options->radio_longtable, table->IsLongTabular());
        if (table->IsLongTabular()) {
            fl_activate_object(fd_form_table_options->radio_lt_firsthead);
            fl_activate_object(fd_form_table_options->radio_lt_head);
            fl_activate_object(fd_form_table_options->radio_lt_foot);
            fl_activate_object(fd_form_table_options->radio_lt_lastfoot);
            fl_activate_object(fd_form_table_options->radio_lt_newpage);
	    fl_set_object_lcol(fd_form_table_options->radio_lt_firsthead,
			       FL_BLACK);
	    fl_set_object_lcol(fd_form_table_options->radio_lt_head,
			       FL_BLACK);
	    fl_set_object_lcol(fd_form_table_options->radio_lt_foot,
			       FL_BLACK);
	    fl_set_object_lcol(fd_form_table_options->radio_lt_lastfoot,
			       FL_BLACK);
	    fl_set_object_lcol(fd_form_table_options->radio_lt_newpage,
			       FL_BLACK);
            fl_set_button(fd_form_table_options->radio_lt_firsthead,
                          table->GetRowOfLTFirstHead(cell));
            fl_set_button(fd_form_table_options->radio_lt_head,
                          table->GetRowOfLTHead(cell));
            fl_set_button(fd_form_table_options->radio_lt_foot,
                          table->GetRowOfLTFoot(cell));
            fl_set_button(fd_form_table_options->radio_lt_lastfoot,
                          table->GetRowOfLTLastFoot(cell));
            fl_set_button(fd_form_table_options->radio_lt_newpage,
                          table->GetLTNewPage(cell));
        } else {
            fl_deactivate_object(fd_form_table_options->radio_lt_firsthead);
            fl_deactivate_object(fd_form_table_options->radio_lt_head);
            fl_deactivate_object(fd_form_table_options->radio_lt_foot);
            fl_deactivate_object(fd_form_table_options->radio_lt_lastfoot);
            fl_deactivate_object(fd_form_table_options->radio_lt_newpage);
            fl_set_button(fd_form_table_options->radio_lt_firsthead, 0);
            fl_set_button(fd_form_table_options->radio_lt_head, 0);
            fl_set_button(fd_form_table_options->radio_lt_foot, 0);
            fl_set_button(fd_form_table_options->radio_lt_lastfoot, 0);
            fl_set_button(fd_form_table_options->radio_lt_newpage, 0);
	    fl_set_object_lcol(fd_form_table_options->radio_lt_firsthead,
			       FL_INACTIVE);
	    fl_set_object_lcol(fd_form_table_options->radio_lt_head,
			       FL_INACTIVE);
	    fl_set_object_lcol(fd_form_table_options->radio_lt_foot,
			       FL_INACTIVE);
	    fl_set_object_lcol(fd_form_table_options->radio_lt_lastfoot,
			       FL_INACTIVE);
	    fl_set_object_lcol(fd_form_table_options->radio_lt_newpage,
			       FL_INACTIVE);
        }
        fl_set_button(fd_form_table_options->radio_rotate_table,
		      table->GetRotateTabular());
	fl_hide_object(fd_form_table_options->button_table_delete);
	fl_set_focus_object(fd_form_table_options->form_table_options,
			    fd_form_table_options->button_table_delete);
    } else if (fd_form_table_options->form_table_options->visible) {
	fl_set_focus_object(fd_form_table_options->form_table_options,
			    fd_form_table_options->button_table_delete);
        fl_hide_form(fd_form_table_options->form_table_options);
    }
    return update;
}


void TabularOptionsCB(FL_OBJECT * ob, long)
{
    if (!inset)
	return;

    int s, num = 0;
    string special, str;

    LyXTabular * table = inset->tabular;
    
    int cell = inset->GetActCell();
    if (ActCell != cell) {
        MenuLayoutTabular(false, inset);
        fl_set_object_label(fd_form_table_options->text_warning,
                          _("Warning: Wrong Cursor position, updated window"));
        fl_show_object(fd_form_table_options->text_warning);
	extra_col_cursor_x = 0; // would rather place it at the end, but...
	extra_multicol_cursor_x = 0;
        return;
    }

    // No point in processing directives that you can't do anything with
    // anyhow, so exit now if the buffer is read-only.
    if (current_view->buffer()->isReadonly()) {
      MenuLayoutTabular(false, inset);
      return;
    }
    
    if (ob != fd_form_table_options->button_table_delete) {
        fl_set_object_label(fd_form_table_options->text_warning, "");
        Confirmed = false;
    }
    str = fl_get_input(fd_form_table_options->input_column_width);
    if (!str.empty() && !isValidLength(str)) {
        fl_set_object_label(fd_form_table_options->text_warning,
                            _("Warning: Invalid Length (valid example: 10mm)"));
        fl_show_object(fd_form_table_options->text_warning);
        return;
    }
    if (((ob==fd_form_table_options->button_delete_row)&&(table->rows()<=1)) ||
        ((ob==fd_form_table_options->button_delete_column)&&(table->columns()<=1)))
        ob = fd_form_table_options->button_table_delete;
    if (ob == fd_form_table_options->button_append_row)
        num = LyXTabular::APPEND_ROW;
    else if (ob == fd_form_table_options->button_append_column)
        num = LyXTabular::APPEND_COLUMN;
    else if (ob == fd_form_table_options->button_delete_row)
        num = LyXTabular::DELETE_ROW;
    else if (ob == fd_form_table_options->button_delete_column)
        num = LyXTabular::DELETE_COLUMN;
    else if (ob == fd_form_table_options->button_set_borders)
        num = LyXTabular::SET_ALL_LINES;
    else if (ob == fd_form_table_options->button_unset_borders)
        num = LyXTabular::UNSET_ALL_LINES;
    else if (ob == fd_form_table_options->radio_border_top)
        num = LyXTabular::TOGGLE_LINE_TOP;
    else if (ob == fd_form_table_options->radio_border_bottom)
        num = LyXTabular::TOGGLE_LINE_BOTTOM;
    else if (ob == fd_form_table_options->radio_border_left)
        num = LyXTabular::TOGGLE_LINE_LEFT;
    else if (ob == fd_form_table_options->radio_border_right)
        num = LyXTabular::TOGGLE_LINE_RIGHT;
    else if (ob == fd_form_table_options->radio_align_left)
        num = LyXTabular::ALIGN_LEFT;
    else if (ob == fd_form_table_options->radio_align_right)
        num = LyXTabular::ALIGN_RIGHT;
    else if (ob == fd_form_table_options->radio_align_center)
        num = LyXTabular::ALIGN_CENTER;
#if 0
    // not needed in tabulars as you can delete them with a single delete!
    else if ((ob == fd_form_table_options->button_table_delete) && !Confirmed) {
        fl_set_object_label(fd_form_table_options->text_warning,
                            _("Confirm: press Delete-Button again"));
        Confirmed = true;
        return;
    }
#endif
    else if ((ob == fd_form_table_options->button_table_delete) 
	       && Confirmed) {
        num = LyXTabular::DELETE_TABULAR;
        Confirmed = false;
    } else if (ob == fd_form_table_options->radio_multicolumn)
        num = LyXTabular::MULTICOLUMN;
    else if (ob == fd_form_table_options->radio_longtable) {
        s = fl_get_button(fd_form_table_options->radio_longtable);
        if (s) {
            num = LyXTabular::SET_LONGTABULAR;
            fl_activate_object(fd_form_table_options->radio_lt_firsthead);
            fl_activate_object(fd_form_table_options->radio_lt_head);
            fl_activate_object(fd_form_table_options->radio_lt_foot);
            fl_activate_object(fd_form_table_options->radio_lt_lastfoot);
            fl_activate_object(fd_form_table_options->radio_lt_newpage);
            fl_set_button(fd_form_table_options->radio_lt_firsthead,
                          table->GetRowOfLTFirstHead(cell));
            fl_set_button(fd_form_table_options->radio_lt_head,
                          table->GetRowOfLTHead(cell));
            fl_set_button(fd_form_table_options->radio_lt_foot,
                          table->GetRowOfLTFoot(cell));
            fl_set_button(fd_form_table_options->radio_lt_lastfoot,
                          table->GetRowOfLTLastFoot(cell));
            fl_set_button(fd_form_table_options->radio_lt_firsthead,
                          table->GetLTNewPage(cell));
        } else {
	    num = LyXTabular::UNSET_LONGTABULAR;
            fl_deactivate_object(fd_form_table_options->radio_lt_firsthead);
            fl_deactivate_object(fd_form_table_options->radio_lt_head);
            fl_deactivate_object(fd_form_table_options->radio_lt_foot);
            fl_deactivate_object(fd_form_table_options->radio_lt_lastfoot);
            fl_deactivate_object(fd_form_table_options->radio_lt_newpage);
            fl_set_button(fd_form_table_options->radio_lt_firsthead, 0);
            fl_set_button(fd_form_table_options->radio_lt_firsthead, 0);
            fl_set_button(fd_form_table_options->radio_lt_firsthead, 0);
            fl_set_button(fd_form_table_options->radio_lt_firsthead, 0);
            fl_set_button(fd_form_table_options->radio_lt_firsthead, 0);
	    fl_set_object_lcol(fd_form_table_options->radio_lt_firsthead,
			       FL_INACTIVE);
	    fl_set_object_lcol(fd_form_table_options->radio_lt_head,
			       FL_INACTIVE);
	    fl_set_object_lcol(fd_form_table_options->radio_lt_foot,
			       FL_INACTIVE);
	    fl_set_object_lcol(fd_form_table_options->radio_lt_lastfoot,
			       FL_INACTIVE);
	    fl_set_object_lcol(fd_form_table_options->radio_lt_newpage,
			       FL_INACTIVE);
        }
    } else if (ob == fd_form_table_options->radio_rotate_table) {
        s = fl_get_button(fd_form_table_options->radio_rotate_table);
	if (s)
            num = LyXTabular::SET_ROTATE_TABULAR;
	else
	    num = LyXTabular::UNSET_ROTATE_TABULAR;
    } else if (ob == fd_form_table_options->radio_rotate_cell) {
        s = fl_get_button(fd_form_table_options->radio_rotate_cell);
	if (s)
            num = LyXTabular::SET_ROTATE_CELL;
	else
	    num = LyXTabular::UNSET_ROTATE_CELL;
    } else if (ob == fd_form_table_options->radio_linebreak_cell) {
        num = LyXTabular::SET_LINEBREAKS;
    } else if (ob == fd_form_table_options->radio_lt_firsthead) {
        num = LyXTabular::SET_LTFIRSTHEAD;
    } else if (ob == fd_form_table_options->radio_lt_head) {
        num = LyXTabular::SET_LTHEAD;
    } else if (ob == fd_form_table_options->radio_lt_foot) {
        num = LyXTabular::SET_LTFOOT;
    } else if (ob == fd_form_table_options->radio_lt_lastfoot) {
        num = LyXTabular::SET_LTLASTFOOT;
    } else if (ob == fd_form_table_options->radio_lt_newpage) {
        num = LyXTabular::SET_LTNEWPAGE;
    } else if (ob == fd_form_table_options->button_table_extra) {
        OpenLayoutTableExtra();
        return;
    } else if (ob == fd_form_table_extra->input_special_alignment) {
        special = fl_get_input(fd_form_table_extra->input_special_alignment);
	int dummy;
	fl_get_input_cursorpos(ob, &extra_col_cursor_x, &dummy);
        num = LyXTabular::SET_SPECIAL_COLUMN;
    } else if (ob == fd_form_table_extra->input_special_multialign) {
        special = fl_get_input(fd_form_table_extra->input_special_multialign);
	int dummy;
	fl_get_input_cursorpos(ob, &extra_multicol_cursor_x, &dummy);
        num = LyXTabular::SET_SPECIAL_MULTI;
    } else
        return;
    if (current_view->available()) {
	current_view->hideCursor();
	inset->TabularFeatures(current_view, num, special);
    }
    UpdateLayoutTabular(true, inset);
    return;
}

void SetPWidthTabularCB(FL_OBJECT * ob, long)
{
    if (!inset) {
	return;
    }

    fl_set_object_label(fd_form_table_options->text_warning, "");
    Confirmed = false;
    if (ob == fd_form_table_options->input_column_width) {
        string str = fl_get_input(ob);
        if (!str.empty() && !isValidLength(str)) {
            fl_set_object_label(fd_form_table_options->text_warning,
                           _("Warning: Invalid Length (valid example: 10mm)"));
            fl_show_object(fd_form_table_options->text_warning);
            return;
        }
        if (current_view->available()){
            current_view->hideCursor();
            inset->TabularFeatures(current_view, LyXTabular::SET_PWIDTH, str);
        }
        MenuLayoutTabular(false, inset); // update for alignment
    }
}

void TabularOptClose()
{
    inset = 0;
}

