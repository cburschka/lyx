
#include <config.h>

#ifdef __GNUG__
#pragma implementation
#endif

#include "Sectioning.h"

string const & Section::name() const
{
	return name_;
}


int Section::level() const
{
	return level_;
}


string const & Section::indent() const
{
	return indent_;
}


string const & Section::beforeskip() const
{
	return beforeskip_;
}


string const & Section::afterskip() const
{
	return afterskip_;
}


LyXFont const & Section::style() const
{
	return style_;
}


bool Section::display() const
{
	// If afterskip is negative it is a display section.
	if (!afterskip_.empty() && afterskip_[0] == '-')
		return false;
	return true;
}
