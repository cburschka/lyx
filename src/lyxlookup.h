/* This file is part of                   -*- C++ -*-
* ====================================================== 
* 
*           LyX, The Document Processor
* 	 
*	    Copyright (C) 1995-1997 Matthias Ettrich 
*                                    & The LyX team
*
* ====================================================== */

/* This header file defines wrappers around the X input method related
   functions. This should probably be moved into LyXView (to have
   different input methods for different frames, but for now we can
   keep it as it is. */

#include <X11/Xlib.h>

// Initialize the compose key handling
extern void InitLyXLookup(Display* , Window ) ;

// Read a keysym and/or a string (like XLookupString)
extern int LyXLookupString(XEvent *event,    
		    char *buffer_return, int bytes_buffer,
		    KeySym *keysym_return);

// Call this when you destroy your window
extern void CloseLyXLookup();

