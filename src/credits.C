/* This file is part of
* ======================================================
* 
*           LyX, The Document Processor
* 	 
*	    Copyright (C) 1995 Matthias Ettrich
*           Copyright (C) 1995-1998 The LyX Team.
*
*======================================================*/

#include <config.h>

#include FORMS_H_LOCATION
#include <stdlib.h>

#include "credits.h"
#include "credits_form.h"

#include "LString.h"
#include "filetools.h"
#include "lyx_gui_misc.h" // CancelCloseBoxCB
#include "gettext.h"

// 	$Id: credits.C,v 1.1 1999/09/27 18:44:37 larsbj Exp $	

#if !defined(lint) && !defined(WITH_WARNINGS)
static char vcid[] = "$Id: credits.C,v 1.1 1999/09/27 18:44:37 larsbj Exp $";
#endif /* lint */

/* extern */
extern LString system_lyxdir;

/**** Forms and Objects ****/

static FD_form_credits *fd_form_credits = NULL;
static FD_copyright *fd_form_copyright = NULL;

void CreditsOKCB(FL_OBJECT *, long) {
      fl_hide_form(fd_form_credits->form_credits);
      fl_free_form(fd_form_credits->form_credits);
      fd_form_credits->form_credits = NULL;
}

/*---------------------------------------*/
/* read credits from file and display them */
void ShowCredits()
{
	/* generate the credits form if it doesn't exist */
	if (!fd_form_credits || !fd_form_credits->form_credits) {
		
		/* read the credits into the browser */ 
		
		/* try file LYX_DIR/CREDITS */ 
		LString real_file = AddName (system_lyxdir, "CREDITS");
		fd_form_credits = create_form_form_credits();
		fl_set_form_atclose(fd_form_credits->form_credits,
				    CancelCloseBoxCB, NULL);

		if (!fl_load_browser(fd_form_credits->browser_credits, 
				     real_file.c_str())) {
			fl_add_browser_line(fd_form_credits->browser_credits,
					    _("ERROR: LyX wasn't able to read"
					    " CREDITS file"));
			fl_add_browser_line(fd_form_credits->browser_credits, "");
			fl_add_browser_line(fd_form_credits->browser_credits,
					    _("Please install correctly to estimate"
					    " the great"));
			fl_add_browser_line(fd_form_credits->browser_credits,
					    _("amount of work other people have done"
					    " for the LyX project."));
		}
	}

	if (fd_form_credits->form_credits->visible) {
		fl_raise_form(fd_form_credits->form_credits);
	} else {
		fl_show_form(fd_form_credits->form_credits,
			     FL_PLACE_MOUSE | FL_FREE_SIZE,FL_FULLBORDER,
			     _("Credits"));
	}
}

// Callback for copyright and warranty form
void CopyrightOKCB(FL_OBJECT *, long) {
	fl_hide_form(fd_form_copyright->copyright);
	fl_free_form(fd_form_copyright->copyright);
	fd_form_copyright->copyright = NULL;
}

// Show copyright and warranty form
void ShowCopyright()
{
	if (!fd_form_copyright || !fd_form_copyright->copyright) {
		fd_form_copyright = create_form_copyright();
		fl_set_form_atclose(fd_form_copyright->copyright, 
				    CancelCloseBoxCB, NULL);
	}

	// Funny emotional remark by Asger deleted. (Lgb)
	
	if (fd_form_copyright->copyright->visible) {
		fl_raise_form(fd_form_copyright->copyright);
	} else {
		fl_show_form(fd_form_copyright->copyright,
			     FL_PLACE_MOUSE, FL_FULLBORDER,
			     _("Copyright and Warranty"));
	}
}
