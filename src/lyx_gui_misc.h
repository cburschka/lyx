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

#ifdef __GNUG__
#pragma interface
#endif

#include FORMS_H_LOCATION
#include "LString.h"
#include <utility>  /* needed for pair<> definition */

class MiniBuffer;

/// Prevents LyX from being killed when the close box is pressed in a popup.
extern "C" int CancelCloseBoxCB(FL_FORM *, void *);
/// Prevents LyX from being killed when the close box is pressed in a popup.
extern "C" int IgnoreCloseBoxCB(FL_FORM *, void *);

/// Prevents LyX from crashing when no buffers available
void CloseAllBufferRelatedDialogs();

/// Ensures info in visible popups are always correct.
void updateAllVisibleBufferRelatedDialogs(bool switched = false);

/* These shortcut extractors should be shifted to frontends/xforms/ eventually */

/// Extract shortcut from <ident>|<shortcut> string
char const * flyx_shortcut_extract(char const * sc);
/// Shortcut for flyx_shortcut_extract
#define scex flyx_shortcut_extract

/// Extract shortcut from <ident>|<shortcut> string
char const * flyx_ident_extract(char const * sc);
/// Shortcut for flyx_ident_extract
#define idex flyx_ident_extract

/// Show status message
void WriteStatus(MiniBuffer * minib, string const & s);

/// Show message
void WriteAlert(string const & s1, string const & s2 = string(), 
		string const & s3 = string());

/// Alarms user of something related to files
void WriteFSAlert(string const & s1, string const & s2 = string());

/// Asks "Yes" or "No". Returns true for yes, false for no
bool AskQuestion(string const & s1, string const & s2 = string(),
		 string const & s3 = string(), bool default_value = true);

/// Asks "Yes", "No" or "Cancel". Returns 1 for yes, 2 for no, 3 for cancel
int AskConfirmation(string const & s1, string const & s2 = string(), 
		    string const & s3 = string(), int default_value = 1);

/// returns a bool: false=cancelled, true=okay. string contains returned text
std::pair<bool, string> const
askForText(string const & msg,
	   string const & dflt = string());

/// Informs the user that changes in the coming form will be ignored
void WarnReadonly(string const & file);

/// Get the dpi setting of the current screen
float getScreenDPI();

/// rings the audio bell.
inline
void LyXBell() {
	// if (audio()) ON/OFF switch yet to be implemented
	fl_ringbell(20);
}

#endif
