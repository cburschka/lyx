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

#include "qevent.h"
#include "qlkey.h"
 
QLyXKeySym::QLyXKeySym()
	: LyXKeySym(), key_(0), text_("")
{
}
 
 
void QLyXKeySym::set(int key, QString const & text)
{
	key_ = key;
	text_ = text;
}
 

void QLyXKeySym::init(string const & symbolname)
{
	// FIXME: ARGHH !
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
	// FIXME
	return qkey_to_string(key_);
}

 
char QLyXKeySym::getISOEncoded() const
{
	// FIXME
	return text_.latin1()[0];
}
 

bool QLyXKeySym::operator==(LyXKeySym const & k) const
{
	QLyXKeySym const & o = static_cast<QLyXKeySym const &>(k);

	// FIXME: UGHGHGHGGH
	return o.key_ == key_ /*&& o.text_ == text_*/;
}
