/* This file is part of
 * ======================================================
 * 
 *           LyX, The Document Processor
 * 	 
 *	     Copyright 1995 Matthias Ettrich
 *           Copyright 1995-1999 The LyX Team.
 *
 * ======================================================*/

#include <config.h>

#include FORMS_H_LOCATION
#include <cstdlib>

#include "credits.h"
#include "credits_form.h"

#include "LString.h"
#include "support/filetools.h"
#include "lyx_gui_misc.h" // CancelCloseBoxCB
#include "gettext.h"

extern string system_lyxdir;

/**** Forms and Objects ****/

static FD_form_credits *fd_form_credits = 0;
static FD_copyright *fd_form_copyright = 0;

void CreditsOKCB(FL_OBJECT *, long) {
      fl_hide_form(fd_form_credits->form_credits);
      fl_free_form(fd_form_credits->form_credits);
      fd_form_credits->form_credits = 0;
}

/*---------------------------------------*/
/* read credits from file and display them */
void ShowCredits()
{
	/* generate the credits form if it doesn't exist */
	if (!fd_form_credits || !fd_form_credits->form_credits) {
		
		/* read the credits into the browser */ 
		
		/* try file LYX_DIR/CREDITS */ 
		string real_file = AddName (system_lyxdir, "CREDITS");
		fd_form_credits = create_form_form_credits();
		fl_set_form_atclose(fd_form_credits->form_credits,
				    CancelCloseBoxCB, 0);

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
	fd_form_copyright->copyright = 0;
}

// Show copyright and warranty form
void ShowCopyright()
{
	if (!fd_form_copyright || !fd_form_copyright->copyright) {
		fd_form_copyright = create_form_copyright();
		fl_set_form_atclose(fd_form_copyright->copyright, 
				    CancelCloseBoxCB, 0);
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
