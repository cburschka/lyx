/**
 * \file QLyXKeySym.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Asger and Juergen
 * \author John Levon 
 *
 * Full author contact details are available in file CREDITS
 */

#include <config.h>

#ifdef __GNUG__
#pragma implementation
#endif

#include "QLyXKeySym.h"
#include "qlkey.h"
#include "debug.h"
 
#include <qevent.h>

using std::endl;

QLyXKeySym::QLyXKeySym()
	: LyXKeySym(), key_(0)
{
}
 
 
void QLyXKeySym::set(QKeyEvent * ev)
{
	key_ = ev->key();
	text_ = ev->text(); 
}
 

void QLyXKeySym::init(string const & symbolname)
{
	key_ = string_to_qkey(symbolname);
	text_ = symbolname.c_str();
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


string QLyXKeySym::getSymbolName() const
{
	string sym(qkey_to_string(key_));

	if (sym.empty()) {
		lyxerr[Debug::KEY] << "sym empty in getSymbolName()" << endl;
		sym = text_.latin1();
	}
	lyxerr[Debug::KEY] << "getSymbolName() -> " << sym << endl;
	return sym;
}

 
char QLyXKeySym::getISOEncoded() const
{
	lyxerr[Debug::KEY] << "getISO returning " << text_.latin1()[0] << endl;
	return text_.latin1()[0]; 
}
 

bool QLyXKeySym::operator==(LyXKeySym const & k) const
{
	QLyXKeySym const & o = static_cast<QLyXKeySym const &>(k);
	// ignore text_ ! 
	return o.key_ == key_;
}
