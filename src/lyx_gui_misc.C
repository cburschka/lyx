// -*- C++ -*-
/* This file is part of
 * ====================================================== 
 * 
 *           LyX, The Document Processor
 *        
 *           Copyright 1995 Matthias Ettrich
 *           Copyright 1995-2000 The LyX Team.
 *
 * ====================================================== */

#include <config.h>

#ifdef __GNUG__
#pragma implementation
#endif

#include <cerrno>
#include "lyx_gui_misc.h"
#include "BufferView.h"
#include "buffer.h"
#include "credits_form.h"
#include "form1.h"
#include "gettext.h"
#include "lyx.h"
#include "lyx_cb.h"
#include "lyx_main.h"
#include "mathed/math_forms.h"
#include "minibuffer.h"
#include "print_form.h"
#include "sp_form.h"
#include "LyXView.h"
#include "bufferview_funcs.h"
#include "support/filetools.h"
#include "lyxrc.h"

using std::pair;
using std::make_pair;
using std::endl;

extern BufferView * current_view;

extern FD_form_credits * fd_form_credits;
extern FD_form_figure * fd_form_figure;
extern FD_form_sendto * fd_form_sendto;
extern FD_form_spell_check * fd_form_spell_check;
extern FD_form_spell_options * fd_form_spell_options;
extern FD_panel  * fd_panel;
extern FD_delim  * fd_delim;
extern FD_deco   * fd_deco;
extern FD_space  * fd_space;
extern FD_matrix * fd_matrix;

extern void HideFiguresPopups();

// Prevents LyX from being killed when the close box is pressed in a popup.
extern "C" int CancelCloseBoxCB(FL_FORM *, void *)
{
	return FL_CANCEL;
}
// Redraw the form (on receipt of a Signal indicating, for example,
// that the xform colors have been re-mapped).
void RedrawAllBufferRelatedDialogs()
{
	if (fd_deco && fd_deco->deco->visible) {
		fl_redraw_form(fd_deco->deco);
	}
	if (fd_delim && fd_delim->delim->visible) {
		fl_redraw_form(fd_delim->delim);
	}
	if (fd_form_credits && fd_form_credits->form_credits->visible) {
		fl_redraw_form(fd_form_credits->form_credits);
	}
	if (fd_form_figure->form_figure->visible) {
		fl_redraw_form(fd_form_figure->form_figure);
	}
	if (fd_form_sendto->form_sendto->visible) {
		fl_redraw_form(fd_form_sendto->form_sendto);
	}
	if (fd_form_spell_check &&
	    fd_form_spell_check->form_spell_check->visible) {
		fl_redraw_form(fd_form_spell_check->form_spell_check);
	}
	if (fd_form_spell_options &&
	    fd_form_spell_options->form_spell_options->visible) {
		fl_redraw_form(fd_form_spell_options->form_spell_options);
	}
	if (fd_matrix && fd_matrix->matrix->visible) {
		fl_redraw_form(fd_matrix->matrix);
	}
	if (fd_panel && fd_panel->panel->visible) {
		fl_redraw_form(fd_panel->panel);
	}
	if (fd_space && fd_space->space->visible) {
		fl_redraw_form(fd_space->space);
	}
}

// Prevents LyX from crashing when no buffers available
// This is also one of the functions that we _really_ dont want
// we should try to finds way to help us with that.
// The signal/slot mechanism can probably help. 
void CloseAllBufferRelatedDialogs()
{
	// don't forget to check that dynamically created forms
	// have been created otherwise hiding one could cause a crash
	// need the visible check otherwise XForms prints a warning
	// if hiding an invisible form
	if (fd_form_figure->form_figure->visible) {
		fl_hide_form(fd_form_figure->form_figure);
	}
	if (fd_form_sendto->form_sendto->visible) {
		fl_hide_form(fd_form_sendto->form_sendto);
	}
	if (fd_form_spell_check) {
		if (fd_form_spell_check->form_spell_check->visible) {
			fl_trigger_object(fd_form_spell_check->done);
		}
	}
	if (fd_panel) {
		if (fd_panel->panel->visible) {
			fl_hide_form(fd_panel->panel);
		}
	}
	if (fd_delim) {
		if (fd_delim->delim->visible) {
			fl_hide_form(fd_delim->delim);
		}
	}
	if (fd_deco) {
		if (fd_deco->deco->visible) {
			fl_hide_form(fd_deco->deco);
		}
	}
	if (fd_space) {
		if (fd_space->space->visible) {
			fl_hide_form(fd_space->space);
		}
	}
	if (fd_matrix) {
		if (fd_matrix->matrix->visible) {
			fl_hide_form(fd_matrix->matrix);
		}
	}
	HideFiguresPopups();
}

// This is another function we really don't want.
// Again the Signal/Slot mechanism is tailor made for this task.
void updateAllVisibleBufferRelatedDialogs(bool)
{
	if (current_view->buffer() &&  current_view->buffer()->isReadonly()) {
		// a little crude perhaps but it works. ARRae
#ifndef ALWAYS_CLOSE_MATH_PANELS
		// The math popups should be closed only if we switch
		// to a readonly buffer
		if (fd_panel) {
			if (fd_panel->panel->visible) {
				fl_hide_form(fd_panel->panel);
			}
		}
		if (fd_delim) {
			if (fd_delim->delim->visible) {
				fl_hide_form(fd_delim->delim);
			}
		}
		if (fd_deco) {
			if (fd_deco->deco->visible) {
				fl_hide_form(fd_deco->deco);
			}
		}
		if (fd_space) {
			if (fd_space->space->visible) {
				fl_hide_form(fd_space->space);
			}
		}
		if (fd_matrix) {
			if (fd_matrix->matrix->visible) {
				fl_hide_form(fd_matrix->matrix);
			}
		}
#endif
	}

	// We have either changed buffers or changed the readonly status
	// so the safest thing to do is hide all inset popups that
	// are editting insets from the previous buffer or aren't
	// allowed in readonly docs.
#ifdef ALWAYS_CLOSE_MATH_PANELS
	if (fd_panel) {
		if (fd_panel->panel->visible) {
			fl_hide_form(fd_panel->panel);
		}
	}
	if (fd_delim) {
		if (fd_delim->delim->visible) {
			fl_hide_form(fd_delim->delim);
		}
	}
	if (fd_deco) {
		if (fd_deco->deco->visible) {
			fl_hide_form(fd_deco->deco);
		}
	}
	if (fd_space) {
		if (fd_space->space->visible) {
			fl_hide_form(fd_space->space);
		}
	}
	if (fd_matrix) {
		if (fd_matrix->matrix->visible) {
			fl_hide_form(fd_matrix->matrix);
		}
	}
#endif
	HideFiguresPopups();
}

// Extract shortcut from <ident>|<shortcut> string
char const * flyx_shortcut_extract(char const * sc)
{
	// Find '|' in the sc and return the string after that.
	register char const * sd = sc;
	while(sd[0]!= 0 && sd[0] != '|') ++sd;

	if (sd[0] == '|') {
		++sd;
		//lyxerr << sd << endl;
		return sd;
	}
	return "";
}


// Extract identifier from <ident>|<shortcut> string
char const * flyx_ident_extract(char const * sc)
{
	register char const * se = sc;
	while(se[0]!= 0 && se[0] != '|') ++se;

	if (se[0] == 0) return sc;
	
	char * sb = new char[se - sc + 1];
	int index = 0;
	register char const * sd = sc;
	while (sd != se) {
		sb[index] = sd[0];
		++index; ++sd;
	}
	sb[index] = 0;
	return sb;
}


void WriteStatus(MiniBuffer * minib, string const & s)
{
	if (minib) {
		minib->Set(s);
		minib->Store();
	} else
		lyxerr << s << endl;
}


//
void WriteAlert(string const & s1, string const & s2, string const & s3)
{
	MiniBuffer * minibuffer = 0;
	if (current_view && current_view->owner())
		minibuffer = current_view->owner()->getMiniBuffer();
	if (minibuffer) {
		/// Write to minibuffer
		ProhibitInput(current_view);
		minibuffer->Set(s1, s2, s3);
		fl_set_resource("flAlert.dismiss.label", _("Dismiss"));
		fl_show_alert(s1.c_str(), s2.c_str(), s3.c_str(), 0);
		AllowInput(current_view);
	} else {
		/// Write to lyxerr
		lyxerr << "----------------------------------------" << endl
		       << s1 << endl << s2 << endl << s3 << endl
		       << "----------------------------------------" << endl;
	}
}


// Alarms user of something related to files
void WriteFSAlert(string const & s1, string const & s2)
{
	WriteAlert(s1, s2, strerror(errno));
}


bool AskQuestion(string const & s1, string const & s2, string const & s3,
		 bool default_value)
{
	if (!lyxrc.use_gui) {
		lyxerr << "----------------------------------------" << endl
		       << s1 << endl;
		if (!s2.empty())
			lyxerr << s2 << endl;
		if (!s3.empty())
			lyxerr << s3 << endl;
		lyxerr << "Assuming answer is "
		       << (default_value ? "yes" : "no")
		       << endl
		       << "----------------------------------------" << endl;
		return default_value;
	}

	fl_set_resource("flQuestion.yes.label", idex(_("Yes|Yy#y")));
	fl_set_resource("flQuestion.no.label", idex(_("No|Nn#n")));
	return fl_show_question((s1 + "\n" + s2 + "\n" + s3).c_str(), 0);
}


// Returns 1 for yes, 2 for no, 3 for cancel.
int AskConfirmation(string const & s1, string const & s2, string const & s3,
		    int default_value)
{
	if (!lyxrc.use_gui) {
		lyxerr << "----------------------------------------" << endl
		       << s1 << endl;
		if (!s2.empty())
			lyxerr << s2 << endl;
		if (!s3.empty())
			lyxerr << s3 << endl;
		lyxerr << "Assuming answer is ";
		if (default_value == 1)
			lyxerr << "yes";
		else if (default_value == 2)
			lyxerr << "no";
		else
			lyxerr << "cancel";
		lyxerr << endl
		       << "----------------------------------------" << endl;
		return default_value;
	}

	fl_set_choices_shortcut(scex(_("Yes|Yy#y")),
				scex(_("No|Nn#n")),
				scex(_("Cancel|^[")));
        return fl_show_choice(s1.c_str(), s2.c_str(), s3.c_str(), 
			      3, idex(_("Yes|Yy#y")),
			      idex(_("No|Nn#n")),
                              idex(_("Cancel|^[")), 3);
}


// Asks for a text
pair<bool, string> const
askForText(string const & msg, string const & dflt)
{
	if (!lyxrc.use_gui) {
		lyxerr << "----------------------------------------" << endl
		       << msg << endl
		       << "Assuming answer is " << dflt
		       << "----------------------------------------" << endl;
		return make_pair<bool, string>(true, dflt);
	}

	fl_set_resource("flInput.cancel.label", idex(_("Cancel|^[")));
	fl_set_resource("flInput.ok.label", idex(_("OK|#O")));
	fl_set_resource("flInput.clear.label", idex(_("Clear|#e")));
	char const * tmp = fl_show_input(msg.c_str(), dflt.c_str());
	if (tmp != 0)
	  return make_pair<bool, string>(true, string(tmp));
	else
	  return make_pair<bool, string>(false, string());
}


// Inform the user that the buffer is read-only, and that changes in the
// dialog box that is to appear will be ignored.
void WarnReadonly(string const & file)
{
	WriteAlert(_("Any changes will be ignored"),
		   _("The document is read-only:"),
		   MakeDisplayPath(file));
}

/// Get the dpi setting of the current screen
float getScreenDPI()
{
	Screen * scr = ScreenOfDisplay(fl_get_display(), fl_screen); //DefaultScreen(fl_get_display());
	return ((HeightOfScreen(scr) * 25.4 / HeightMMOfScreen(scr)) +
		(WidthOfScreen(scr) * 25.4 / WidthMMOfScreen(scr))) / 2;
}
