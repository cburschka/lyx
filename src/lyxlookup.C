/* This file is part of
 * ====================================================== 
 * 
 *           LyX, The Document Processor
 * 	 
 *           Copyright 1995 Matthias Ettrich 
 *           Copyright 1995-2000 The LyX team.
 *
 * ====================================================== */

#ifdef __GNUG__
#pragma implementation
#endif

#include <config.h>

#ifdef HAVE_XOPENIM
// This part is the full blown Input Method manager for X11R5 and up.
// For the plain-and-old-X11R4 version, see later.
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/keysym.h>
#ifdef HAVE_LOCALE_H
#include <locale.h>
#endif
#include <clocale>

#include "lyxlookup.h"
#include "debug.h"
#include "lyxrc.h"

using std::endl;

static XIM xim;
static XIC xic;
XComposeStatus compose_status= {0, 0};

// This is called after the main LyX window has been created
void InitLyXLookup(Display * display, Window window) 
{
	xic = 0;

	lyxerr[Debug::KEY]
			<< "InitLyXLookup: creating an input context."
			<< endl;

	// This part could be done before opening display
	setlocale(LC_CTYPE, "");
       	if (!XSupportsLocale()) {
		lyxerr[Debug::KEY]
			<< "InitLyXLookup: X does not support this locale."
			<< endl;
		return;
	} 
	if (!XSetLocaleModifiers("")) {
		lyxerr[Debug::KEY] << "InitLyXLookup: Could not set modifiers "
			"for this locale." << endl;
		return;
	}
	
	// This part will have to be done for each frame
	xim = XOpenIM (display, 0, 0, 0);
	if (xim) {
		xic = XCreateIC(xim, XNInputStyle,
				XIMPreeditNothing | XIMStatusNothing,
				XNClientWindow, window,
				XNFocusWindow, window, 
				0);
		
		if (!xic) {
			lyxerr[Debug::KEY] << "InitLyXLookup: could not create "
				"an input context" << endl;
			XCloseIM (xim);
			xim = 0;
		} 
	}
	else 
		lyxerr[Debug::KEY] << "InitLyXLookup: could not open "
			"an input method." << endl;
}


static
bool isDeadEvent(XEvent * event,
		  char * buffer_return, int bytes_buffer,
		  KeySym * keysym_return)
{
	XLookupString(&event->xkey, buffer_return,
		      bytes_buffer, keysym_return,
		      0);

	// somehow it is necessary to do the lookup. Why? (JMarc)
	if (!lyxrc.override_x_deadkeys)
		return false;

	// Can this be done safely in any other way?
	// This is all the dead keys I know of in X11R6.1
	switch (*keysym_return) {
#ifdef XK_dead_grave
	case XK_dead_grave:
#endif
#ifdef XK_dead_acute
	case XK_dead_acute:
#endif
#ifdef XK_dead_circumflex
	case XK_dead_circumflex:
#endif
#ifdef XK_dead_tilde
	case XK_dead_tilde:
#endif
#ifdef XK_dead_macron
	case XK_dead_macron:
#endif
#ifdef XK_dead_breve
	case XK_dead_breve:
#endif
#ifdef XK_dead_abovedot
	case XK_dead_abovedot:
#endif
#ifdef XK_dead_diaeresis
	case XK_dead_diaeresis:
#endif
#ifdef XK_dead_abovering
	case XK_dead_abovering:
#endif
#ifdef XK_dead_doubleacute
	case XK_dead_doubleacute:
#endif
#ifdef XK_dead_caron
	case XK_dead_caron:
#endif
#ifdef XK_dead_cedilla
	case XK_dead_cedilla:
#endif
#ifdef XK_dead_ogonek
	case XK_dead_ogonek:
#endif
#ifdef XK_dead_iota
	case XK_dead_iota:
#endif
#ifdef XK_dead_voiced_sound
	case XK_dead_voiced_sound:
#endif
#ifdef XK_dead_semivoiced_sound
	case XK_dead_semivoiced_sound:
#endif
#ifdef XK_dead_belowdot
	case XK_dead_belowdot:
#endif
		return true;
	default:
		return false;
	}
}


// This is called instead of XLookupString()
int LyXLookupString(XEvent * event,    
		    char * buffer_return, int bytes_buffer,
		    KeySym * keysym_return) 
{
	int result = 0;
	if (xic) {
		if (isDeadEvent(event, buffer_return, bytes_buffer,
				keysym_return)) {
			lyxerr[Debug::KEY]  
				<< "LyXLookupString: found DeadEvent" << endl;
			return 0;
		}
#if 1
		if (XFilterEvent (event, None)) {
			lyxerr[Debug::KEY] <<"XFilterEvent" << endl;
			*keysym_return = NoSymbol;
                        return 0;
		}
#endif
		if (event->type != KeyPress)
			lyxerr << "LyXLookupString: wrong event type" 
			       <<  event->type << endl;
		Status status_return = 0;
		
		result =  XmbLookupString(xic, &event->xkey, buffer_return,
				       bytes_buffer, keysym_return,
				       &status_return);
		switch (status_return) {
		case XBufferOverflow:
			lyxerr[Debug::KEY] << "XBufferOverflow" << endl;
			break;
		case XLookupBoth:
			lyxerr[Debug::KEY] << "XLookupBoth "
					   << string(buffer_return, result)
					   << endl;
			break;
		case XLookupChars:
			lyxerr[Debug::KEY] << "XLookupChars "
					   << string(buffer_return, result)
					   << endl;
			
			*keysym_return = NoSymbol;
			break;
		case XLookupKeySym:
			lyxerr[Debug::KEY] << "XLookupKeySym" << endl;
			result = 0;
			break;
		case XLookupNone:
			lyxerr[Debug::KEY] << "XLookupNone" << endl;
			*keysym_return = NoSymbol;
			result = 0;
			break;
		default:
			lyxerr << "Unknown status_return from"
				" XmbLookupString" << endl;
			break;
		}
	} else {
		result = XLookupString(&event->xkey, buffer_return,
				  bytes_buffer, keysym_return,
				  &compose_status);
	}
	return result;
}


// This is called after the main window has been destroyed
void CloseLyXLookup() 
{
	if (xic) {
		lyxerr[Debug::KEY] << "CloseLyXLookup: destroying input context"
			       << endl;
		XDestroyIC(xic);
		xic = 0;
		XCloseIM(xim);
	}
}


#else // We do not have XOpenIM, so we stick with normal XLookupString

#include <X11/Xlib.h>
#include <X11/Xutil.h>

XComposeStatus compose_status= {0, 0};

// This is called after the main LyX window has been created
void InitLyXLookup(Display *, Window ) 
{
	//Nothing to do.
}

// This is called instead of XLookupString(). I this particular case,
// this *is* XLookupString...
int LyXLookupString(XEvent * event,    
		    char * buffer_return, int bytes_buffer,
		    KeySym * keysym_return) 
{
	return XLookupString(&event->xkey, buffer_return,
				  bytes_buffer, keysym_return,
				  &compose_status);
}

// This is called after the main window has been destroyed
void CloseLyXLookup() 
{
	// Nothing to do
}

#endif // HAVE_XOPENIM

