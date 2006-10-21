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

// Too hard to make concept checks work with this file
#ifdef _GLIBCXX_CONCEPT_CHECKS
#undef _GLIBCXX_CONCEPT_CHECKS
#endif
#ifdef _GLIBCPP_CONCEPT_CHECKS
#undef _GLIBCPP_CONCEPT_CHECKS
#endif

#include "GLyXKeySym.h"
#include "kbmap.h"

#include "support/lstrings.h"

#include <gtkmm.h>
#include <gdk/gdkkeysyms.h>


namespace lyx {

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


size_t GLyXKeySym::getUCSEncoded() const
{
    return gdk_keyval_to_unicode(keyval_);
}


// Produce a human readable version (eg "Ctrl+N")
string const GLyXKeySym::print(key_modifier::state mod) const
{
	string buf;

	if (mod & key_modifier::ctrl)
		buf += "Ctrl+";
	if (mod & key_modifier::shift)
		buf += "Shift+";
	if (mod & key_modifier::alt)
		buf += "Alt+";

	// Uppercase the first letter, for Ctrl+N rather than Ctrl+n,
	// and for Ctrl+Greater rather than Ctrl+GREATER
	string symname = getSymbolName();
	if (!symname.empty()) {
	  symname[0] = lyx::support::uppercase(symname[0]);
	  buf += symname;
	}

	return buf;
}


bool GLyXKeySym::isText() const
{
	return getUCSEncoded() != 0;
}


bool operator==(LyXKeySym const & k1, LyXKeySym const & k2)
{
	return static_cast<GLyXKeySym const &>(k1).getKeyval()
		== static_cast<GLyXKeySym const &>(k2).getKeyval();
}


} // namespace lyx
