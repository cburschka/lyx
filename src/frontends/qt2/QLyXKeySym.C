/**
 * \file QLyXKeySym.C
 * Copyright 2002 the LyX Team
 * Read the file COPYING
 *
 * \author Asger and Juergen
 * \author John Levon <moz@compsoc.man.ac.uk>
 */

#include <config.h>

#ifdef __GNUG__
#pragma implementation
#endif

#include "QLyXKeySym.h"
#include "qlkey.h"
#include "debug.h"
 
#include <qevent.h>
 
QLyXKeySym::QLyXKeySym()
	: LyXKeySym(), key_(0)
{
}
 
 
void QLyXKeySym::set(QKeyEvent * ev)
{
	key_ = ev->key();
	text_ = ev->text(); 
	ascii_ = ev->ascii();
}
 

void QLyXKeySym::init(string const & symbolname)
{
	key_ = string_to_qkey(symbolname);
	text_ = symbolname.c_str();
	ascii_ = 0;
	lyxerr[Debug::KEY] << "Init key to " << key_ << ", " << text_ << endl;
}

 
bool QLyXKeySym::isOK() const
{
	return ! key_ == 0;
}
 

bool QLyXKeySym::isModifier() const
{
	return q_is_modifier(key_);
}


// This is one ALMIGHTY hack. When you press C-S-z, you get
// "Press key 90 text "?", ascii "26"
// where text is meaningless. So we check specifically
// for this case ! (90 is 'Z')
// We also check against 0 for when we're comparing
// against a stored binding. 
bool QLyXKeySym::is_qt_bogon() const
{
	if (ascii_ == 0)
		return false;
	return (ascii_ < 27 && !text_.isEmpty());
} 
 
 
char QLyXKeySym::debogonify() const
{
	return 'A' + ascii_ - 1;
}


string QLyXKeySym::getSymbolName() const
{
	string sym(qkey_to_string(key_));

	// deal with "A", "a" properly
	if (sym.empty()) {
		lyxerr[Debug::KEY] << "sym empty in getSymbolName()" << endl;
 
		if (is_qt_bogon()) {
			sym = debogonify();
		} else {
			sym = text_.latin1();
		}
	}
	lyxerr[Debug::KEY] << "getSymbolName() -> " << sym << endl;
	return sym;
}

 
char QLyXKeySym::getISOEncoded() const
{
	lyxerr[Debug::KEY] << "getISO returning " << text_.latin1()[0] << endl;

	if (is_qt_bogon()) {
		return debogonify();
	}
 
	return text_.latin1()[0]; 
}
 

bool QLyXKeySym::operator==(LyXKeySym const & k) const
{
	QLyXKeySym const & o = static_cast<QLyXKeySym const &>(k);
	// ignore text_ ! 
	return o.key_ == key_;
}
