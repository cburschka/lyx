/* This file is part of
 * ======================================================
 * 
 *           LyX, The Document Processor
 *
 *           Copyright 1998-2000 The LyX Team.
 *
 * ======================================================
 */

#include <config.h>

#ifdef __GNUG__
#pragma implementation
#endif

#include "Floating.h"


Floating::Floating() 
{}


Floating::Floating(string const & type, string const & placement,
		   string const & ext, string const & within,
		   string const & style, string const & name,
		   bool builtin)
	: type_(type), placement_(placement), ext_(ext), within_(within),
	  style_(style), name_(name), builtin_(builtin)
{}


string const & Floating::type() const
{
	return type_;
}


string const & Floating::placement() const
{
	return placement_;
}


string const & Floating::name() const
{
	return name_;
}


bool Floating::builtin() const
{
	return builtin_;
}
