// -*- C++ -*-
/* This file is part of
* ======================================================
* 
*           LyX, The Document Processor
*        
*           Copyright (C) 1995 1996 Matthias Ettrich
*           and the LyX Team.
*
*======================================================*/

// Misc. GUI specific routines

#ifndef _LYX_GUI_MISC_H
#define _LYX_GUI_MISC_H

#include FORMS_H_LOCATION
#include "LString.h"

/// Prevents LyX from being killed when the close box is pressed in a popup.
extern "C" int CancelCloseBoxCB(FL_FORM *, void *);
/// Prevents LyX from being killed when the close box is pressed in a popup.
extern "C" int IgnoreCloseBoxCB(FL_FORM *, void *);

/// Prevents LyX from crashing when no buffers available
void CloseAllBufferRelatedPopups();

/// Ensures info in visible popups are always correct.
void updateAllVisibleBufferRelatedPopups();

/// Extract shortcut from <ident>|<shortcut> string
const char* flyx_shortcut_extract(const char*sc);
/// Make a shortnamed version of the above func
#define scex flyx_shortcut_extract

/// Extract shortcut from <ident>|<shortcut> string
const char* flyx_ident_extract(const char *sc);
/// Make a shortnamed versjon of the above func
#define idex flyx_ident_extract

/// Show message
void WriteAlert(LString const & s1, LString const & s2=LString(), 
		LString const & s3=LString());

/// Alarms user of something related to files
void WriteFSAlert(LString const & s1, LString const & s2=LString());

/// Asks "Yes" or "No". Returns true for yes, false for no
bool AskQuestion(LString const & s1, LString const & s2=LString(),
		 LString const & s3=LString());

/// Asks "Yes", "No" or "Cancel". Returns 1 for yes, 2 for no, 3 for cancel
int AskConfirmation(LString const & s1, LString const & s2=LString(), 
		    LString const & s3=LString());

/// returns a text
LString askForText(LString const & msg, LString const & dflt=LString());

/// Informs the user that changes in the coming form will be ignored
void WarnReadonly();

// inlined functions
/// rings the audio bell.
inline
void LyXBell() {
	// if (audio()) ON/OFF switch yet to be implemented
#if FL_REVISION > 85
	fl_ringbell(20);
#else
	ringbell();
#endif
}

#endif
