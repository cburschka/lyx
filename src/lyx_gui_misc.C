/* This file is part of
 * ====================================================== 
 * 
 *           LyX, The Document Processor
 *        
 *           Copyright 1995 Matthias Ettrich
 *           Copyright 1995-2001 The LyX Team.
 *
 * ====================================================== */

#include <config.h>

#ifdef __GNUG__
#pragma implementation
#endif

#include FORMS_H_LOCATION
 
#include "lyx_gui_misc.h"
#include "print_form.h"

extern FD_form_sendto * fd_form_sendto;

// The code below is just waiting to go away really ...
// I think it is Juergen's responsibility to port print_form,
// since he wants it :) 
 
extern "C"
int CancelCloseBoxCB(FL_FORM *, void *)
{
	return FL_CANCEL;
}

void RedrawAllBufferRelatedDialogs()
{
	if (fd_form_sendto->form_sendto->visible) {
		fl_redraw_form(fd_form_sendto->form_sendto);
	}
}

void CloseAllBufferRelatedDialogs()
{
	if (fd_form_sendto->form_sendto->visible) {
		fl_hide_form(fd_form_sendto->form_sendto);
	}
}
