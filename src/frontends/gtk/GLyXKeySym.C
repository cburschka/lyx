/**
 * \file GLyXKeySym.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Huang Ying
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "GLyXKeySym.h"
#include "kbmap.h"

#include "support/lstrings.h"

#include <gtkmm.h>
#include <gdk/gdkkeysyms.h>

using std::string;


GLyXKeySym::GLyXKeySym() : keyval_(GDK_VoidSymbol)
{
}


GLyXKeySym::GLyXKeySym(unsigned int keyval) : keyval_(keyval)
{
}


void GLyXKeySym::setKeyval(unsigned int keyval)
{
	keyval_ = keyval;
}


void GLyXKeySym::init(string const & symbolname)
{
	keyval_ = gdk_keyval_from_name(symbolname.c_str());
}


bool GLyXKeySym::isOK() const
{
	return keyval_ != GDK_VoidSymbol;
}


bool GLyXKeySym::isModifier() const
{
	return ((keyval_ >= GDK_Shift_L && keyval_ <= GDK_Hyper_R)
		|| keyval_ == GDK_Mode_switch || keyval_ == 0);
}


string GLyXKeySym::getSymbolName() const
{
	const char * name = gdk_keyval_name(keyval_);
	return name ? name : string();
}


char GLyXKeySym::getISOEncoded(string const & /*encoding*/) const
{
	if (keyval_ == GDK_VoidSymbol)
		return 0;

	unsigned int c = keyval_;

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


//Produce a human readable version (eg "Ctrl+N")
string const GLyXKeySym::print(key_modifier::state mod) const
{
	string buf;

	if (mod & key_modifier::ctrl)
		buf += "Ctrl+";
	if (mod & key_modifier::shift)
		buf += "Shift+";
	if (mod & key_modifier::alt)
		buf += "Alt+";

	//Uppercase the first letter, for Ctrl+N rather than Ctrl+n,
	//and for Ctrl+Greater rather than Ctrl+GREATER
	string symname = getSymbolName();
	if (!symname.empty()) {
	  symname[0] = lyx::support::uppercase(symname[0]);
	  buf += symname;
	}

	return buf;
}


bool operator==(LyXKeySym const & k1, LyXKeySym const & k2)
{
	return static_cast<GLyXKeySym const &>(k1).getKeyval()
		== static_cast<GLyXKeySym const &>(k2).getKeyval();
}
