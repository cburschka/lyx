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
 
QLyXKeySym::QLyXKeySym()
	: LyXKeySym(), key_(0), shift_(false)
{
}
 
 
void QLyXKeySym::set(int key, bool shift)
{
	key_ = key;
	shift_ = shift;
}
 

void QLyXKeySym::init(string const & symbolname)
{
	key_ = string_to_qkey(symbolname);
}

 
bool QLyXKeySym::isOK() const
{
	// FIXME
	return key_ != 0;
}
 

bool QLyXKeySym::isModifier() const
{
	return q_is_modifier(key_);
}

 
string QLyXKeySym::getSymbolName() const
{
	return qkey_to_string(key_, shift_);
}

 
char QLyXKeySym::getISOEncoded() const
{
	/* Even though we could try to use QKeyEvent->text(),
	 * it won't work, because it returns something other
	 * than 'Z' for things like C-S-z. Do not ask me why,
	 * just more Qt bullshit.	
	 */
	return qkey_to_char(key_, shift_);
}
 

bool QLyXKeySym::operator==(LyXKeySym const & k) const
{
	QLyXKeySym const & o = static_cast<QLyXKeySym const &>(k);
	return o.key_ == key_;
}
