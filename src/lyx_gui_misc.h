// -*- C++ -*-
/* This file is part of
 * ======================================================
 * 
 *           LyX, The Document Processor
 *        
 *           Copyright 1995 1996 Matthias Ettrich
 *           Copyright 1995-2001 The LyX Team
 *
 * ====================================================== */

// Misc. GUI specific routines

#ifndef LYX_GUI_MISC_H
#define LYX_GUI_MISC_H

#ifdef __GNUG__
#pragma interface
#endif

#include FORMS_H_LOCATION

/// Prevents LyX from being killed when the close box is pressed in a popup.
extern "C"
int CancelCloseBoxCB(FL_FORM *, void *);

/** Redraw the form (on receipt of a Signal indicating, for example,
    that the xform colors have been re-mapped). */
void RedrawAllBufferRelatedDialogs();

/// Prevents LyX from crashing when no buffers available
void CloseAllBufferRelatedDialogs();

/// Ensures info in visible popups are always correct.
void updateAllVisibleBufferRelatedDialogs(bool switched = false);

#endif
