// -*- C++ -*-
/* This file is part of
 * ======================================================
 * 
 *           LyX, The Document Processor
 *        
 *           Copyright 1995 1996 Matthias Ettrich
 *           Copyright 1995-2000 The LyX Team
 *
 * ====================================================== */

// Misc. GUI specific routines

#ifndef LYX_GUI_MISC_H
#define LYX_GUI_MISC_H

#include FORMS_H_LOCATION
#include "LString.h"
#include <utility>  /* needed for pair<> definition */

/// Prevents LyX from being killed when the close box is pressed in a popup.
extern "C" int CancelCloseBoxCB(FL_FORM *, void *);
/// Prevents LyX from being killed when the close box is pressed in a popup.
extern "C" int IgnoreCloseBoxCB(FL_FORM *, void *);

/// Prevents LyX from crashing when no buffers available
void CloseAllBufferRelatedPopups();

/// Ensures info in visible popups are always correct.
void updateAllVisibleBufferRelatedPopups();

/// Extract shortcut from <ident>|<shortcut> string
char const * flyx_shortcut_extract(char const * sc);
/// Make a shortnamed version of the above func
#define scex flyx_shortcut_extract

/// Extract shortcut from <ident>|<shortcut> string
char const * flyx_ident_extract(char const * sc);
/// Make a shortnamed versjon of the above func
#define idex flyx_ident_extract

/// Show message
void WriteAlert(string const & s1, string const & s2 = string(), 
		string const & s3 = string());

/// Alarms user of something related to files
void WriteFSAlert(string const & s1, string const & s2 = string());

/// Asks "Yes" or "No". Returns true for yes, false for no
bool AskQuestion(string const & s1, string const & s2 = string(),
		 string const & s3 = string());

/// Asks "Yes", "No" or "Cancel". Returns 1 for yes, 2 for no, 3 for cancel
int AskConfirmation(string const & s1, string const & s2 = string(), 
		    string const & s3 = string());

/// returns a bool: false=cancelled, true=okay. string contains returned text
std::pair<bool, string> askForText(string const & msg,
				   string const & dflt = string());

/// Informs the user that changes in the coming form will be ignored
void WarnReadonly(string const & file);

// inlined functions
/// rings the audio bell.
inline
void LyXBell() {
	// if (audio()) ON/OFF switch yet to be implemented
	fl_ringbell(20);
}

#endif
