/* This file is part of
* ======================================================
* 
*           LyX, The Document Processor
*
*           Copyright (C) 1995 Matthias Ettrich
*           Copyright (C) 1995-1998 The LyX Team.
*
*           This file is Copyleft (C) 1996-1998
*           Lars Gullik Bjønnes
*
*======================================================*/

#include <config.h>
#include FORMS_H_LOCATION
#include "latexoptions.h"
#include "lyx_cb.h"
#include "lyx_main.h"
#include "buffer.h"
#include "BufferView.h"

// 	$Id: texoptions.C,v 1.1 1999/09/27 18:44:38 larsbj Exp $	

#if !defined(lint) && !defined(WITH_WARNINGS)
static char vcid[] = "$Id: texoptions.C,v 1.1 1999/09/27 18:44:38 larsbj Exp $";
#endif /* lint */

extern FD_LaTeXOptions *fd_latex_options;
extern BufferView *current_view;

void LaTeXOptionsOK(FL_OBJECT *ob, long data)
{
	LaTeXOptionsApply(ob, data);
	LaTeXOptionsCancel(ob, data);
}

void LaTeXOptionsApply(FL_OBJECT *, long)
{
	if (fl_get_button(fd_latex_options->accents))
		current_view->currentBuffer()->params.allowAccents = true;
	else
		current_view->currentBuffer()->params.allowAccents = false;
}

void LaTeXOptionsCancel(FL_OBJECT *, long)
{
	fl_hide_form(fd_latex_options->LaTeXOptions);
}
