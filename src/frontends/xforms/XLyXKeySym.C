/**
 * \file XLyXKeySym.C
 * Read the file COPYING
 *
 * \author Asger and Juergen
 *
 * Full author contact details are available in file CREDITS
 */

#include <config.h>

#ifdef __GNUG__
#pragma implementation
#endif

#include "XLyXKeySym.h"

#include "debug.h"

#include <X11/Xlib.h>
#include <X11/keysym.h>

using std::endl;

XLyXKeySym::XLyXKeySym()
	: LyXKeySym(), keysym(NoSymbol)
{
}
 

void XLyXKeySym::initFromKeySym(KeySym ks)
{
	keysym = ks;
}
 

void XLyXKeySym::init(string const & symbolname)
{
	keysym = XStringToKeysym(symbolname.c_str());
	if (keysym == NoSymbol) {
		lyxerr[Debug::KBMAP]
			<< "XLyXKeySym.C: No such keysym: "
			<< symbolname << endl;
	}
}


bool XLyXKeySym::isOK() const
{
	return keysym != NoSymbol;
}


bool XLyXKeySym::isModifier() const
{
	// Can we be sure that this will work for all X Window
	// implementations? (Lgb)
	// Perhaps all of them should be explictly mentioned?
	return ((keysym >= XK_Shift_L && keysym <= XK_Hyper_R)
	    || keysym == XK_Mode_switch || keysym == 0x0);
}


string XLyXKeySym::getSymbolName() const
{
	char * name = XKeysymToString(keysym);
	return name ? name : string();
}

 
char XLyXKeySym::getISOEncoded() const
{
	if (keysym == NoSymbol) {
		return 0;
	}

	unsigned int c = keysym;

	switch (c & 0x0000FF00) {
		// latin 1 byte 3 = 0
	case 0x00000000: break;
		// latin 2 byte 3 = 1
	case 0x00000100:
		// latin 3 byte 3 = 2
	case 0x00000200:
		// latin 4 byte 3 = 3
	case 0x00000300:
		// cyrillic KOI8 & Co
	case 0x00000600:
		// greek
	case 0x00000700:
		// latin 8 byte 3 = 18 (0x12)
	case 0x00001200:
		// latin 9 byte 3 = 19 (0x13)
	case 0x00001300:
		c &= 0x000000FF;
		break;
	default:
		c = 0;
	}
	return c;
}

 
bool XLyXKeySym::operator==(LyXKeySym const & k) const
{
	// This is dangerous! Ideally, we should use dynamic_cast instead,
	// but on the other hand, we are sure that we will always get
	// the right type, because we decide at compile time which
	// frontend we use. (Asger)
	return keysym == static_cast<XLyXKeySym const &>(k).keysym;
}
