/* This file is part of
 * ====================================================== 
 * 
 *           LyX, The Document Processor
 * 	 
 *	    Copyright 1995 Matthias Ettrich,
 *          Copyright 1995-2001 The LyX Team.
 *
 * ====================================================== */

#include <config.h>

#include <fstream>
#include <algorithm>
#include <utility> 
#include <iostream>

#include FORMS_H_LOCATION

#include "figure_form.h"
#include "insets/figinset.h"
#include "BufferView.h"
#include "lyxtext.h"
#include "LyXView.h"
#include "undo_funcs.h"
#include "gettext.h"

extern FD_form_figure * fd_form_figure;
extern BufferView * current_view;

void Figure()
{
	if (fd_form_figure->form_figure->visible) {
		fl_raise_form(fd_form_figure->form_figure);
	} else {
		fl_show_form(fd_form_figure->form_figure,
			     FL_PLACE_MOUSE | FL_FREE_SIZE, FL_TRANSIENT,
			     _("Insert Figure"));
	}
}


/* callbacks for form form_figure */
extern "C" {
	
void FigureApplyCB(FL_OBJECT *, long)
{
	if (!current_view->available())
		return;

	Buffer * buffer = current_view->buffer();
	if (buffer->isReadonly()) // paranoia
		return;

	current_view->owner()->message(_("Inserting figure..."));
	if (fl_get_button(fd_form_figure->radio_inline)) {
		InsetFig * new_inset = new InsetFig(100, 20, *buffer);
		current_view->insertInset(new_inset);
		current_view->owner()->message(_("Figure inserted"));
		new_inset->edit(current_view, 0, 0, 0);
		return;
	}
	
	current_view->hideCursor();
	current_view->update(current_view->text, BufferView::SELECT|BufferView::FITCUR);
	current_view->beforeChange(current_view->text);
      
	setCursorParUndo(current_view); 
	freezeUndo();

	current_view->text->breakParagraph(current_view);
	current_view->update(current_view->text, BufferView::SELECT|BufferView::FITCUR|BufferView::CHANGE);

	if (current_view->text->cursor.par()->size()) {
		current_view->text->cursorLeft(current_view);
	 
		current_view->text->breakParagraph(current_view);
		current_view->update(current_view->text, BufferView::SELECT|BufferView::FITCUR|BufferView::CHANGE);
	}

	// The standard layout should always be numer 0;
	current_view->text->setLayout(current_view, 0);

	LyXLength len(0.3 * buffer->params.spacing.getValue(), LyXLength::CM);
	current_view->text->setParagraph(current_view, 0, 0, 0, 0,
		VSpace(len), VSpace(len), Spacing(),
		LYX_ALIGN_CENTER, string(), 0);
	
	current_view->update(current_view->text, BufferView::SELECT|BufferView::FITCUR|BufferView::CHANGE);
      
	Inset * new_inset = new InsetFig(100, 100, *buffer);
	current_view->insertInset(new_inset);
	new_inset->edit(current_view, 0, 0, 0);
	current_view->update(current_view->text, BufferView::SELECT|BufferView::FITCUR);
	current_view->owner()->message(_("Figure inserted"));
	unFreezeUndo();
	current_view->setState();
}


void FigureCancelCB(FL_OBJECT *, long)
{
	fl_hide_form(fd_form_figure->form_figure);
}


void FigureOKCB(FL_OBJECT * ob, long data)
{
	FigureApplyCB(ob, data);
	FigureCancelCB(ob, data);
}

}

