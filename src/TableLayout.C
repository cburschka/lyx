#include <config.h>

#include <cstdlib>
#include "definitions.h"
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
#include "lyxscreen.h"
#include "gettext.h"
#include "lyxtext.h"

extern void BeforeChange();

/* Prototypes */
extern FD_form_table_options *fd_form_table_options;
extern FD_form_table_extra *fd_form_table_extra;
extern BufferView *current_view;

static int
    Confirmed = false,
    ActCell;
// hack to keep the cursor from jumping to the end of the text in the Extra
// form input fields during editing. The values in LyXTable itself is changed in
// real-time, but we have no callbacks for the input fields, so I simply
// store and restore the cursor position for now. (too much of a hazzle to
// do it proper; we'll trash all this code in 1.1 anyway)
    static int
	extra_col_cursor_x, // need no y's, one-line input fields
	extra_multicol_cursor_x;
// Joacim

bool UpdateLayoutTable(int flag)
{
    bool update = true;
    if (!current_view->getScreen() || !current_view->available())
        update = false;
    
    if (update && current_view->currentBuffer()->text->cursor.par->table) {
        int
            align,
            cell,
            column,row;
        char
            buf[12];
        string
            pwidth, special;
    
        LyXTable *table = current_view->currentBuffer()->text->cursor.par->table;

        cell = current_view->currentBuffer()->text->
            NumberOfCell(current_view->currentBuffer()->text->cursor.par, 
                         current_view->currentBuffer()->text->cursor.pos);
        ActCell = cell;
        column = table->column_of_cell(cell)+1;
        fl_set_object_label(fd_form_table_options->text_warning,"");
        Confirmed = false;
        fl_activate_object(fd_form_table_extra->input_special_alignment);
        fl_activate_object(fd_form_table_extra->input_special_multialign);
        fl_activate_object(fd_form_table_options->input_column_width);
        sprintf(buf,"%d",column);
        fl_set_input(fd_form_table_options->input_table_column, buf);
        fl_deactivate_object(fd_form_table_options->input_table_column);
        row = table->row_of_cell(cell)+1;
        sprintf(buf,"%d",row);
        fl_set_input(fd_form_table_options->input_table_row, buf);
        fl_deactivate_object(fd_form_table_options->input_table_row);
        if (table->IsMultiColumn(cell))
            fl_set_button(fd_form_table_options->radio_multicolumn, 1);
        else
            fl_set_button(fd_form_table_options->radio_multicolumn, 0);
        if (table->RotateCell(cell))
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
        align = table->GetAlignment(cell);
        fl_set_button(fd_form_table_options->radio_align_left, 0);
        fl_set_button(fd_form_table_options->radio_align_right, 0);
        fl_set_button(fd_form_table_options->radio_align_center, 0);
        special = table->GetAlignSpecial(cell,LyXTable::SET_SPECIAL_COLUMN);
        if (flag)
	{
            fl_set_input(fd_form_table_extra->input_special_alignment,
                         special.c_str());
	    fl_set_input_cursorpos(fd_form_table_extra->input_special_alignment,
		    extra_col_cursor_x, 0); // restore the cursor
	}
        if (current_view->currentBuffer()->isReadonly()) 
            fl_deactivate_object(fd_form_table_extra->input_special_alignment);
        special = table->GetAlignSpecial(cell,LyXTable::SET_SPECIAL_MULTI);
        if (flag)
	{
            fl_set_input(fd_form_table_extra->input_special_multialign,
                         special.c_str());
	    fl_set_input_cursorpos(fd_form_table_extra->input_special_multialign,
		    extra_multicol_cursor_x, 0); // restore the cursor
	}
        if (current_view->currentBuffer()->isReadonly()) 
            fl_deactivate_object(fd_form_table_extra->input_special_multialign);
        pwidth = table->GetPWidth(cell);
        if (flag)
            fl_set_input(fd_form_table_options->input_column_width,pwidth.c_str());
        if (current_view->currentBuffer()->isReadonly()) 
            fl_deactivate_object(fd_form_table_options->input_column_width);
        if (!pwidth.empty()) {
            fl_activate_object(fd_form_table_options->radio_linebreak_cell);
	    fl_set_object_lcol(fd_form_table_options->radio_linebreak_cell,
			       FL_BLACK);
            fl_set_button(fd_form_table_options->radio_linebreak_cell,
                          table->Linebreaks(table->FirstVirtualCell(cell)));
        } else {
            fl_deactivate_object(fd_form_table_options->radio_linebreak_cell);
	    fl_set_object_lcol(fd_form_table_options->radio_linebreak_cell,
			       FL_INACTIVE);
            fl_set_button(fd_form_table_options->radio_linebreak_cell,0);
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
        fl_set_button(fd_form_table_options->radio_longtable,table->IsLongTable());
        if (table->IsLongTable()) {
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
                          table->RowOfLTFirstHead(cell));
            fl_set_button(fd_form_table_options->radio_lt_head,
                          table->RowOfLTHead(cell));
            fl_set_button(fd_form_table_options->radio_lt_foot,
                          table->RowOfLTFoot(cell));
            fl_set_button(fd_form_table_options->radio_lt_lastfoot,
                          table->RowOfLTLastFoot(cell));
            fl_set_button(fd_form_table_options->radio_lt_newpage,
                          table->LTNewPage(cell));
        } else {
            fl_deactivate_object(fd_form_table_options->radio_lt_firsthead);
            fl_deactivate_object(fd_form_table_options->radio_lt_head);
            fl_deactivate_object(fd_form_table_options->radio_lt_foot);
            fl_deactivate_object(fd_form_table_options->radio_lt_lastfoot);
            fl_deactivate_object(fd_form_table_options->radio_lt_newpage);
            fl_set_button(fd_form_table_options->radio_lt_firsthead,0);
            fl_set_button(fd_form_table_options->radio_lt_head,0);
            fl_set_button(fd_form_table_options->radio_lt_foot,0);
            fl_set_button(fd_form_table_options->radio_lt_lastfoot,0);
            fl_set_button(fd_form_table_options->radio_lt_newpage,0);
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
		      table->RotateTable());
	fl_set_focus_object(fd_form_table_options->form_table_options,
			    fd_form_table_options->button_table_delete);
    } else if (fd_form_table_options->form_table_options->visible) {
	fl_set_focus_object(fd_form_table_options->form_table_options,
			    fd_form_table_options->button_table_delete);
        fl_hide_form(fd_form_table_options->form_table_options);
    }
    return update;
}

void OpenLayoutTableExtra()
{
    static int ow = -1, oh;

    if (fd_form_table_extra->form_table_extra->visible) {
        fl_raise_form(fd_form_table_extra->form_table_extra);
    } else {
        fl_show_form(fd_form_table_extra->form_table_extra,
                     FL_PLACE_MOUSE | FL_FREE_SIZE,FL_FULLBORDER,
                     _("Table Extra Form"));
		if (ow < 0) {
			ow = fd_form_table_extra->form_table_extra->w;
			oh = fd_form_table_extra->form_table_extra->h;
		}
		fl_set_form_minsize(fd_form_table_extra->form_table_extra,
				    ow,oh);
    }
}

void MenuLayoutTable(int flag)
{
    if (UpdateLayoutTable(flag)) {
        if (fd_form_table_options->form_table_options->visible) {
            fl_raise_form(fd_form_table_options->form_table_options);
        }
        else {
            fl_show_form(fd_form_table_options->form_table_options,
                         FL_PLACE_MOUSE, FL_FULLBORDER,
                         _("Table Layout"));
        }
    }
}

void TableOptionsCB(FL_OBJECT *ob, long)
{
    LyXTable
        *table = 0;
    int
        cell,
        s,
        num = 0;
    string
        special,
        str;

    if (!current_view->available()
	||
	!(table = current_view->currentBuffer()->text->cursor.par->table)) 
      {
        MenuLayoutTable(0);
	return;
      }
    cell = current_view->currentBuffer()->text->
        NumberOfCell(current_view->currentBuffer()->text->cursor.par, 
                     current_view->currentBuffer()->text->cursor.pos);
    if (ActCell != cell) {
        MenuLayoutTable(0);
        fl_set_object_label(fd_form_table_options->text_warning,
                            _("Warning: Wrong Cursor position, updated window"));
        fl_show_object(fd_form_table_options->text_warning);
	extra_col_cursor_x=0; // would rather place it at the end, but...
	extra_multicol_cursor_x=0;
        return;
    }
    // No point in processing directives that you can't do anything with
    // anyhow, so exit now if the buffer is read-only.
    if (current_view->currentBuffer()->isReadonly()) {
      MenuLayoutTable(0);
      return;
    }
    
    if (ob != fd_form_table_options->button_table_delete) {
        fl_set_object_label(fd_form_table_options->text_warning,"");
        Confirmed = false;
    }
    str = fl_get_input(fd_form_table_options->input_column_width);
    if (!str.empty() && !isValidLength(str)) {
        fl_set_object_label(fd_form_table_options->text_warning,
                            _("Warning: Invalid Length (valid example: 10mm)"));
        fl_show_object(fd_form_table_options->text_warning);
        return;
    }
    if (((ob==fd_form_table_options->button_delete_row) && (table->rows<=1)) ||
        ((ob==fd_form_table_options->button_delete_column) && (table->columns<=1)))
        ob = fd_form_table_options->button_table_delete;
    if (ob == fd_form_table_options->button_append_row)
        num = LyXTable::APPEND_ROW;
    else if (ob == fd_form_table_options->button_append_column)
        num = LyXTable::APPEND_COLUMN;
    else if (ob == fd_form_table_options->button_delete_row)
        num = LyXTable::DELETE_ROW;
    else if (ob == fd_form_table_options->button_delete_column)
        num = LyXTable::DELETE_COLUMN;
    else if (ob == fd_form_table_options->button_set_borders)
        num = LyXTable::SET_ALL_LINES;
    else if (ob == fd_form_table_options->button_unset_borders)
        num = LyXTable::UNSET_ALL_LINES;
    else if (ob == fd_form_table_options->radio_border_top)
        num = LyXTable::TOGGLE_LINE_TOP;
    else if (ob == fd_form_table_options->radio_border_bottom)
        num = LyXTable::TOGGLE_LINE_BOTTOM;
    else if (ob == fd_form_table_options->radio_border_left)
        num = LyXTable::TOGGLE_LINE_LEFT;
    else if (ob == fd_form_table_options->radio_border_right)
        num = LyXTable::TOGGLE_LINE_RIGHT;
    else if (ob == fd_form_table_options->radio_align_left)
        num = LyXTable::ALIGN_LEFT;
    else if (ob == fd_form_table_options->radio_align_right)
        num = LyXTable::ALIGN_RIGHT;
    else if (ob == fd_form_table_options->radio_align_center)
        num = LyXTable::ALIGN_CENTER;
    else if ((ob==fd_form_table_options->button_table_delete) && !Confirmed) {
        fl_set_object_label(fd_form_table_options->text_warning,
                            _("Confirm: press Delete-Button again"));
        Confirmed = true;
        return;
    } else if ((ob == fd_form_table_options->button_table_delete) 
	       && Confirmed) {
        num = LyXTable::DELETE_TABLE;
        Confirmed = false;
    } else if (ob == fd_form_table_options->radio_multicolumn)
        num = LyXTable::MULTICOLUMN;
    else if (ob == fd_form_table_options->radio_longtable) {
        s=fl_get_button(fd_form_table_options->radio_longtable);
        if (s) {
            num = LyXTable::SET_LONGTABLE;
            fl_activate_object(fd_form_table_options->radio_lt_firsthead);
            fl_activate_object(fd_form_table_options->radio_lt_head);
            fl_activate_object(fd_form_table_options->radio_lt_foot);
            fl_activate_object(fd_form_table_options->radio_lt_lastfoot);
            fl_activate_object(fd_form_table_options->radio_lt_newpage);
            fl_set_button(fd_form_table_options->radio_lt_firsthead,
                          table->RowOfLTFirstHead(cell));
            fl_set_button(fd_form_table_options->radio_lt_head,
                          table->RowOfLTHead(cell));
            fl_set_button(fd_form_table_options->radio_lt_foot,
                          table->RowOfLTFoot(cell));
            fl_set_button(fd_form_table_options->radio_lt_lastfoot,
                          table->RowOfLTLastFoot(cell));
            fl_set_button(fd_form_table_options->radio_lt_firsthead,
                          table->LTNewPage(cell));
        } else {
	    num = LyXTable::UNSET_LONGTABLE;
            fl_deactivate_object(fd_form_table_options->radio_lt_firsthead);
            fl_deactivate_object(fd_form_table_options->radio_lt_head);
            fl_deactivate_object(fd_form_table_options->radio_lt_foot);
            fl_deactivate_object(fd_form_table_options->radio_lt_lastfoot);
            fl_deactivate_object(fd_form_table_options->radio_lt_newpage);
            fl_set_button(fd_form_table_options->radio_lt_firsthead,0);
            fl_set_button(fd_form_table_options->radio_lt_firsthead,0);
            fl_set_button(fd_form_table_options->radio_lt_firsthead,0);
            fl_set_button(fd_form_table_options->radio_lt_firsthead,0);
            fl_set_button(fd_form_table_options->radio_lt_firsthead,0);
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
        s=fl_get_button(fd_form_table_options->radio_rotate_table);
	if (s)
            num = LyXTable::SET_ROTATE_TABLE;
	else
	    num = LyXTable::UNSET_ROTATE_TABLE;
    } else if (ob == fd_form_table_options->radio_rotate_cell) {
        s=fl_get_button(fd_form_table_options->radio_rotate_cell);
	if (s)
            num = LyXTable::SET_ROTATE_CELL;
	else
	    num = LyXTable::UNSET_ROTATE_CELL;
    } else if (ob == fd_form_table_options->radio_linebreak_cell) {
        num = LyXTable::SET_LINEBREAKS;
    } else if (ob == fd_form_table_options->radio_lt_firsthead) {
        num = LyXTable::SET_LTFIRSTHEAD;
    } else if (ob == fd_form_table_options->radio_lt_head) {
        num = LyXTable::SET_LTHEAD;
    } else if (ob == fd_form_table_options->radio_lt_foot) {
        num = LyXTable::SET_LTFOOT;
    } else if (ob == fd_form_table_options->radio_lt_lastfoot) {
        num = LyXTable::SET_LTLASTFOOT;
    } else if (ob == fd_form_table_options->radio_lt_newpage) {
        num = LyXTable::SET_LTNEWPAGE;
    } else if (ob == fd_form_table_options->button_table_extra) {
        OpenLayoutTableExtra();
        return;
    } else if (ob == fd_form_table_extra->input_special_alignment) {
        special=fl_get_input(fd_form_table_extra->input_special_alignment);
	int dummy;
	fl_get_input_cursorpos(ob, &extra_col_cursor_x, &dummy);
        num = LyXTable::SET_SPECIAL_COLUMN;
    } else if (ob == fd_form_table_extra->input_special_multialign) {
        special=fl_get_input(fd_form_table_extra->input_special_multialign);
	int dummy;
	fl_get_input_cursorpos(ob, &extra_multicol_cursor_x, &dummy);
        num = LyXTable::SET_SPECIAL_MULTI;
    } else
        return;
    if (current_view->available()){
        current_view->getScreen()->HideCursor();
        if (!current_view->currentBuffer()->text->selection){
            BeforeChange(); 
            current_view->currentBuffer()->update(-2);
        }
        if ((num == LyXTable::SET_SPECIAL_COLUMN) ||
            (num == LyXTable::SET_SPECIAL_MULTI))
            current_view->currentBuffer()->text->TableFeatures(num,special);
        else
            current_view->currentBuffer()->text->TableFeatures(num);
        current_view->currentBuffer()->update(1);
    }
    if (num == LyXTable::DELETE_TABLE) {
	fl_set_focus_object(fd_form_table_options->form_table_options,
			    fd_form_table_options->button_table_delete);
        fl_hide_form(fd_form_table_options->form_table_options);
    } else
        UpdateLayoutTable(true);
    return;
}

void TableOptCloseCB(FL_OBJECT *, long)
{
    fl_set_focus_object(fd_form_table_options->form_table_options,
			fd_form_table_options->button_table_delete);
    fl_hide_form(fd_form_table_options->form_table_options);
    return;
}

void TableSpeCloseCB(FL_OBJECT *, long)
{
    fl_set_focus_object(fd_form_table_options->form_table_options,
			fd_form_table_options->button_table_delete);
    fl_hide_form(fd_form_table_extra->form_table_extra);
    return;
}

void SetPWidthCB(FL_OBJECT *ob, long)
{
    fl_set_object_label(fd_form_table_options->text_warning,"");
    Confirmed = false;
    if (ob == fd_form_table_options->input_column_width) {
        string
            str;
        str = fl_get_input(ob);
        if (!str.empty() && !isValidLength(str)) {
            fl_set_object_label(fd_form_table_options->text_warning,
                            _("Warning: Invalid Length (valid example: 10mm)"));
            fl_show_object(fd_form_table_options->text_warning);
            return;
        }
        if (current_view->available()){
            current_view->getScreen()->HideCursor();
            if (!current_view->currentBuffer()->text->selection){
                BeforeChange(); 
                current_view->currentBuffer()->update(-2);
            }
            current_view->currentBuffer()->text->TableFeatures(LyXTable::SET_PWIDTH,str);
            current_view->currentBuffer()->update(1);
        }
        MenuLayoutTable(0); // update for alignment
    }
}
