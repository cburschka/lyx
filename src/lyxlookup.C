/* This file is part of
 * ====================================================== 
 * 
 *           LyX, The Document Processor
 * 	 
 *           Copyright 1995 Matthias Ettrich 
 *           Copyright 1995-1999 The LyX team.
 *
 * ====================================================== */

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

#include "debug.h"

static XIM xim;
static XIC xic;
XComposeStatus compose_status= {0, 0};

// This is called after the main LyX window has been created
void InitLyXLookup(Display * display, Window window) 
{
	xic = 0;
	
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
	// Can this be done safely in any other way?
	// This is all the dead keys I know of in X11R6.1
	if (false
#ifdef XK_dead_grave
	    || *keysym_return == XK_dead_grave
#endif
#ifdef XK_dead_acute
	    || *keysym_return == XK_dead_acute
#endif
#ifdef XK_dead_circumflex
	    || *keysym_return == XK_dead_circumflex
#endif
#ifdef XK_dead_tilde
	    || *keysym_return == XK_dead_tilde
#endif
#ifdef XK_dead_macron
	    || *keysym_return == XK_dead_macron
#endif
#ifdef XK_dead_breve
	    || *keysym_return == XK_dead_breve
#endif
#ifdef XK_dead_abovedot
	    || *keysym_return == XK_dead_abovedot
#endif
#ifdef XK_dead_diaeresis
	    || *keysym_return == XK_dead_diaeresis
#endif
#ifdef XK_dead_abovering
	    || *keysym_return == XK_dead_abovering
#endif
#ifdef XK_dead_doubleacute
	    || *keysym_return == XK_dead_doubleacute
#endif
#ifdef XK_dead_caron
	    || *keysym_return == XK_dead_caron
#endif
#ifdef XK_dead_cedilla
	    || *keysym_return == XK_dead_cedilla
#endif
#ifdef XK_dead_ogonek
	    || *keysym_return == XK_dead_ogonek
#endif
#ifdef XK_dead_iota
	    || *keysym_return == XK_dead_iota
#endif
#ifdef XK_dead_voiced_sound
	    || *keysym_return == XK_dead_voiced_sound
#endif
#ifdef XK_dead_semivoiced_sound
	    || *keysym_return == XK_dead_semivoiced_sound
#endif
#ifdef XK_dead_belowdot
	    || *keysym_return == XK_dead_belowdot
#endif
	    )
		return true;
	return false;
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
			return 0;
		}
		if (XFilterEvent (event, None)) {
			//lyxerr <<"XFilterEvent");
			*keysym_return = NoSymbol;
                        return 0;
		}
		if (event->type != KeyPress)
			lyxerr << "LyXLookupString: wrong event type" 
			       <<  event->type << endl;
		Status status_return;
		
		result =  XmbLookupString(xic, &event->xkey, buffer_return,
				       bytes_buffer, keysym_return,
				       &status_return);
		switch(status_return) {
		case XLookupBoth:
			//lyxerr <<"XLookupBoth");
			break;
		case XLookupChars:
			//lyxerr <<"XLookupChars");
			*keysym_return = NoSymbol;
			break;
		case XLookupKeySym:
			//lyxerr <<"XLookupKeySym");
			result = 0;
			break;
		default:
			//lyxerr <<"default");
			*keysym_return = NoSymbol;
			result = 0;
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
