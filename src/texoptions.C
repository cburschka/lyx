/* This file is part of
* ====================================================== 
* 
*           LyX, The Document Processor
*
*           Copyright 1995 Matthias Ettrich
*           Copyright 1995-2000 The LyX Team.
*
*           This file is Copyleft 1996-2000
*           Lars Gullik Bjønnes
*
* ====================================================== */

#include <config.h>
#include FORMS_H_LOCATION
#include "latexoptions.h"
#include "lyx_main.h"
#include "buffer.h"
#include "BufferView.h"

extern FD_LaTeXOptions * fd_latex_options;
extern BufferView * current_view;

void LaTeXOptionsOK(FL_OBJECT * ob, long data)
{
	LaTeXOptionsApply(ob, data);
	LaTeXOptionsCancel(ob, data);
}

void LaTeXOptionsApply(FL_OBJECT *, long)
{
	if (fl_get_button(fd_latex_options->accents))
		current_view->buffer()->params.allowAccents = true;
	else
		current_view->buffer()->params.allowAccents = false;
}

void LaTeXOptionsCancel(FL_OBJECT *, long)
{
	fl_hide_form(fd_latex_options->LaTeXOptions);
}
